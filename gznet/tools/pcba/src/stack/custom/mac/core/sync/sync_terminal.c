#include "sync_terminal.h"
#include "sync_general.h"
#include "sync_mac_package.h"
#include "../../general/neighbors.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include <driver.h>
#include <hal_board.h>

static sync_info_t info;
static hal_timer_t *recv_timer = NULL;
static uint8_t lost_beacon = 0;
static bool_t cap_listen = FALSE;

terminal_cb_t terminal_cb;


static mac_state_e get_state(void)
{
	return *info.state;
}

static void set_state(mac_state_e state)
{
    *info.state = state;
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
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}

static void intra_gts_slot_txok_cb(sbuf_t *sbuf, bool_t res)
{
	m_tran_sleep();
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
	pbuf_t *pbuf = sbuf->primargs.pbuf;
	pbuf->attri.already_send_times.mac_send_times++;
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + pbuf->attri.mac_length;
    pbuf->data_len -= pbuf->attri.mac_length;
    sbuf->primargs.prim_arg.mac_prim_arg.status = res;
	terminal_cb.send_cb(sbuf,res);
}

static void coord_timeout_cb(void *p)
{
	hal_led_close(HAL_LED_GREEN);
	recv_timer = NULL;
	if(!tran_rx_sfd_get())
	{
		if(++lost_beacon == SYNC_LOST_CNT)
		{
			terminal_cb.mac_restart(SYNC_S);
		}
		else
			m_tran_sleep();
	}
	else
		lost_beacon = 0;
}

static void recv_over_cb(void *p)
{
	hal_led_open(HAL_LED_GREEN);
	recv_timer = NULL;
	if(!tran_rx_sfd_get())
	{
//		m_tran_sleep();
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

static void beacon_slot_cb(void *seq_p)
{
	uint16_t slot = m_slot_get_seq();
	supf_spec_t *supf = &info.mac_pib->supf_cfg_arg;
	if(self_coord_beacon_recv_enable(slot,supf->intra_gts_number,
                                     info.mac_pib->coord_cluster_index))
	{//1.接收父节点的beacon
		hal_led_open(HAL_LED_GREEN);
		m_tran_recv();
		mac_sched_recv_timeout_set(TRUE);
	}
	else
	{
		m_tran_sleep();
	}
}

static void intra_gts_slot_cb(void *seq_p)
{
	if(get_state() != WORK_ON)
	{
		m_tran_sleep();	return;
	}
	uint16_t slot = m_slot_get_seq();
	supf_spec_t *supf = &info.mac_pib->supf_cfg_arg;
	uint8_t seq = *(uint8_t *)seq_p;
	if(coord_intra_gts_enable(slot,supf->intra_gts_number,
                              info.mac_pib->coord_cluster_index))
	{
		if((pending_me_seq_array & (1<<seq))!=0)	
		{
			m_tran_recv();
			mac_sched_recv_timeout_set(FALSE);
			return;
		}        
		if(pending_me_seq_array == 0)
		{
			sbuf_t *sbuf = gts_list_node_get(m_slot_get_seq(),UP_LINK);
			if (NULL != sbuf)
			{
				delay_us(DELAY_TIME);
				m_tran_send(sbuf, intra_gts_slot_txok_cb, 1);
			}
			else
			{
				m_tran_sleep();
			}
		}
		else
			m_tran_sleep();
	}
	else
    {
		m_tran_sleep();
    }
}

static void super_frame_slot_cb(void *seq_p)
{
	if(get_state() != WORK_ON)
	{
		m_tran_sleep();	return;
	}
	if(!gts_list_empty(UP_LINK))
	{
		uint16_t start,end;
		intra_gts_range(info.mac_pib,&start,&end);
		gts_list_sort(UP_LINK,start,end);
	}
}

static void intracom_slot_cb(void *seq_p)
{
    ;
}

static void inter_gts_slot_cb(void *seq_p)
{
    ;
}

static void intercom_sub_slot_cb(void *seq_p)
{
	m_tran_sleep();
}

static void intercom_slot_cb(void *seq_p)
{
    ;
}


static void sub_cap_slot_cb(void *seq_p)
{
    sbuf_t *sbuf = cap_list_node_get(m_slot_get_seq());
    if (NULL != sbuf)	//发送关联请求
    {
		cap_listen = TRUE;
		delay_us(DELAY_TIME);
		m_tran_send(sbuf, sub_cap_txok_cb, 1);
    }
    else
	{
		if(!cap_listen)	m_tran_sleep();
		else	
		{
			m_tran_recv();
			mac_sched_recv_timeout_set(FALSE);
		}
    }
}
static void capcom_slot_cb(void *seq_p)
{
	cap_listen = FALSE;
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
		//DBG_ASSERT(FALSE __DBG_LINE);
	}
	else
	{
		mac_beacon_frame(pbuf, &info);
	}
	pbuf_free(&pbuf __PLINE2);		
}

static void mac_data(pbuf_t *pbuf)
{
	if(pbuf->attri.dst_id == info.mac_pib->self_saddr
	   || pbuf->attri.dst_id == info.mac_pib->mac_addr)
	{
		DBG_ASSERT(pbuf != NULL __DBG_LINE);
		sbuf_t *sbuf = sbuf_alloc(__SLINE1);
		DBG_ASSERT(sbuf != NULL __DBG_LINE);
		
		mac_frm_ctrl_t mac_frm_ctrl;
		mac_prim_arg_t *mac_prim_arg = &(sbuf->primargs.prim_arg.mac_prim_arg);
		osel_memcpy(&mac_frm_ctrl, (pbuf->head + PHY_HEAD_SIZE), MAC_HEAD_CTRL_SIZE);
		
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		mac_prim_arg->msdu_length = pbuf->data_len;
		sbuf->primtype = M2N_DATA_INDICATION;
		sbuf->primargs.pbuf = pbuf;
		
		pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE;
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.des_addr_mode, &mac_prim_arg->dst_addr);
		mac_prim_arg->dst_mode = mac_frm_ctrl.des_addr_mode;
		get_addr(pbuf, (mac_addr_mode_e)mac_frm_ctrl.src_addr_mode, &mac_prim_arg->src_addr);
		mac_prim_arg->src_mode = mac_frm_ctrl.src_addr_mode;
		mac_prim_arg->msdu = pbuf->data_p;
		terminal_cb.recv_cb(sbuf);
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
	if(pbuf->attri.dst_id == info.mac_pib->self_saddr
	   || pbuf->attri.dst_id == info.mac_pib->mac_addr)
	{
		if(frm_type == MAC_CMD_ASSOC_RESP)
		{
			if(!assoc_response_frame(pbuf,info.mac_pib))
			{
				mac_neighbors_node_set_state(pbuf->attri.src_id, FALSE);
				set_state(WORK_DOWN);
			}
			else
			{//关联请求成功
				mac_neighbors_node_set_state(pbuf->attri.src_id, TRUE);
				set_state(WORK_ON);
				terminal_cb.mac_assoc_cb(TRUE);
			}
		}
	}
	pbuf_free(&pbuf __PLINE2);
}

static void slot_cb_init(void)
{
    info.mac_pib->time_slot[BEACON_SLOT] = beacon_slot_cb;
    info.mac_pib->time_slot[INTRA_GTS_SLOT] = intra_gts_slot_cb;
    info.mac_pib->time_slot[SUPER_FRAME_SLOT] = super_frame_slot_cb;
    info.mac_pib->time_slot[INTRACOM_SLOT] = intracom_slot_cb;
    info.mac_pib->time_slot[INTER_GTS_SLOT] = inter_gts_slot_cb;
    info.mac_pib->time_slot[INTERCOM_SUB_SLOT] = intercom_sub_slot_cb;
    info.mac_pib->time_slot[INTERCOM_SLOT] = intercom_slot_cb;
    info.mac_pib->time_slot[SUB_CAP_SLOT] = sub_cap_slot_cb;
    info.mac_pib->time_slot[CAPCOM_SLOT] = capcom_slot_cb;
    info.mac_pib->time_slot[SLEEP_SLOT] = sleep_slot_cb;
    info.mac_pib->time_slot[BEACON_INTERVAL] = beacon_interval_cb;
}

static bool_t run(void)
{
    set_state(READY_IDLE);
    mac_frames_cb_init();
    frame_switch_init(mac_beacon,mac_data,mac_ack,mac_command);
    slot_cb_init();
    m_tran_recv();
    
    return TRUE;
}

static bool_t stop(void)
{
    set_state(WORK_DOWN);
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
        mac_data_fill_package(sbuf);
        gts_list_insert(info.mac_pib->self_cluster_index,
                        &info.mac_pib->supf_cfg_arg,sbuf);
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

const struct terminal_t terminal =
{
    run,
    stop,
    terminal_send,
    get,
};
