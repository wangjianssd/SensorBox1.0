#include "app.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "stack/custom/mac/mac.h"
#include "stack/custom/nwk/gateway/nwk_interface.h"
#include "stack/custom/nwk/gateway/nwk_frames.h"
#include "common/dev/dev.h"
#include "apps/gateway/app_func.h"

#define BUILD_IP_ADDRESS(b3, b2, b1, b0) ((uint32_t)(b3) << 24) | \
((uint32_t)(b2) << 16) | ((uint32_t)(b1) << 8) | ((uint32_t)(b0))
#define RETREAT_NUM_MAX                             (10u)
#define GW_SEND_MAX                                 (3u)
#define GPRS_DATA_NUM_MAX                           (25u)


#define APP_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t app_event_store[APP_EVENT_MAX];
static osel_task_t *app_task_handle = NULL;

static osel_etimer_t cycle_timer_handle;
static osel_etimer_t app_cycle_gprs_send_timer;
static osel_etimer_t restart_gprs_send_timer;
static uint8_t retreat_num = 0;
static uint8_t gw_send_times = 0;
static bool_t gprs_can_send = TRUE;

static list_head_t gprs_cache_head;

void app_gprs_can_send(bool_t mark)
{
	gprs_can_send = mark;
}

bool_t app_gprs_can_send_flag(void)
{
	return gprs_can_send;
}

static void escort_gprs_init(void)
{
	P1SEL &=~BIT3;//EN_6130 power
	P1DIR |= BIT3;
	P1OUT |= BIT3;

	P1DIR |= BIT7;
	P1OUT |= BIT7;  	
}

static void app_cycle_timer_cb(void)
{
    hal_wdt_clear(16000);
}

static void app_cycle_timer_start(void)
{
    osel_etimer_arm(&cycle_timer_handle, 1000/OSEL_TICK_PER_MS, 1000/OSEL_TICK_PER_MS);
}

static void app_cycle_gprs_send_timer_start(void)
{
    osel_etimer_arm(&app_cycle_gprs_send_timer, 120000/OSEL_TICK_PER_MS, 0);
}

static void restart_gprs_send_timer_handler(void)
{
    osel_etimer_arm(&restart_gprs_send_timer, 2000/OSEL_TICK_PER_MS, 0);
}

static void gprs_receive_cb(gprs_receive_t param)
{
	_NOP();
}

static void gprs_list_del(void)
{
    if(!list_empty(&gprs_cache_head))
    {
        pbuf_t *pbuf = list_entry_decap(&gprs_cache_head, pbuf_t, list);
        pbuf_free(&pbuf __PLINE2);
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}


static void gprs_data_add_list(uint8_t *pload, uint16_t len)
{
    osel_event_t event;
    
    DBG_ASSERT(pload != NULL __DBG_LINE);
    DBG_ASSERT(len <= LARGE_PBUF_BUFFER_SIZE __DBG_LINE);
    
    uint8_t count = 0;
    pbuf_t *pbuf;
    if(!list_empty(&gprs_cache_head))
    {
        list_count(&gprs_cache_head,count);
    }
    
    if(count == GPRS_DATA_NUM_MAX )                //list满了，删除最老的
    {
        pbuf_t *pbuf = list_entry_decap(&gprs_cache_head, pbuf_t, list);
        pbuf_free(&pbuf __PLINE2);
    }
    pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    osel_memcpy(pbuf->data_p, pload, len);
    pbuf->data_len = len;
    list_add_to_tail(&(pbuf->list), &gprs_cache_head);
    
    if(app_gprs_can_send_flag())
    {
        event.sig = APP_GPRS_DATA_SENT_START_EVENT;
        event.param = NULL;
        osel_post(NULL, &app_process, &event);
//        osel_post(APP_GPRS_DATA_SENT_START_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
    }
}

static void gprs_send_cb(uint16_t param, uint16_t tag)
{
    osel_event_t event;
    
	hal_led_close(RED);
    app_gprs_can_send(TRUE);

	if (TRUE == param)                                            //发送成功
	{
//        printf("GPRS_SEND_OK");
        gw_send_times = 0;
        gprs_list_del();
        uint8_t count = 0;
        list_count(&gprs_cache_head,count);
        //删除队列中元素
        if(!list_empty(&gprs_cache_head))                               //队列中还有数据，继续发送
        {
            event.sig = APP_GPRS_DATA_SENT_START_EVENT;
            event.param = NULL;
            osel_post(NULL, &app_process, &event); 
//            osel_post(APP_GPRS_DATA_SENT_START_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
        }
	}
    else
    {                                                                   //发送失败
        printf("GPRS_SEND_ERROR");
        if(gw_send_times <= GW_SEND_MAX)                                //小于重传次数，继续发送
        {
            gw_send_times ++;
            if(!list_empty(&gprs_cache_head))
            {
                event.sig = APP_GPRS_DATA_SENT_START_EVENT;
                event.param = NULL;
                osel_post(NULL, &app_process, &event); 
//                osel_post(APP_GPRS_DATA_SENT_START_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
            } 
            else
            {
                DBG_ASSERT(FALSE __DBG_LINE);
            }
        }
        else
        {                                                               //达到最大重传次数，删除list中的数据
            gw_send_times = 0;
            gprs_list_del();
            if(!list_empty(&gprs_cache_head))                           //list中还有数据，继续发送
            {
                event.sig = APP_GPRS_DATA_SENT_START_EVENT;
                event.param = NULL;
                osel_post(NULL, &app_process, &event); 
                    
//                osel_post(APP_GPRS_DATA_SENT_START_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
            }
            
        }
    }
}

static void gprs_send_start(void)
{
    if(!app_gprs_can_send_flag())                   //GPRS不能发送
    {
        if(retreat_num < RETREAT_NUM_MAX)
        {
            retreat_num ++;
            app_cycle_gprs_send_timer_start();      //开始退避2分钟
        }
        else
        {
            retreat_num = 0;
            app_gprs_can_send(FALSE);
            gprs_pow_close();
        }
    }
    else
    {
        if(list_empty(&gprs_cache_head))
        {
            app_gprs_can_send(TRUE);
            return;
        }
        pbuf_t *gprs_send_pbuf = NULL;
        gprs_send_pbuf = list_entry_addr_find(list_first_elem_look(&gprs_cache_head), pbuf_t, list);
        if(gprs_send_pbuf != NULL)
        {
            app_gprs_can_send(FALSE);
            hal_led_open(RED);
            gprs_tran_send(gprs_send_pbuf->data_p, gprs_send_pbuf->data_len, UDP_CONTINE, 0);
        }
        else
        {
            DBG_ASSERT(FALSE __DBG_LINE);
            app_gprs_can_send(TRUE);
        }
    }
}

static void gprs_restart_event(void )
{
    app_gprs_can_send(TRUE);
	gprs_pow_open();
    gprs_send_start();
}

static void gprs_config_init(void)
{
	device_info_t device_info = hal_board_info_look();
	gprs_init_cfg_t gprs_cfg;
		
    gprs_cfg.ip_addr = BUILD_IP_ADDRESS(58,214,236,152);
    gprs_cfg.port = 8057;
    gprs_cfg.gprs_send_cb = gprs_send_cb;
    gprs_cfg.gprs_recv_cb = gprs_receive_cb;
    gprs_tran_init(&gprs_cfg);

}

void app_gprs_init(void)
{
	gprs_config_init();
	escort_gprs_init();	
	app_cycle_timer_start();	
}

PROCESS(app_process, "app_process");
PROCESS_THREAD(app_process, ev, data)
{
	PROCESS_BEGIN();
	
	while(1)
	{   
		if(ev == APP_UART_EVENT)
		{
			uart_event_handle();
		}
		else if(ev == APP_CYCLE_TIMER_EVENT)
		{
			app_cycle_timer_cb();
		}
        else if(ev == APP_GPRS_DATA_SENT_START_EVENT)
		{
			gprs_send_start();
		}
        else if(ev == APP_GPRS_RESTART_EVENT)
		{
			 gprs_restart_event();
		}
		else if(ev == APP_RF_DATA_EVENT)
		{
			sbuf_t *sbuf = (sbuf_t *)(data);
            gprs_data_add_list(sbuf->primargs.pbuf->data_p, sbuf->primargs.pbuf->data_len);
            serial_write(HAL_UART_1, sbuf->primargs.pbuf->data_p, sbuf->primargs.pbuf->data_len); 
			pbuf_free(&sbuf->primargs.pbuf __PLINE2);
            sbuf_free(&sbuf __SLINE2);
		}
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}

void nwk2app_deal(sbuf_t *sbuf)	//处理nwk来的数据
{
	osel_event_t event;
	event.sig = APP_RF_DATA_EVENT;
	event.param = sbuf;
	osel_post(NULL, &app_process, &event);
}

static void debug_cfg(void)
{
	mac_info_t *info = mac_get();
	info->mac_pib.mac_addr = NODE_ID;
    
	uint16_t short_addr  = mac_short_addr_get(info->mac_pib.mac_addr);
    SSN_RADIO.set_value(RF_LADDR0,LO_UINT16(short_addr));
	SSN_RADIO.set_value(RF_LADDR1,HI_UINT16(short_addr));
    
	osel_memset(info->ch,0xff,CH_NUM);
	info->ch[0] = CH_SN;
	info->mode = SYNC_S;	//ASYN_S,SYNC_S
	info->drivce_type = NODE_TYPE_GATEWAY;
	supf_spec_t  *supf_cfg = &info->agreement.sync_attribute.supf_cfg_arg;
	memset(supf_cfg,0,sizeof(supf_spec_t));
	supf_cfg->beacon_interv_order = 3000;
	supf_cfg->beacon_duration_order = 15;
	supf_cfg->gts_duration = 16;
	supf_cfg->intra_gts_number = GTS_NUM;
	supf_cfg->cluster_number = 5;				//5跳配置
	supf_cfg->inter_unit_number = 4 ;
	supf_cfg->intra_cap_number = 5;
	uint8_t inter_gts_num[MAX_HOP_NUM] = {6,6,6,6,6,6};
	for(int i=0; i<MAX_HOP_NUM; i++)
	{
		supf_cfg->inter_gts_number[i] = inter_gts_num[i];
	}
	nwk_init();
	nwk2app_register((void *)nwk2app_deal);
	mac_run();
    nwk_run();
}

void app_init(void)
{
	app_task_handle = osel_task_create(NULL, APP_TASK_PRIO, app_event_store, APP_EVENT_MAX);
	osel_pthread_create(app_task_handle, &app_process, NULL);
    osel_etimer_ctor(&cycle_timer_handle, &app_process, APP_CYCLE_TIMER_EVENT, NULL);
	osel_etimer_ctor(&app_cycle_gprs_send_timer, &app_process, APP_GPRS_DATA_SENT_START_EVENT, NULL);
    osel_etimer_ctor(&restart_gprs_send_timer, &app_process, APP_GPRS_RESTART_EVENT, NULL);	
	
    app_gprs_init();
    SSN_RADIO.init();
    discnn_platform();
	list_init(&gprs_cache_head);
    mac_init();
#if DEBUG_DEVICE_INFO_EN == 1
	debug_cfg();
#endif
    uint8_t test_buf[] = {0x40, 0x02, 0x50, 0x00, 0xB8, 0x03, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x74, 0x65, 0x6D, 0x70, 
    0x65, 0x72, 0x61, 0x74, 0x75, 0x72, 0x65, 0xFF, 0x32, 0x30, 0x2E, 
    0x31, 0x31, 0x00};

    gprs_data_add_list(test_buf, sizeof(test_buf));
    serial_write(HAL_UART_1, test_buf, sizeof(test_buf)); 
}