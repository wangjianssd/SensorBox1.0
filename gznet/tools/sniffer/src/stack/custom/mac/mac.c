#include "mac.h"
#include <osel_arch.h>
#include <mac_module.h>
#include <driver.h>
#include "core/sync/sync_ctrl.h"
#include "general/mac_package.h"
#include "general/assos_table.h"

#define MAC_EVENT_MAX       (20u)   //*< 最多处理10个事件
static osel_event_t mac_event_store[MAC_EVENT_MAX];
static osel_task_t *mac_task_tcb = NULL;

const uint16_t SCAN_CYCLE = 10*1000;

static mac_info_t info;
static osel_etimer_t mac_switch_timer;
static osel_etimer_t mac_maintain_timer;
static osel_etimer_t mac_restart_timer;
static bool_t gznet_run = FALSE;
mac_dependent_t mac_cb;

PROCESS_NAME(mac_event_process);


static mac_state_e get_state(void)
{
	return info.state;
}
static void set_state(const mac_state_e state)
{
    info.state = state;
}

static void mac_switch_timer_start(uint32_t time)
{
    osel_etimer_arm(&mac_switch_timer,time/TICK_PRECISION, 0);
}

static void mac_maintain_timer_start(void)
{
	osel_etimer_arm(&mac_maintain_timer,50, SCAN_CYCLE*4/TICK_PRECISION);
}

static void mac_switch(void)     /**< 启动网络切换 */
{
    static uint8_t ch_index = 0;
    if(ch_index == CH_NUM)
    {
        ch_index = 0;
        if(info.mode == SYNC_S && info.drivce_type != GATEWAY)
        {
            if(!sync.find_sys_coord())
            {
                sync.stop();
            }
            else
            {
                set_state(COORD_READY);
                sync.assoc_switch();
            }
        }
        return;
    }
	
    if(info.mode == RANDOM_S && (get_state() == READY_IDLE || get_state() == WORK_DOWN))
    {
        if(info.current_mode == NON_S)
        {
            info.current_mode = ASYN_S;
			info.agreement.asyn_pib.intra_channel = info.ch[ch_index];
			asyn.run();
            mac_switch_timer_start(BASE*ASYN_CYCLE*DOCKING + 1000);
        }
        else if(info.current_mode == ASYN_S)
        {
            info.current_mode = SYNC_S;
            info.agreement.mac_pib.intra_channel = info.ch[ch_index];;
            sync.run();
            mac_switch_timer_start(SCAN_SYNC_TIME);
        }
        else if(info.current_mode == SYNC_S)
        {
            ch_index++;
            info.current_mode = NON_S;
            mac_switch_timer_start(10);
        }
    }
    else if(info.mode == ASYN_S && (get_state() == READY_IDLE || get_state() == WORK_DOWN))
    {
        info.current_mode = ASYN_S;
		info.agreement.asyn_pib.intra_channel = info.ch[ch_index++];
        asyn.run();
		mac_switch_timer_start(BASE*ASYN_CYCLE*DOCKING + 1000);
    }
    else if(info.mode == SYNC_S && (get_state() == READY_IDLE || get_state() == WORK_DOWN))
    {
        info.current_mode = SYNC_S;
        info.agreement.mac_pib.intra_channel = info.ch[ch_index++];
        sync.run();
        mac_switch_timer_start(SCAN_SYNC_TIME);
    }
    else
    {
		
    }
}

static void ready_idle(void)
{
    set_state(READY_IDLE);
    info.current_mode = NON_S;
    switch(info.mode)
    {
    case ASYN_S:
    case SYNC_S:
    case RANDOM_S:
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
    }
    mac_package_t mac_package;
    mac_package.drivce_type = &info.drivce_type;
    mac_package.mode = &info.mode;
	mac_package.agreement.mac_pib = &info.agreement.mac_pib;
	mac_package_init(&mac_package);
    //异步初始化
	asyn.mac_task_register(mac_task_tcb);
    asyn_info_t *asyn_info;
    asyn_info = asyn.get();
    asyn_info->drivce_type = &info.drivce_type;
    asyn_info->state = &info.state;
    asyn_info->asyn_pib = &info.agreement.asyn_pib;
    //同步初始化
    sync_info_t *sync_info;
    sync_info = sync.get();
    sync_info->drivce_type = &info.drivce_type;
    sync_info->state = &info.state;
    sync_info->mac_pib = &info.agreement.mac_pib;

    mac_maintain_timer_start();
}

static void mac_restart_deal(void)
{
	mac_stop();
	mac_cb.mac_assoc_cb(FALSE);
}

static void mac_restart(net_mode_e mode)
{
    osel_event_t event;
    event.sig = MAC_RESTART_EVENT;
    event.param = NULL;
    osel_post(NULL, &mac_event_process, &event);
}

static bool_t cb_register(void)
{
	if(mac_cb.recv_cb==NULL || mac_cb.send_cb==NULL || mac_cb.mac_assoc_cb==NULL)
		return FALSE;
	sync_cb.recv_cb = mac_cb.recv_cb;
    sync_cb.send_cb = mac_cb.send_cb;
    sync_cb.mac_assoc_cb = mac_cb.mac_assoc_cb;
	sync_cb.mac_restart = mac_restart;
	return TRUE;
}

PROCESS(mac_event_process, "mac_event_process");
PROCESS_THREAD(mac_event_process, ev, data)
{
    PROCESS_BEGIN();
    
    while(1)
    {   
        if(ev == MAC_SWITCH_EVENT)
        {
            if(get_state() == WORK_DOWN) 
                ready_idle();
            else    
                mac_switch();
        }
        else if(ev == MAC_MAINTAIN_NETWORK)
        {
            mac_switch();
        }
        else if(ev == ASSOC_EVENT)
        {
            if(info.current_mode == SYNC_S)
				sync.assoc_switch();
        }
        else if(ev == MAC_RESTART_EVENT)
        {
            if(get_state() == WORK_DOWN)
            {
                gznet_run = FALSE; 
                mac_run();
            }
            else	
                mac_restart_deal();
        }
        else
        {
        
        }
        PROCESS_YIELD();
    }
    
    PROCESS_END();
}


static void sys_enter_lpm_handler(void *p)
{
    debug_info_printf();
//    LPM3;
}

bool_t mac_init(void)
{
	info.agreement.mac_pib.mac_addr = NODE_ID;
	info.ch[0] = CH_SN;
    info.mode = SYNC_S;	//SYNC_S
#if NODE_TYPE == NODE_TYPE_COLLECTOR
    info.drivce_type = GATEWAY;
#elif NODE_TYPE == NODE_TYPE_DETECTOR
    info.drivce_type = TERMINAL;
#elif NODE_TYPE == NODE_TYPE_ROUTER
	info.drivce_type = ROUTER;
#endif

    mac_task_tcb = osel_task_create(NULL, MAC_TASK_PRIO, mac_event_store, MAC_EVENT_MAX);
    
    osel_pthread_create(mac_task_tcb, &mac_event_process, NULL);
    
    m_tran_init(mac_task_tcb);  //需要进行反初始化
	m_sync_init(mac_task_tcb);
	m_slot_init(mac_task_tcb);
	
	osel_etimer_ctor(&mac_switch_timer, &mac_event_process, MAC_SWITCH_EVENT, NULL);
	osel_etimer_ctor(&assoc_timer, &mac_event_process, ASSOC_EVENT, NULL);
	osel_etimer_ctor(&mac_restart_timer, &mac_event_process, MAC_RESTART_EVENT, NULL);
	osel_etimer_ctor(&mac_maintain_timer, &mac_event_process, MAC_MAINTAIN_NETWORK, NULL);
    
	return TRUE;
}

bool_t mac_run(void)
{
	osel_idle_hook(sys_enter_lpm_handler);
	if(gznet_run == TRUE)
	{
		osel_etimer_arm(&mac_restart_timer,200, 0);
		return TRUE;
	}
	if(!cb_register())  DBG_ASSERT(FALSE __DBG_LINE);
    info.agreement.mac_pib.self_saddr = info.agreement.asyn_pib.self_saddr = mac_short_addr_get(info.agreement.mac_pib.mac_addr);
	
	assos_table_init();
    ready_idle();
    
    return TRUE;
}

bool_t mac_stop(void)
{
	gznet_run = TRUE;
	osel_etimer_disarm(&mac_switch_timer);
	osel_etimer_disarm(&mac_maintain_timer);
	asyn.stop();
    sync.stop();
    set_state(WORK_DOWN);
    return TRUE;
}

bool_t mac_send(sbuf_t *sbuf)
{
	bool_t mark = FALSE;
    switch(info.current_mode)
    {
    case ASYN_S:
        break;
    case SYNC_S:
		mark = sync.send(sbuf);
    default:
        return mark;
    }
    return mark;
}

mac_info_t* mac_get(void)
{
    return &info;
}

/**
mac_info_t *info = mac_get();
info->agreement.mac_pib.mac_addr = NODE_ID;
info->ch[0] = 5;
info->mode = SYNC_S;
mac_dependent_t mac_cfg;
mac_cfg.mac_assoc_cb = mac_assoc_cb;
mac_cfg.send_cb = send_cb;
mac_cfg.recv_cb = recv_cb;
mac_dependent_cfg(&mac_cfg);
#if NODE_TYPE == NODE_TYPE_COLLECTOR
info->drivce_type = GATEWAY;
#elif NODE_TYPE == NODE_TYPE_DETECTOR
info->drivce_type = TERMINAL;
#elif NODE_TYPE == NODE_TYPE_ROUTER
info->drivce_type = ROUTER;
#endif
mac_init();
mac_run();
**/
