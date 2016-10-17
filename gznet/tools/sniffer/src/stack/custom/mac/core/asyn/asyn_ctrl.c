
#include <wsnos.h>
#include <phy_state.h>
#include <hal_board.h>
#include "asyn_gateway.h"
#include "asyn_router.h"
#include "asyn_terminal.h"
#include "../../general/neighbors.h"
#include "../../general/mac_package.h"

#include "asyn_ctrl.h"
/**< 定义私有变量 */
asyn_cb_t asyn_cb;
static asyn_info_t info;
static osel_task_t *mac_task = NULL;
static osel_etimer_t task_auery_time_evt;
static mac_state_e get_state(void)
{
    return *info.state;
}
static void set_state(mac_state_e state)
{
    *info.state = state;
}

static void auery_tx_done(sbuf_t *sbuf, bool_t result)
{
	if (sbuf->orig_layer == MAC_LAYER)
    {
        if ((sbuf != NULL) && (sbuf->used))
        {
            if (sbuf->primargs.pbuf->used)
            {
                pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
            }
            sbuf_free(&sbuf __SLINE2);
        }
    }
    else
        DBG_ASSERT(FALSE __DBG_LINE);
}

PROCESS(auery_process, "auery_process");
PROCESS_THREAD(auery_process, ev, data)
{
	sbuf_t *sbuf = NULL;
	uint8_t time = BASE_AUERY/TICK_PRECISION;
    PROCESS_BEGIN();
	osel_etimer_ctor(&task_auery_time_evt, &auery_process, ASSOC_EVENT, NULL);
	mac_neighbors_init();
	static uint8_t send_num = 0;
	send_num = 0;
    while(1)
    {   
		/**< 发送对准 */      
		if(send_num++ < DOCKING)
		{
			sbuf = mac_auery_package(MAC_BROADCAST_ADDR);
			osel_etimer_arm(&task_auery_time_evt, time, 0);
			m_tran_send(sbuf, auery_tx_done, 1);
			PROCESS_WAIT_EVENT_UNTIL(ev == ASSOC_EVENT);
		}
		else
		{
			PROCESS_EXIT();
		}
    }
    PROCESS_END();
}

static void auery(void)
{
	osel_pthread_create(mac_task, &auery_process, NULL);
}

static void gateway_config(void)
{
    asyn_info_t *gateway_info = asyn_gateway.get();
    gateway_info->drivce_type = info.drivce_type;
    gateway_info->state = info.state;
    gateway_info->asyn_pib = info.asyn_pib;
}

static void terminal_config(void)
{
    asyn_info_t *terminal_info = asyn_terminal.get();
    terminal_info->drivce_type = info.drivce_type;
    terminal_info->state = info.state;
    terminal_info->asyn_pib = info.asyn_pib;
}

static void router_config(void)
{
    asyn_info_t *router_info = asyn_router.get();
    router_info->drivce_type = info.drivce_type;
    router_info->state = info.state;
    router_info->asyn_pib = info.asyn_pib;
}

static void cb_register(void)
{
    asyn_terminal_cb.recv_cb = asyn_router_cb.recv_cb = asyn_gateway_cb.recv_cb = asyn_cb.recv_cb;
    asyn_terminal_cb.send_cb = asyn_router_cb.send_cb = asyn_gateway_cb.send_cb = asyn_cb.send_cb;
    asyn_terminal_cb.mac_assoc_cb = asyn_router_cb.mac_assoc_cb = asyn_gateway_cb.mac_assoc_cb = asyn_cb.mac_assoc_cb;
    asyn_terminal_cb.mac_restart = asyn_router_cb.mac_restart = asyn_cb.mac_restart;
}

static void init(void)
{
	DBG_ASSERT(mac_task!=NULL __DBG_LINE);
}

static bool_t run(void)
{
    phy_set_channel(info.asyn_pib->intra_channel);
	init();
    mac_neighbors_init();
    cb_register();
	
    switch(*info.drivce_type)
    {
    case TERMINAL:
        terminal_config();
        //asyn_terminal.run();
		auery();
        break;
    case ROUTER:
        router_config();
        //asyn_router.run();
		auery();
        break;
    case GATEWAY:
        gateway_config();
        asyn_gateway.run();
        break;
    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }
    return TRUE;
}

static bool_t stop(void)
{
    return TRUE;
}

static bool_t asyn_send(sbuf_t *sbuf)
{
    return TRUE;
}

static bool_t find_sys_coord(void)
{
    return FALSE;
}

static void mac_task_register(osel_task_t *task)		/**< 关联流程状态机 */
{
	mac_task = task;
}

static asyn_info_t* get(void)
{
    return &info;
}

const struct asyn_t asyn =
{
    run,
    stop,
    asyn_send,
    find_sys_coord,
    mac_task_register,
    get,
};
