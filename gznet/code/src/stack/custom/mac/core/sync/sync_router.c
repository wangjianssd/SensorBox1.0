#include "sync_router.h"
#include "sync_general.h"
#include "sync_mac_package.h"
#include "../../general/neighbors.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include "../../general/assos_table.h"
#include "platform/platform.h"
#include "common/hal/hal.h"
static sync_info_t info;
static sbuf_t beacon;
static hal_timer_t *recv_timer = NULL;
static uint8_t lost_beacon = 0;

router_cb_t router_cb;
static mac_state_e get_state(void)
{
	return *info.state;
}
static void set_state(mac_state_e state)
{
	*info.state = state;
}

static void mac_beacon_deinit(void)
{
	pbuf_t *pbuf = beacon.primargs.pbuf;
	if(pbuf != NULL)
	{
		pbuf_free(&pbuf __PLINE2);
	}
}

static void mac_beacon_init(void)
{
    pbuf_t *beacon_packet = pbuf_alloc(MEDIUM_PBUF_BUFFER_SIZE __PLINE1);
	DBG_ASSERT(beacon_packet != NULL __DBG_LINE);
	beacon.primargs.pbuf = beacon_packet;
}

static void beacon_tx_done(sbuf_t *packet, bool_t result)
{
	hal_led_close(HAL_LED_RED);
	pbuf_t *pbuf = packet->primargs.pbuf;
    pbuf->data_len = 0;
	m_tran_sleep();
}

static void sub_cap_txok_cb(sbuf_t *sbuf, bool_t res)
{
	m_tran_sleep();
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

static void intra_gts_slot_txok_cb(sbuf_t *sbuf, bool_t res)
{
	m_tran_sleep();
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
	pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
	sbuf_free(&sbuf __SLINE2);
}

static void inter_gts_slot_txok_cb(sbuf_t *sbuf, bool_t res)
{
	m_tran_sleep();
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
	pbuf_t *pbuf = sbuf->primargs.pbuf;
	pbuf->attri.already_send_times.mac_send_times++;
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + pbuf->attri.mac_length;
    pbuf->data_len -= pbuf->attri.mac_length;
    sbuf->primargs.prim_arg.mac_prim_arg.status = res;
	router_cb.send_cb(sbuf,res);
}

static void coord_timeout_cb(void *p)
{
	recv_timer = NULL;
	if(!tran_rx_sfd_get())
	{
		if(++lost_beacon == 4)
		{
			hal_led_close(HAL_LED_RED);
			router_cb.mac_restart(SYNC_S);
		}
		else
			m_tran_sleep();
	}
	else
		lost_beacon = 0;
}

static void recv_over_cb(void *p)
{
	recv_timer = NULL;
	if(!tran_rx_sfd_get())
	{
		m_tran_sleep();
	}
}

static void mac_sched_recv_timeout_set(bool_t state)
{
	uint16_t time = 4000;
	if(recv_timer != NULL)
    {
		DBG_ASSERT(FALSE __DBG_LINE);
		hal_timer_cancel(&recv_timer);
    }
	if(state)	HAL_TIMER_SET_REL(US_TO_TICK(time), coord_timeout_cb, NULL, recv_timer);
	else	HAL_TIMER_SET_REL(US_TO_TICK(time), recv_over_cb, NULL, recv_timer);
    
	DBG_ASSERT(recv_timer != NULL __DBG_LINE);
}

static void beacon_slot_cb(void *seq_p)	//beacon时隙
{
	uint16_t slot = m_slot_get_seq();
	if(self_coord_beacon_recv_enable(slot,
                                     info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                     info.sync_attribute->coord_cluster_index))
	{//1.接收父节点的beacon
		m_tran_recv();
		mac_sched_recv_timeout_set(TRUE);
	}
	else
	{
		if(self_beacon_send_enable(slot,
                                   info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                   info.sync_attribute->self_cluster_index)
		   && (get_state() == WORK_ON))
		{//2.在自己的beacon时隙发送
			hal_led_open(HAL_LED_RED);
			mac_beacon_package(&beacon,info.sync_attribute, info.mac_pib);
			delay_us(DELAY_TIME);
			m_tran_send(&beacon, beacon_tx_done, 1);
		}
		else if(beacon_recv_enable(slot,
                                   info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                   info.sync_attribute->local_beacon_map))
		{//侦听邻居beacon
			m_tran_recv();
			mac_sched_recv_timeout_set(FALSE);
		}
		else
		{
			m_tran_sleep();
		}
	}
}

static void intra_gts_slot_cb(void *seq_p)	//簇内时隙
{
	uint16_t slot = m_slot_get_seq();
	uint8_t seq = *(uint8_t *)seq_p;
	if(coord_intra_gts_enable(slot,
                              info.sync_attribute->supf_cfg_arg.intra_gts_number,
                              info.sync_attribute->coord_cluster_index))
	{//在上级设备的簇内时隙
		if((pending_me_seq_array & (1<<seq))!=0)
		{
			m_tran_recv();
			mac_sched_recv_timeout_set(FALSE);
			//pending_me_seq_array &= ~(1<<seq);
		}
		else	m_tran_sleep();
		return;
	}
	
	if(self_intra_gts_recv_enable(slot,
                                  info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                  info.sync_attribute->self_cluster_index))//在自己的簇内时隙接收
	{
		if(!gts_list_empty(DOWN_LINK))
		{/**< 有下行数据需要发送 */
			uint8_t seq  = *(uint8_t *)seq_p;
			sbuf_t *sbuf = gts_list_node_get(seq,DOWN_LINK);
			if (NULL != sbuf)
			{
				delay_us(DELAY_TIME);
				m_tran_send(sbuf, intra_gts_slot_txok_cb, 1);
				return;
			}
			else	
				m_tran_sleep();
		}
		else	
			m_tran_recv();mac_sched_recv_timeout_set(FALSE);
	}
	else
	{
		m_tran_sleep();
	}
}
static void super_frame_slot_cb(void *seq_p)
{
	;
}
static void intracom_slot_cb(void *seq_p)
{
	if(!gts_list_empty(UP_LINK))
	{
		uint16_t start,end;
		inter_gts_range(info.sync_attribute,&start,&end, info.mac_pib->hops);
		gts_list_sort(UP_LINK,start,end);
	}
}

static void inter_gts_slot_cb(void *seq_p)	//簇间时隙
{
	uint16_t slot = m_slot_get_seq();
	if(self_coord_inter_gts_enable(slot, info.sync_attribute, info.mac_pib->hops))		//1.是上级的簇间时隙发送数据
	{
		sbuf_t *sbuf = gts_list_node_get(slot,UP_LINK);
		if (NULL != sbuf)
		{
			delay_us(DELAY_TIME);
			m_tran_send(sbuf, inter_gts_slot_txok_cb, 1);
		}
		else
		{
			m_tran_sleep();
		}
	}
	else if(self_inter_gts_enable(slot,info.sync_attribute,info.mac_pib->hops))	//2.是自己的簇间时隙接收数据
	{
		m_tran_recv();
		mac_sched_recv_timeout_set(FALSE);
	}
	else
	{
		m_tran_sleep();
	}
}

static void intercom_sub_slot_cb(void *seq_p)
{
}
static void intercom_slot_cb(void *seq_p)
{
}

static void sub_cap_slot_cb(void *seq_p)
{
	sbuf_t *sbuf = cap_list_node_get(m_slot_get_seq());
    if (NULL != sbuf)	//发送关联请求
    {
		delay_us(DELAY_TIME);
		m_tran_send(sbuf, sub_cap_txok_cb, 1);
    }
    else
	{
		m_tran_recv();
		mac_sched_recv_timeout_set(FALSE);
    }
}
static void capcom_slot_cb(void *seq_p)
{
	
}
static void sleep_slot_cb(void *seq_p)
{
	m_tran_sleep();
}
static void beacon_interval_cb(void *seq_p)
{
}

static void mac_beacon(pbuf_t *pbuf)
{
	uint16_t addr = pbuf->attri.src_id;
	if(addr != info.mac_pib->coord_saddr && get_state() == WORK_ON)
	{
		uint8_t cluster_index = m_slot_get_seq()/info.sync_attribute->supf_cfg_arg.cluster_number+1;
		cluster_node_update(cluster_index,addr);
	}
	else
	{
		mac_beacon_frame(pbuf, &info);
	}
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
		router_cb.recv_cb(sbuf);
	}
	else
	{
		pbuf_free(&pbuf __PLINE2);
	}
}

static void mac_ack(pbuf_t *pbuf)
{
	pbuf_free(&pbuf __PLINE2);
}

static void mac_command(pbuf_t *pbuf)
{
	uint8_t frm_type = 0;
	frm_type = *pbuf->data_p ;
    pbuf->data_p++;
	if(pbuf->attri.dst_id == info.mac_pib->self_saddr)
	{
		if(frm_type == MAC_CMD_ASSOC_RESP)
		{
			if(!assoc_response_frame(pbuf,info.mac_pib,info.sync_attribute))
			{
				mac_neighbors_node_set_state(pbuf->attri.src_id, FALSE);
				set_state(WORK_DOWN);
			}
			else
			{//关联请求成功
				mac_neighbors_node_set_state(pbuf->attri.src_id, TRUE);
				set_state(ASSOC_RESPONSE);
				router_cb.mac_assoc_cb(TRUE);
			}
		}
		else if(frm_type == MAC_CMD_ASSOC_REQ)
		{
			uint64_t src_addr = mac_head_info.addr.src_addr;
			mac_assoc_req_arg_t assoc_req;
			mac_assoc_state_e state = ASSOC_STATUS_RESERVED;
			uint8_t cluster_index = 0;
			uint16_t slot_seq = m_slot_get_seq();
			assoc_request_frame(pbuf, &assoc_req);	//关联解析
            uint16_t local_beacon_map = info.sync_attribute->local_beacon_map;
			assos_table_add(&assoc_req, &state, &cluster_index, 
							&local_beacon_map, src_addr, 
                            info.sync_attribute->supf_cfg_arg.cluster_number);//加入关联表	
			sbuf_t *sbuf = mac_assoc_respond_package(state,--slot_seq ,src_addr, cluster_index);//组关联应答
            supf_spec_t supf_cfg_arg = info.sync_attribute->supf_cfg_arg;
			cap_list_insert(&supf_cfg_arg,sbuf);
		}
	}
	pbuf_free(&pbuf __PLINE2);
}

static void slot_cb_init(void)
{
	info.sync_attribute->time_slot[BEACON_SLOT] = beacon_slot_cb;
	info.sync_attribute->time_slot[INTRA_GTS_SLOT] = intra_gts_slot_cb;
	info.sync_attribute->time_slot[SUPER_FRAME_SLOT] = super_frame_slot_cb;
	info.sync_attribute->time_slot[INTRACOM_SLOT] = intracom_slot_cb;
	info.sync_attribute->time_slot[INTER_GTS_SLOT] = inter_gts_slot_cb;
	info.sync_attribute->time_slot[INTERCOM_SUB_SLOT] = intercom_sub_slot_cb;
	info.sync_attribute->time_slot[INTERCOM_SLOT] = intercom_slot_cb;
    info.sync_attribute->time_slot[SUB_CAP_SLOT] = sub_cap_slot_cb;
	info.sync_attribute->time_slot[CAPCOM_SLOT] = capcom_slot_cb;
	info.sync_attribute->time_slot[SLEEP_SLOT] = sleep_slot_cb;
	info.sync_attribute->time_slot[BEACON_INTERVAL] = beacon_interval_cb;
}

static bool_t run(void)
{
    set_state(READY_IDLE);
	mac_beacon_init();
	mac_frames_sync_cb_init();
	frame_switch_init(mac_beacon,mac_data,mac_ack,mac_command);
	slot_cb_init();
	m_tran_recv();
	return TRUE;
}

static bool_t stop(void)
{
	mac_beacon_deinit();
	set_state(WORK_DOWN);
	if (m_slot_get_state())
	{
		m_slot_stop();
    }
	m_tran_sleep();
    return TRUE;
}

static bool_t router_send(sbuf_t *sbuf)
{
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
    supf_spec_t supf_cfg_arg = info.sync_attribute->supf_cfg_arg;
	switch (sbuf->primtype)
	{
	case N2M_DATA_REQUEST:
		mac_data_fill_package(sbuf);
		gts_list_insert(info.sync_attribute->self_cluster_index,&supf_cfg_arg,sbuf);
		break;
	default:
		return FALSE;
    }
    return TRUE;
}

static sync_info_t* get(void)
{
	return &info;
}

const struct router_t router =
{
	run,
	stop,
    router_send,
	get,
};
