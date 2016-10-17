#include "app.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "stack/custom/mac/mac.h"
#include "stack/custom/nwk/nwk_interface.h"
#include "common/dev/dev.h"
#include "common/coap/coap_client.h"

#define APP_TASK_PRIO       (4u)
typedef enum
{
    APP_AT_RECV =  ((APP_TASK_PRIO<<8) | 0x01),
	APP_AT_SEND,
    APP_RF_DATA_EVENT,
} app_task_sig_enum_t;

#define APP_EVENT_MAX       (6u)   //*< 最多处理10个事件
static osel_event_t app_event_store[APP_EVENT_MAX];
static osel_task_t *app_task_handle = NULL;
static osel_etimer_t coap_et;

/**< coap */
/**
 * @brief CoAP请求发送提示
 * @param[in]  sbuf_t *sbuf
 * @return 无
 */
static void coap_msg_send(sbuf_t *sbuf)
{
    _NOP();
}

/**
 * @brief 处理COAP响应
 * @param[in]  data
 * @param[in]  len
 * @return 无
 */
static void coap_msg_recv(uint8_t *const data, uint8_t len)
{
    _NOP();
}

nwk_dependent_t nwk_dependent =
{
    coap_msg_send,
    coap_msg_recv,
    NULL
};


#define SERVER_NUI  (0x00000003ull)
void coap_put_led(uint8_t status)
{
    // 第一步，声明发送Buf和接收Buf，最大长度均为64字节
    uint8_t msg_send_buf[64];
    coap_pdu_t msg_send = {msg_send_buf, 0, 64};
    uint8_t msg_recv_buf[64];
    coap_pdu_t msg_recv = {msg_recv_buf, 0, 64};

    // 第二步，辅助步骤，初始化发送Buf
    coap_init_pdu(&msg_send);

    // 第三步，设置CoAP Header部分四字节
    coap_set_version(&msg_send, COAP_V1);
    coap_set_type(&msg_send, CT_CON);
    coap_set_code(&msg_send, CC_PUT);
    coap_set_mid(&msg_send, 0x6000);            // Message ID 任意

    // 第四步，加入路由 Uri-Path
    uint64_t nui = NODE_ID;
    coap_add_option(&msg_send, CON_URI_PATH, (uint8_t*)"led", 3);
    
    // 第五步，01代表打开LED 00代表关闭LED
    coap_set_payload(&msg_send, (uint8_t*)&status, 1);
    
    // 第六步，向SSN网络中的SERVER发送
    nwk_send(SERVER_NUI, msg_send.buf, msg_send.len);
}

PROCESS(app_coap_client_process, "app_coap_client_process");
PROCESS_THREAD(app_coap_client_process, ev, data)
{
    PROCESS_BEGIN();
    
    nwk_dependent_cfg(&nwk_dependent);
    while (1)
    {
        OSEL_ETIMER_DELAY(&coap_et, 10000/TICK_PRECISION); 
        hal_led_toggle(GREEN);
        coap_put_led(1);
        
        OSEL_ETIMER_DELAY(&coap_et, 10000/TICK_PRECISION); 
        hal_led_toggle(GREEN);
        coap_put_led(0);
    }
    
    PROCESS_END();
}

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
	info->drivce_type = NODE_TYPE_TAG;
	nwk_run();
	mac_run();
}

void app_init(void)
{
	app_task_handle = osel_task_create(NULL, APP_TASK_PRIO, app_event_store, APP_EVENT_MAX);
	osel_pthread_create(app_task_handle, &app_process, NULL);
	
	osel_pthread_create(app_task_handle, &app_coap_client_process, NULL);
	nwk_init();	
	mac_init();
#if DEBUG_DEVICE_INFO_EN == 1
	debug_cfg();
#endif
	
}