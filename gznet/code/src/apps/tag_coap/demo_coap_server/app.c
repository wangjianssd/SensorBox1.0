#include "app.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "stack/custom/mac/mac.h"
#include "stack/custom/nwk/nwk_interface.h"
#include "common/dev/dev.h"
#include "common/coap/coap_server.h"

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
	coap_packet_t request_pkt;
	uint8_t response_data[48];               // 发送缓冲区
	uint8_t scratch_raw[48];
	coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
	int rc;
	int response_len;
	// coap_parse把从UDP获得的二进制请求转换为CoAP结构体
	if (0 != (rc = coap_parse(&request_pkt, data, len)))
	{
		// printf("Bad packet rc=%d\n", rc); 
		_NOP();
	}
	else
	{
		response_len = sizeof(response_data);
		coap_packet_t response_pkt;
		
		// 处理请求，获得response_pkt
		coap_handle_req(&scratch_buf, &request_pkt, &response_pkt);
		
		if (0 != (rc = coap_build(response_data, (size_t*)&response_len, &response_pkt)))
		{
			// printf("coap_build failed rc=%d\n", rc);
		}
		else
		{
			// 通过SSN发送
			// sendto(fd, (char*)response_data, response_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
		}
	}
}

nwk_dependent_t nwk_dependent =
{
	coap_msg_send,
	coap_msg_recv,
	NULL
};

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
	mac_init();
	nwk_init();
	nwk_dependent_cfg(&nwk_dependent);      // 注册
	endpoint_setup();
#if DEBUG_DEVICE_INFO_EN == 1
	debug_cfg();
#endif
}