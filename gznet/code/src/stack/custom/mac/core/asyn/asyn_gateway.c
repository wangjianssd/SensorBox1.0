//#include "rtimer_arch.h"
#include "asyn_gateway.h"
#include "asyn_general.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include "../../general/assos_table.h"
#include "../../general/general.h"
#include "common/hal/hal.h"
asyn_gateway_cb_t asyn_gateway_cb;
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
		//m_tran_sleep();	//网关不睡眠
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
		mac_prim_arg_t *mac_prim_arg =&(sbuf->primargs.prim_arg.mac_prim_arg);
		pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
		osel_memcpy(&mac_frm_ctrl, pbuf->data_p, MAC_HEAD_CTRL_SIZE);
		pbuf->data_p += MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE;
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		sbuf->primtype = M2N_DATA_INDICATION;
		sbuf->primargs.pbuf = pbuf;
		
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.des_addr_mode, &mac_prim_arg->dst_addr);
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.src_addr_mode, &mac_prim_arg->src_addr);
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->msdu = pbuf->head;
		mac_prim_arg->msdu_length = pbuf->data_len;
		asyn_gateway_cb.recv_cb(sbuf);
		mac_sched_sleep_timeout_set();
	}
	else
	{
		pbuf_free(&pbuf __PLINE2);
		//m_tran_sleep();
		set_asyn_state(TRUE);
	}
}

static void mac_ack(pbuf_t *pbuf)
{
	pbuf_free(&pbuf __PLINE2);
}

static void mac_command(pbuf_t *pbuf)
{
	if(pbuf->attri.dst_id !=info.mac_pib->self_saddr
		   && pbuf->attri.dst_id!=0xffff)
	{
		pbuf_free(&pbuf __PLINE2);
        return;
	}
	uint8_t frm_type = 0;
	frm_type = *(uint8_t *)(pbuf->data_p);
	pbuf->data_p++;
	pbuf->data_p++;	//多偏移接收个数1个字节
	if(frm_type == MAC_CMD_WAKEUP)
	{
		mac_head_t *mac_frm_ctrl;
		mac_frm_ctrl =(mac_head_t *)(pbuf->head + PHY_HEAD_SIZE);
		bool_t pending = FALSE;	
		set_asyn_state(FALSE);
		if(mac_frm_ctrl->ctrl.frm_pending)	//需要接收数据
		{
//			uint16_t id = 0;
			assoc_info_t *temp = NULL;
			temp = asyn_assos_table_find(pbuf->attri.src_id);
			if(NULL != temp)
			{
				bcn_payload_t beacon_payload;
				osel_memcpy(&beacon_payload, pbuf->data_p, sizeof(bcn_payload_t));
				temp->new_time_stamp =  hal_timer_now().w + beacon_payload.time_stamp;
			}
			pending = TRUE;
		}
		uint32_t r_time = MS_TO_TICK(info.attribute->asyn_cfg_arg.asyn_cycle) - (hal_timer_now().w - info.attribute->recv_time);
		sbuf_t *sbuf = mac_auery_request_package(pbuf->attri.src_id,pending,r_time);
		m_tran_send(sbuf, auery_request_txok_cb, 1);
	}
	else if(frm_type == MAC_CMD_WAKEUP_RESP)
	{
		bcn_payload_t beacon_payload;
		osel_memcpy(&beacon_payload, pbuf->data_p, sizeof(bcn_payload_t));
		if(get_state() == WORK_ON)
		{
			uint16_t id = pbuf->attri.src_id;
			assoc_info_t *temp = asyn_assos_table_find(id);
			temp->new_time_stamp =  hal_timer_now().w + beacon_payload.time_stamp;
			osel_pthread_exit(info.mac_task,&asyn_process,&asyn_process);
			sbuf_t *sbuf = asyn_send_list_node_get();
			set_asyn_state(FALSE);
			m_tran_send(sbuf, data_tx_done, 1);
		}
	}
	else if(frm_type == MAC_CMD_ASSOC_REQ)
	{
		uint64_t src_addr = mac_head_info.addr.src_addr;
		mac_assoc_req_arg_t assoc_req;
		mac_assoc_state_e state = ASSOC_STATUS_RESERVED;
		uint8_t cluster_index = 0;
		assoc_request_frame(pbuf, &assoc_req);	//关联解析
		assoc_info_t id;
		osel_memset(&id,0,sizeof(assoc_info_t));
		id.device_type = assoc_req.device_type;
		id.mac_addr = src_addr;
		asyn_assos_table_add(id, &state);
		sbuf_t *sbuf = mac_assoc_respond_package(state,0 ,src_addr, cluster_index);//组关联应答
		set_asyn_state(FALSE);
		m_tran_send(sbuf, data_tx_done, 1);
	}
	pbuf_free(&pbuf __PLINE2);
}
static void mac_schedule_init(void)
{
	asyn_config(0x00);
	asyn_cfg(info.attribute, SLOT_LOCAL_TIME);
	hal_time_t now;
	now = hal_timer_now();
	now.w += MS_TO_TICK(1000);
	m_slot_run(&now);
}

static bool_t run(void)
{
	set_state(WORK_ON);
	info.mac_pib->hops = 0;
	mac_frames_asyn_cb_init();
	asyn_frame_switch_init(none,mac_data,mac_ack,mac_command);
	slot_cb_init();
	mac_schedule_init();
	return TRUE;
}
static bool_t stop(void)
{
	set_state(WORK_DOWN);
	m_slot_stop();
	return TRUE;
}
static bool_t gateway_send(sbuf_t *sbuf)
{
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	switch (sbuf->primtype)
	{
	case N2M_DATA_REQUEST:
		{
#if NODE_TYPE == NODE_TYPE_COLLECTOR_STD
			south_sbuf_fill(sbuf,info.mac_pib->self_saddr);
#else
			mac_data_fill_package(sbuf);
#endif
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

const struct asyn_gateway_t asyn_gateway =
{
	run,
	stop,
	gateway_send,
	get,
};
