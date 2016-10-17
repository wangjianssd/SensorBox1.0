#include "asyn_terminal.h"
#include "asyn_general.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include "../../general/neighbors.h"
#include "../../general/assos_table.h"
#include "common/hal/hal.h"
asyn_terminal_cb_t asyn_terminal_cb;
static asyn_info_t info;

static mac_state_e get_state(void)
{
	return *info.state;
}
static void set_state(mac_state_e state)
{
	*info.state = state;
}

static void set_asyn_state(bool_t state)
{
	*info.asyn_en = state;
}

static bool_t get_asyn_state()
{
	return *info.asyn_en;
}

static void recv_slot_cb(void *seq_p)
{
	info.attribute->recv_time = hal_timer_now().w;
	if(get_asyn_state())
	{
		hal_led_open(HAL_LED_GREEN);
		m_tran_recv();
	}
}

static void sleep_slot_cb(void *seq_p)
{
	if(get_state() == WORK_ON || get_state() == ASSOC_REQUEST)
	{	
		asyn_time_update(MS_TO_TICK(info.attribute->asyn_cfg_arg.asyn_cycle));
		if(get_asyn_state())
		{
			uint16_t id = 0;
			if(!asyn_send_list_empty(&id))
			{
				info.mac_pib->mac_seq_num = 0;
				send_idle(&id);
			}
		}
	}
	if(get_asyn_state())
	{
		hal_led_close(HAL_LED_GREEN);
		m_tran_sleep();
	}
}

static void idle_slot_cb(void *seq_p)
{
	
}

static void slot_cb_init(void)
{
	info.attribute->time_slot[ASYN_RECV] = recv_slot_cb;
	info.attribute->time_slot[ASYN_SLEEP] = sleep_slot_cb;
	info.attribute->time_slot[ASYN_IDLE] = idle_slot_cb;
}

static void none(pbuf_t *pbuf)
{
	pbuf_free(&pbuf __PLINE2);
}

static void mac_data(pbuf_t *pbuf)
{
	if(pbuf->attri.dst_id == info.mac_pib->self_saddr)
	{
		DBG_ASSERT(pbuf != NULL __DBG_LINE);
		sbuf_t *sbuf = sbuf_alloc(__SLINE1);
		DBG_ASSERT(sbuf != NULL __DBG_LINE);
		
		mac_frm_ctrl_t mac_frm_ctrl;
		mac_prim_arg_t *mac_prim_arg = &(sbuf->primargs.prim_arg.mac_prim_arg);
		osel_memcpy(&mac_frm_ctrl, (pbuf->head + PHY_HEAD_SIZE), MAC_HEAD_CTRL_SIZE);
		
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		mac_prim_arg->msdu_length = pbuf->data_len - PHY_HEAD_SIZE -
			pbuf->attri.mac_length - MAC_FCS_SIZE;
		sbuf->primtype = M2N_DATA_INDICATION;
		sbuf->primargs.pbuf = pbuf;
		
		pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE;
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.des_addr_mode, &mac_prim_arg->dst_addr);
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.src_addr_mode, &mac_prim_arg->src_addr);
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->msdu = pbuf->data_p;
		asyn_terminal_cb.recv_cb(sbuf);
		mac_sched_sleep_timeout_set();
	}
	else
	{
		pbuf_free(&pbuf __PLINE2);
		m_tran_sleep();
		set_asyn_state(TRUE);
	}
}

static void mac_ack(pbuf_t *pbuf)
{
	pbuf_free(&pbuf __PLINE2);
}

static void mac_command(pbuf_t *pbuf)
{
	uint8_t frm_type = 0;
	frm_type = *(uint8_t *)(pbuf->data_p);
	pbuf->data_p++;
	pbuf->data_p++;	//多偏移接收个数1个字节
	if(pbuf->attri.dst_id != info.mac_pib->self_saddr)
	{
		pbuf_free(&pbuf __PLINE2);
		return;
	}
	if(frm_type == MAC_CMD_WAKEUP)
	{
		set_asyn_state(FALSE);
		bool_t pending = FALSE;	
		mac_head_t *mac_frm_ctrl;
		mac_frm_ctrl =(mac_head_t *)(pbuf->head + PHY_HEAD_SIZE);
		if(mac_frm_ctrl->ctrl.frm_pending)	//需要接收数据
		{
//			uint16_t id = 0;
			assoc_info_t *temp = NULL;
			temp = asyn_assos_table_find(pbuf->attri.src_id);
			if(NULL != temp)
			{
				bcn_payload_t beacon_payload;
				osel_memcpy(&beacon_payload, pbuf->data_p, sizeof(bcn_payload_t));
				temp->new_time_stamp =  hal_timer_now().w + beacon_payload.time_stamp + US_TO_TICK(800);
			}
			pending = TRUE;
		}
		else
			DBG_ASSERT(OSEL_FALSE __DBG_LINE);
		uint16_t r_time = MS_TO_TICK(info.attribute->asyn_cfg_arg.asyn_cycle) - (hal_timer_now().w - info.attribute->recv_time);
		sbuf_t *sbuf = mac_auery_request_package(pbuf->attri.src_id,pending,r_time);
		m_tran_send(sbuf, auery_request_txok_cb, 1);
	}
	else if(frm_type == MAC_CMD_WAKEUP_RESP)
	{
		bcn_payload_t beacon_payload;
		osel_memcpy(&beacon_payload, pbuf->data_p, sizeof(bcn_payload_t));
		if(get_state() == WORK_ON || get_state()== ASSOC_REQUEST)
		{
			osel_pthread_exit(info.mac_task,&asyn_process,&asyn_process);
			uint16_t id = pbuf->attri.src_id;
			assoc_info_t *temp = asyn_assos_table_find(id);
			temp->new_time_stamp = hal_timer_now().w + beacon_payload.time_stamp + US_TO_TICK(800);
			sbuf_t *sbuf = asyn_send_list_node_get();
            if(NULL != sbuf)
            {
                set_asyn_state(FALSE);
                m_tran_send(sbuf, data_tx_done, 1);
            }
		}
		else
		{
			neighbor_node_t neighbor_node;
			bcn_payload_t beacon_payload;
			osel_memcpy(&beacon_payload, pbuf->data_p, sizeof(bcn_payload_t));
#if DEBUG_COORD_HOP != 0
			if ((beacon_payload.hops) < DEBUG_COORD_HOP)
			{
				pbuf_free(&pbuf __PLINE2);
				return;
			}
#endif
			memset(&neighbor_node, 0, sizeof(neighbor_node_t));
			neighbor_node.dev_id     = pbuf->attri.src_id;
			neighbor_node.rssi       = pbuf->attri.rssi_dbm;
			neighbor_node.hops       = beacon_payload.hops;
			neighbor_node.time_stamp = hal_timer_now().w - (MS_TO_TICK(info.attribute->asyn_cfg_arg.asyn_cycle) - beacon_payload.time_stamp);
			neighbor_node.coord_index = beacon_payload.index;
			mac_neighbor_node_add(&neighbor_node);
		}
	}
	else if(frm_type == MAC_CMD_ASSOC_RESP)
	{
		sync_attribute_t attribute;
		if(!assoc_response_frame(pbuf,info.mac_pib,&attribute))
		{
			mac_neighbors_node_set_state(pbuf->attri.src_id, FALSE);
			set_state(WORK_DOWN);
		}
		else
		{//关联请求成功
			mac_neighbors_node_set_state(pbuf->attri.src_id, TRUE);
			set_state(WORK_ON);
			asyn_terminal_cb.mac_assoc_cb(TRUE);
		}
		set_asyn_state(TRUE);
	}
	pbuf_free(&pbuf __PLINE2);
}
static void mac_schedule_init(void)
{
	asyn_config(0x00);
	asyn_cfg(info.attribute, SLOT_LOCAL_TIME);
	hal_time_t now;
	now = hal_timer_now();
	now.w += MS_TO_TICK(0);
	m_slot_run(&now);
}

void terminal_init(void)
{
	set_state(READY_IDLE);
	mac_frames_asyn_cb_init();
	asyn_frame_switch_init(none,mac_data,mac_ack,mac_command);
}

static bool_t run(void)
{
	set_asyn_state(TRUE);
	slot_cb_init();
	mac_schedule_init();
	return TRUE;
}
static bool_t stop(void)
{
	set_state(WORK_DOWN);
	osel_pthread_exit(info.mac_task,&asyn_process,&asyn_process);
	if (m_slot_get_state())
	{
		m_slot_stop();
	}
	m_tran_sleep();
	return TRUE;
}
static bool_t terminal_send(sbuf_t *sbuf)
{
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	switch (sbuf->primtype)
	{
	case N2M_DATA_REQUEST:
		{
			mac_data_fill_package(sbuf);
			asyn_send_list_insert(sbuf);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

static asyn_info_t* get(void)
{
	return &info;
}

const struct asyn_terminal_t asyn_terminal =
{
	run,
	stop,
	terminal_send,
	get,
};
