#include "asyn_ctrl.h"
#include "sys_arch/osel_arch.h"
#include "stack/common/prim.h"

#include "stack/common/prim.h"
#include "stack/custom/phy/phy_state.h"
#include "stack/custom/phy/phy_cca.h"

#include "common/hal/hal.h"

DBG_THIS_MODULE("asyn_ctrl")

#include "asyn_gateway.h"
#include "asyn_router.h"
#include "asyn_terminal.h"
#include "asyn_general.h"
#include "../../general/neighbors.h"
#include "../../general/mac_package.h"
#include "../../general/assos_table.h"
/**< 定义私有变量 */
asyn_cb_t asyn_cb;
static asyn_info_t info;

static osel_etimer_t task_auery_time_evt;
static osel_etimer_t assoc_timer;
static hal_timer_t *recv_timer = NULL;
static uint8_t assoc_request_num = 0;
static volatile bool_t asyn_en = TRUE;
static hal_timer_t *sleep_timer= NULL;
static hal_timer_t *send_idle_timer= NULL;

static bool_t stop(void);
#if 0
static mac_state_e get_state(void)
{
	return *info.state;
}
#endif
static void set_state(mac_state_e state)
{
	*info.state = state;
}

static void set_asyn_state(bool_t state)
{
	asyn_en = state;
}

static void sleep_over_cb(void *p)
{
	sleep_timer = NULL;
	if(*info.drivce_type != NODE_TYPE_GATEWAY)
	{
		m_tran_sleep();
	}
	set_asyn_state(TRUE);
}

void mac_sched_sleep_timeout_set(void)
{
	uint16_t time = 4000;
	if(sleep_timer != NULL)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
		hal_timer_cancel(&sleep_timer);
	}
	HAL_TIMER_SET_REL(US_TO_TICK(time), sleep_over_cb, NULL, sleep_timer);
	DBG_ASSERT(sleep_timer != NULL __DBG_LINE);
}

static void recv_over_cb(void *p)
{
	recv_timer = NULL;
	if(!tran_rx_sfd_get())
	{
		if(*info.drivce_type != NODE_TYPE_GATEWAY)
		{
			m_tran_sleep();
		}
		set_asyn_state(TRUE);
	}
}

void mac_sched_recv_timeout_set(void)
{
	uint16_t time = 4000;
	
	if(recv_timer != NULL)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
		hal_timer_cancel(&recv_timer);
	}
	HAL_TIMER_SET_REL(US_TO_TICK(time), recv_over_cb, NULL, recv_timer);
	DBG_ASSERT(recv_timer != NULL __DBG_LINE);
}

static void assoc_timer_start(uint32_t time)
{
	osel_etimer_arm(&assoc_timer,time/TICK_PRECISION, 0);
}

static void assoc_request(void)	//关联请求超时或者关联失败
{
	//??超过3次失败的判断
	if(*info.state == ASSOC_REQUEST)
	{
		if(assoc_request_num++ >= 3 || *info.state == WORK_DOWN)
		{
			stop();
		}
		else
		{
			sbuf_t *sbuf = mac_assoc_request_package();
			asyn_send_list_insert(sbuf);
		}
		return;
	}
	DBG_ASSERT(FALSE __DBG_LINE);
}

static void auery_tx_done(sbuf_t *sbuf, bool_t result)
{
	m_tran_recv();
	mac_sched_recv_timeout_set();
	pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
	sbuf_free(&sbuf __SLINE2);
}

void auery_request_txok_cb(sbuf_t *sbuf, bool_t result)
{
	if(sbuf->primargs.pbuf->attri.is_pending)
	{
		set_asyn_state(FALSE);
		m_tran_recv();
		mac_sched_recv_timeout_set();
	}
	else
	{
		if(*info.drivce_type != NODE_TYPE_GATEWAY)
		{
			m_tran_sleep();
		}
		set_asyn_state(TRUE);
	}
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

void data_tx_done(sbuf_t *sbuf, bool_t result)
{
	if(sbuf->primargs.pbuf->attri.is_pending)
	{
		set_asyn_state(FALSE);
		m_tran_recv();
		mac_sched_recv_timeout_set();
	}
	else
	{
		if(*info.drivce_type != NODE_TYPE_GATEWAY)
		{
			m_tran_sleep();
		}
		set_asyn_state(TRUE);
	}
	if(sbuf->orig_layer != MAC_LAYER && result == FALSE)
	{//重传处理
		NOP();
	}
	else
	{
		pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
		sbuf_free(&sbuf __SLINE2);
	}
}

PROCESS(asyn_process, "asyn_process");
static uint16_t send_id = 0;
PROCESS_THREAD(asyn_process, ev, data)
{
	sbuf_t *sdata;
	uint8_t time = info.attribute->asyn_cfg_arg.duration;
	uint32_t r_time= 0;
	PROCESS_BEGIN();
	static uint8_t i =0;
	i = 0;
	while(TRUE)
	{
		/**< 发送对准 */ 
		for(; i<8; i++)
		{
			r_time = MS_TO_TICK(info.attribute->asyn_cfg_arg.asyn_cycle) - (hal_timer_now().w - info.attribute->recv_time);
			if(!phy_cca())
			{
				set_asyn_state(TRUE);
				PROCESS_EXIT();
			}
			sdata = mac_auery_package(send_id, TRUE, r_time);
			set_asyn_state(FALSE);
			m_tran_send(sdata, data_tx_done, 1);
			OSEL_ETIMER_DELAY(&asyn_timer_ev, time/TICK_PRECISION);
			info.attribute->recv_time-=time;
		}
		if(!asyn_send_list_empty(&send_id))
		{
			sbuf_t *sbuf = asyn_send_list_node_get();
			if(sbuf->primargs.pbuf->attri.already_send_times.mac_send_times++>=3)
			{
				up_down_link_t e = sbuf->up_down_link;
				pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
				sbuf_free(&sbuf __SLINE2);
				if(e == UP_LINK)
				{
					asyn_cb.mac_restart(ASYN_S);
				}
			}
			else
			{
				asyn_send_list_insert(sbuf);
			}
		}
		set_asyn_state(TRUE);
		PROCESS_EXIT();
	}
	
	PROCESS_END();
}

static void send_idle_cb(void *p)
{
	send_idle_timer = NULL;
	osel_event_t event;
	event.sig = ASYN_EVENT;
	event.param = NULL;
	set_asyn_state(FALSE);
	osel_post(NULL, &mac_event_process, &event);
}

void send_idle(uint16_t *id)
{
	if(send_idle_timer != NULL)
	{
		return;
	}
	send_id = *id;
	assoc_info_t *temp = NULL;
//	uint16_t delay_time = 0;
	temp = asyn_assos_table_find(send_id);
	int32_t diff = 0;
	diff = temp->time_stamp - (hal_timer_now().w);
	
	HAL_TIMER_SET_REL(diff, send_idle_cb, NULL, send_idle_timer);
	DBG_ASSERT(send_idle_timer != NULL __DBG_LINE);
}

PROCESS_NAME(auery_process);
PROCESS(auery_process, "auery_process");
PROCESS_THREAD(auery_process, ev, data)
{
	sbuf_t *sbuf = NULL;
	uint8_t time = info.attribute->asyn_cfg_arg.auery_duration;
	PROCESS_BEGIN();
	osel_etimer_ctor(&task_auery_time_evt, PROCESS_CURRENT(), ASSOC_EVENT, NULL);
	static assoc_info_t *temp;
	static uint8_t send_num = 0,cycle = 0;
	send_num = 0;
	cycle = 2;
	while(1)
	{   
		/**< 发送对准 */ 
		for(; send_num<info.attribute->asyn_cfg_arg.auery_order; send_num++)
		{
			sbuf = mac_auery_package(MAC_BROADCAST_ADDR, FALSE, 0xffff);
			m_tran_send(sbuf, auery_tx_done, 1);
			OSEL_ETIMER_DELAY(&task_auery_time_evt, time/TICK_PRECISION);
		}
		if(--cycle > 0)
		{
			send_num = 0;
			asyn_update_cycle(MS_TO_TICK((time)*info.attribute->asyn_cfg_arg.auery_order));
		}
		else
		{
			m_tran_sleep();
			asyn_update_cycle(MS_TO_TICK(time*info.attribute->asyn_cfg_arg.auery_order));
			if (mac_get_coord(&coord))
			{
				info.mac_pib->coord_saddr = coord.dev_id;
				info.mac_pib->hops = coord.hops + 1;
				info.mac_pib->down_rssi = coord.rssi;
				assoc_info_t id;
				osel_memset(&id,0,sizeof(assoc_info_t));
				id.mac_addr = coord.dev_id;
				id.device_type = (coord.hops == 0)?NODE_TYPE_GATEWAY:NODE_TYPE_ROUTER;
				id.time_stamp = coord.time_stamp;
				mac_assoc_state_e state;
				asyn_assos_table_add(id,&state);
				temp = asyn_assos_table_find(info.mac_pib->coord_saddr);
				switch(*info.drivce_type)
				{
				case NODE_TYPE_TAG:
					asyn_terminal.run();
					break;
				case NODE_TYPE_ROUTER:
					asyn_router.run();
					break;
				}
				OSEL_ETIMER_DELAY(&task_auery_time_evt, 1000/TICK_PRECISION);
				temp->time_stamp+=MS_TO_TICK(1000);
				set_state(ASSOC_REQUEST);
			}
			else
				set_state(READY_IDLE);
			PROCESS_EXIT();
		}
	}
	PROCESS_END();
}

static void auery(void)
{
	osel_pthread_create(info.mac_task, &auery_process, NULL);
}

static void gateway_config(void)
{
	asyn_info_t *gateway_info = asyn_gateway.get();
	gateway_info->drivce_type = info.drivce_type;
	gateway_info->state = info.state;
	gateway_info->mac_task = info.mac_task;
	gateway_info->mac_pib = info.mac_pib;
	gateway_info->attribute = info.attribute;
	gateway_info->asyn_en = info.asyn_en;
}

static void terminal_config(void)
{
	asyn_info_t *terminal_info = asyn_terminal.get();
	terminal_info->drivce_type = info.drivce_type;
	terminal_info->state = info.state;
	terminal_info->mac_task = info.mac_task;
	terminal_info->mac_pib = info.mac_pib;
	terminal_info->attribute = info.attribute;
	terminal_info->asyn_en = info.asyn_en;
}

static void router_config(void)
{
	asyn_info_t *router_info = asyn_router.get();
	router_info->drivce_type = info.drivce_type;
	router_info->state = info.state;
	router_info->mac_task = info.mac_task;
	router_info->mac_pib = info.mac_pib;
	router_info->attribute = info.attribute;
	router_info->asyn_en = info.asyn_en;
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
	DBG_ASSERT(info.mac_task!=NULL __DBG_LINE);
	assoc_request_num = 0;
	info.asyn_en = &asyn_en;
	m_sync_en(FALSE);
	phy_set_channel(info.attribute->intra_channel);
}

static bool_t run(void)
{
	init();
	mac_neighbors_init();
	asyn_general_init();
	cb_register();
	switch(*info.drivce_type)
	{
	case NODE_TYPE_TAG:
		terminal_config();
		terminal_init();
		auery();
		break;
	case NODE_TYPE_ROUTER:
		router_config();
		router_init();
		auery();
		break;
	case NODE_TYPE_GATEWAY:
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
	osel_pthread_exit(info.mac_task,&asyn_process,&asyn_process);
	osel_pthread_exit(info.mac_task,&auery_process,&auery_process);
	info.mac_pib->mac_seq_num = 0;
	switch(*info.drivce_type)
	{
	case NODE_TYPE_TAG:
		asyn_terminal.stop();
		break;
	case NODE_TYPE_ROUTER:
		asyn_router.stop();
		break;
	case NODE_TYPE_GATEWAY:
		asyn_gateway.stop();
		break;
	default:
		DBG_ASSERT(FALSE __DBG_LINE);
		break;
	}
	return TRUE;
}

static bool_t asyn_send(sbuf_t *sbuf)
{
	bool_t mark = FALSE;
	switch(*info.drivce_type)
	{
	case NODE_TYPE_TAG:
		mark = asyn_terminal.send(sbuf);
		break;
	case NODE_TYPE_ROUTER:
		mark = asyn_router.send(sbuf);
		break;
	case NODE_TYPE_GATEWAY:
		mark = asyn_gateway.send(sbuf);
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
	return TRUE;
}

static void assoc_switch(void)
{
//	uint32_t time = info.attribute->asyn_cfg_arg.asyn_cycle;
	switch(*info.state)
	{
	case ASSOC_REQUEST:
		phy_set_channel(info.attribute->intra_channel);
		assoc_request();
		assoc_timer_start(info.attribute->asyn_cfg_arg.asyn_cycle*20);
		break;
	}
}

static void mac_task_register(osel_task_t *task)		/**< 关联流程状态机 */
{
	info.mac_task = task;
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
	assoc_switch,
	mac_task_register,
	get,
};
