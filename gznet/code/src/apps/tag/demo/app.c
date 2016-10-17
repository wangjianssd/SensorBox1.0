#include "app.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "stack/custom/mac/mac.h"
#include "stack/custom/nwk/nwk_interface.h"
#include "common/dev/dev.h"

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
	nwk_init();
	nwk_run();
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