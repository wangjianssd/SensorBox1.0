#include "common/lib/lib.h"
#include "platform/platform.h"
#include "sync_ctrl.h"
#include "sync_general.h"
#include "sync_mac_package.h"
#include "sync_gateway.h"
#include "sync_terminal.h"
#include "sync_router.h"
#include "../../general/neighbors.h"
#include "../../general/mac_package.h"

static sync_info_t info;
//neighbor_node_t coord;
extern neighbor_node_t coord;
osel_etimer_t assoc_timer;
static uint8_t assoc_request_num = 0;

static bool_t stop(void);
sync_cb_t sync_cb;

static void set_state(mac_state_e state)
{
    *info.state = state;
}

static void assoc_timer_start(uint32_t time)
{
    osel_etimer_arm(&assoc_timer,time/TICK_PRECISION, 0);
}

static void slot_ready(void)
{
    hal_time_t slot_run_time;
    if (!m_slot_get_state())
    {
        slot_run_time.w = coord.time_stamp;
        m_slot_run((hal_time_t *) &slot_run_time);
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
    set_state(ASSOC_REQUEST);
    assoc_request_num = 0;
}

static void assoc_request(void)	//关联请求超时或者关联失败
{
    //??超过3次失败的判断
    if(*info.state == ASSOC_REQUEST)
    {
        if(assoc_request_num++ == 3 || *info.state == WORK_DOWN)
        {
            stop();
        }
        else
        {
            sbuf_t *sbuf = mac_assoc_request_package();
            supf_spec_t supf_cfg_arg = info.sync_attribute->supf_cfg_arg;
            cap_list_insert(&supf_cfg_arg,sbuf);
        }
        return;
    }
    DBG_ASSERT(FALSE __DBG_LINE);
}

static void gateway_config(void)
{
    sync_info_t *gateway_info = gateway.get();
    gateway_info->drivce_type = info.drivce_type;
    gateway_info->state = info.state;
    gateway_info->mac_pib = info.mac_pib;
	gateway_info->sync_attribute = info.sync_attribute;
}

static void terminal_config(void)
{
    sync_info_t *terminal_info = terminal.get();
    terminal_info->drivce_type = info.drivce_type;
    terminal_info->state = info.state;
    terminal_info->mac_pib = info.mac_pib;
	terminal_info->sync_attribute = info.sync_attribute;
}

static void router_config(void)
{
	sync_info_t *router_info = router.get();
    router_info->drivce_type = info.drivce_type;
    router_info->state = info.state;
    router_info->mac_pib = info.mac_pib;
	router_info->sync_attribute = info.sync_attribute;
}

static void cb_register(void)
{
	terminal_cb.recv_cb = router_cb.recv_cb = gateway_cb.recv_cb = sync_cb.recv_cb;
	terminal_cb.send_cb = router_cb.send_cb = gateway_cb.send_cb = sync_cb.send_cb;
	terminal_cb.mac_assoc_cb = router_cb.mac_assoc_cb = gateway_cb.mac_assoc_cb = sync_cb.mac_assoc_cb;
	terminal_cb.mac_restart = router_cb.mac_restart = sync_cb.mac_restart;
}

static bool_t run(void)
{
	m_sync_en(TRUE);
    phy_set_channel(info.sync_attribute->intra_channel);
	info.mac_pib->mac_seq_num = 0;
    sync_mac_package_init();
	mac_neighbors_init();
	cb_register();
    switch(*info.drivce_type)
    {
    case NODE_TYPE_TAG:
        terminal_config();
        terminal.run();
        break;
    case NODE_TYPE_ROUTER:
        router_config();
		router.run();
        break;
    case NODE_TYPE_GATEWAY:
        gateway_config();
        gateway.run();
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }
    return TRUE;
}

static bool_t stop(void)
{
	osel_etimer_disarm(&assoc_timer);
    switch(*info.drivce_type)
    {
    case NODE_TYPE_TAG:
        terminal.stop();
        break;
    case NODE_TYPE_ROUTER:
		router.stop();
        break;
    case NODE_TYPE_GATEWAY:
        gateway.stop();
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }
    return TRUE;
}

static bool_t sync_send(sbuf_t *sbuf)
{
	bool_t mark = FALSE;
    switch(*info.drivce_type)
    {
    case NODE_TYPE_TAG:
        mark = terminal.send(sbuf);
        break;
    case NODE_TYPE_ROUTER:
		mark = router.send(sbuf);
        break;
    case NODE_TYPE_GATEWAY:
		mark = gateway.send(sbuf);
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }
    return mark;
}

static bool_t find_sys_coord(void)
{
   switch(*info.drivce_type)
    {
    case NODE_TYPE_TAG:
    case NODE_TYPE_ROUTER:
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }
    if (!mac_get_coord(&coord))
    {
        m_tran_sleep();
        return FALSE;
    }
    mac_pib_t *mac_pib = info.mac_pib;
    mac_pib->hops = coord.hops + 1;
    mac_pib->down_rssi = coord.rssi;
    info.sync_attribute->coord_cluster_index = coord.coord_index;
    mac_pib->coord_saddr = coord.dev_id;
    info.sync_attribute->intra_channel = coord.intra_channel;
    info.sync_attribute->supf_cfg_arg = coord.supf_cfg_arg;
//    osel_memcpy(&info.sync_attribute->supf_cfg_arg, &coord.supf_cfg_arg, sizeof(supf_spec_t));
    super_frame_cfg(info.sync_attribute, SLOT_GLOBAL_TIME);
    sync_config(mac_pib->coord_saddr);
    phy_set_channel(info.sync_attribute->intra_channel);
    m_tran_recv();
    return TRUE;
}

static void assoc_switch(void)		/**< 关联流程状态机 */
{
	uint32_t time = TICK_TO_US(beacon_interval.slot_duration)/1000;
    switch(*info.state)
    {
    case COORD_READY:
        set_state(SLOT_READY);
		assoc_timer_start(time*2);
        break;
    case SLOT_READY:
        slot_ready();
        assoc_timer_start(300);
        break;
    case ASSOC_REQUEST:
        assoc_request();
		assoc_timer_start(time*3);
		break;
	case ASSOC_RESPONSE:
        break;
    default :
        break;
    }
}


static sync_info_t* get(void)
{
    return &info;
}

const struct sync_t m_sync =
{
    run,
    stop,
    sync_send,
    find_sys_coord,
    assoc_switch,
    get,
};
