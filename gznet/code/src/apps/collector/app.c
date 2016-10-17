#include "app.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "../../stack/custom/mac/mac.h"
#include "../../common/dev/dev.h"

#define APP_TASK_PRIO       (5u)
typedef enum
{
    APP_AT_RECV =  ((APP_TASK_PRIO<<8) | 0x01),
	APP_AT_SEND,
    APP_RF_DATA_EVENT,
} app_task_sig_enum_t;

#define APP_EVENT_MAX       (6u)   //*< 最多处理10个事件
static osel_event_t app_event_store[APP_EVENT_MAX];
static osel_task_t *app_task_handle = NULL;

PROCESS(app_process, "app_process");
PROCESS_THREAD(app_process, ev, data)
{
	PROCESS_BEGIN();
	
	while(1)
	{   
		if(ev == APP_AT_RECV)
		{
			
		}
		else if(ev == APP_AT_SEND)
		{
			
		}
		else if(ev == APP_RF_DATA_EVENT)
		{
			
		}
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}

static void mac_assoc_cb(bool_t state)
{
}
static void send_cb(sbuf_t *sbuf,bool_t state)
{
	pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
	sbuf_free(&sbuf __SLINE2);
}
static void recv_cb(sbuf_t *sbuf)
{
	pbuf_t *pbuf = sbuf->primargs.pbuf;
	DBG_ASSERT(pbuf != NULL __DBG_LINE);
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
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
	mac_dependent_t mac_cfg;
	mac_cfg.mac_assoc_cb = mac_assoc_cb;
	mac_cfg.send_cb = send_cb;
	mac_cfg.recv_cb = recv_cb;
	mac_dependent_cfg(&mac_cfg);
	info->drivce_type = NODE_TYPE_GATEWAY;
	supf_spec_t  *supf_cfg = &info->agreement.sync_attribute.supf_cfg_arg;
	memset(supf_cfg,0,sizeof(supf_spec_t));
	supf_cfg->beacon_interv_order = 600;
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
	mac_run();
}

void app_init(void)
{
	app_task_handle = osel_task_create(NULL, APP_TASK_PRIO, app_event_store, APP_EVENT_MAX);
	osel_pthread_create(app_task_handle, &app_process, NULL);
	
	mac_init();
#if DEBUG_DEVICE_INFO_EN == 1
	debug_cfg();
#endif
	
}