#include "asyn_general.h"
#include "../../general/mac_package.h"
#include "stack/module/mac_module/mac_module.h"

#include "stack/custom/phy/phy_packet.h"
#include "common/hal/hal.h"

typedef void (*func_t)(void *args);
static frame_switch_t frame_switch[MAC_FRAME_TYPE_RESERVED];
static list_head_t send_list;

static slot_cfg_t asyn_interval;
static slot_cfg_t recv_slot;
static slot_cfg_t sleep_slot;

osel_etimer_t asyn_timer_ev;

static void slot_node_cfg(slot_cfg_t *node,
                          uint32_t duration,
                          uint8_t repeat_cnt,
                          func_t func,
                          slot_cfg_t *parent,
                          slot_cfg_t *first_child,
                          slot_cfg_t *next_sibling)
{
    node->slot_duration = duration;
    node->slot_repeat_cnt = repeat_cnt;
    node->func = func;
    node->parent = parent;
    node->first_child = first_child;
    node->next_sibling = next_sibling;
    node->slot_start = 0;
    node->slot_repeat_seq = 0;
}

void asyn_config(uint16_t target_id)
{
    sync_cfg_t cfg;
    cfg.background_compute = FALSE;
    cfg.sync_source        = FALSE;
    cfg.sync_target        = target_id;
    cfg.flag_byte_pos      = 0x03;
    cfg.flag_byte_msk      = 0x07;
    cfg.flag_byte_val      = 0;
    cfg.len_pos            = 0;
    cfg.len_modfiy         = TRUE;
    cfg.stamp_len          = 4;
    cfg.stamp_byte_pos     = 0;
    cfg.tx_sfd_cap         = FALSE;
    cfg.rx_sfd_cap         = TRUE;
    cfg.tx_offset          = 67;
    cfg.rx_offset          = 0;
    m_sync_cfg(&cfg);
}

void asyn_cfg(asyn_attribute_t *asyn_attribute, uint8_t type)
{
    uint32_t recv_duration = MS_TO_TICK(asyn_attribute->asyn_cfg_arg.duration);
    uint32_t sleep_duration = MS_TO_TICK(asyn_attribute->asyn_cfg_arg.asyn_cycle-asyn_attribute->asyn_cfg_arg.duration);
    slot_node_cfg(&recv_slot, recv_duration, 1, asyn_attribute->time_slot[ASYN_RECV],
                  &asyn_interval, NULL, &sleep_slot);
    slot_node_cfg(&sleep_slot, sleep_duration, 1, asyn_attribute->time_slot[ASYN_SLEEP],
                  &asyn_interval, NULL, NULL);
    slot_node_cfg(&asyn_interval, 0, 0, asyn_attribute->time_slot[ASYN_IDLE],
                  NULL, &recv_slot, NULL);
    m_slot_cfg(&asyn_interval, type);
}

static pbuf_t *mac_frame_get(void)
{
    pbuf_t *frame = NULL;
    frame = phy_get_packet();
    return frame;
}

static bool_t mac_frame_head_parse(pbuf_t *pbuf)
{
    uint8_t mac_offset = 0;
    osel_memset(&mac_head_info, 0 , sizeof(mac_head_t));
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    osel_memcpy(&(mac_head_info.ctrl), pbuf->data_p, MAC_HEAD_CTRL_SIZE);
	pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    mac_offset += MAC_HEAD_CTRL_SIZE;
    mac_head_info.seq = (*pbuf->data_p);
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    mac_offset += MAC_HEAD_SEQ_SIZE;
    uint64_t dst_addr = 0;
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head_info.ctrl.des_addr_mode, &dst_addr);
    mac_head_info.addr.dst_addr = dst_addr;
    uint64_t src_addr = 0;
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head_info.ctrl.src_addr_mode, &src_addr);
    mac_head_info.addr.src_addr = src_addr;
    pbuf->attri.src_id      = mac_head_info.addr.src_addr;
	pbuf->attri.dst_id 		= mac_head_info.addr.dst_addr;
    pbuf->attri.need_ack    = mac_head_info.ctrl.ack_req;
    pbuf->attri.seq         = mac_head_info.seq;
    pbuf->attri.is_ack      = (mac_head_info.ctrl.frm_type == MAC_FRAME_TYPE_ACK) ? (TRUE) : (FALSE);
    pbuf->data_p = pbuf->head + pbuf->data_len - PHY_FCS_SIZE;
	pbuf->attri.mac_length = mac_offset;
	return TRUE;
}

static bool_t mac_frame_parse(pbuf_t *pbuf)
{
	DBG_ASSERT(pbuf != NULL __DBG_LINE);
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + pbuf->attri.mac_length;
    mac_head_t *mac_frm_ctrl;
    mac_frm_ctrl =(mac_head_t *)(pbuf->head + PHY_HEAD_SIZE);
	
    if (mac_frm_ctrl->ctrl.frm_type < MAC_FRAME_TYPE_RESERVED)
    {
        frame_switch[mac_frm_ctrl->ctrl.frm_type](pbuf);
    }
    else
    {
        m_tran_recv();
    }
	return TRUE;
}

static void ack_tx_ok_callback(sbuf_t *sbuf, bool_t res)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
    sbuf_free(&sbuf __SLINE2);
	m_tran_sleep();
}

static void mac_send_ack(uint8_t seqno)
{
	pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(NULL != pbuf __DBG_LINE);
    mac_ack_fill_package(pbuf, seqno, mac_head_info.addr.src_addr);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    if (sbuf != NULL)
    {
        sbuf->primargs.pbuf = pbuf;
		
        if (m_tran_can_send())
        {
            m_tran_send(sbuf, ack_tx_ok_callback, 1);
        }
        else
        {
            pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
            sbuf_free(&sbuf __SLINE2);
        }
    }
    else
    {
        pbuf_free(&pbuf __PLINE2);
    }
}

static void mac_tx_finish(sbuf_t *const sbuf, bool_t result)
{
	
}

void mac_frames_asyn_cb_init(void)
{
	tran_cfg_t mac_tran_cb;
    mac_tran_cb.frm_get             = mac_frame_get;
    mac_tran_cb.frm_head_parse      = mac_frame_head_parse;
    mac_tran_cb.frm_payload_parse   = mac_frame_parse;
    mac_tran_cb.tx_finish           = mac_tx_finish;
    mac_tran_cb.send_ack            = mac_send_ack;
    m_tran_cfg(&mac_tran_cb);
}

void asyn_frame_switch_init(frame_switch_t mac_beacon,frame_switch_t mac_data,frame_switch_t mac_ack,
							frame_switch_t mac_command)
{
	frame_switch[MAC_FRAME_TYPE_BEACON] = mac_beacon;
    frame_switch[MAC_FRAME_TYPE_DATA] = mac_data;
    frame_switch[MAC_FRAME_TYPE_ACK] = mac_ack;
    frame_switch[MAC_FRAME_TYPE_COMMAND] = mac_command;
}

bool_t asyn_send_list_empty(uint16_t *id)
{
	sbuf_t *sbuf = NULL;
	if(list_empty(&send_list))
	{
		return TRUE;
	}
	else
	{
		uint8_t s = 0;
		OSEL_ENTER_CRITICAL(s);
		sbuf = list_entry_addr_find(list_first_elem_look(&send_list), sbuf_t, list);
		OSEL_EXIT_CRITICAL(s);
		*id = sbuf->primargs.pbuf->attri.dst_id;
		return FALSE;
	}
}

sbuf_t *asyn_send_list_node_get(void)
{
	sbuf_t *sbuf = NULL;
    if (list_empty(&send_list))
    {
        return NULL;
    }
	uint8_t s = 0;
	OSEL_ENTER_CRITICAL(s);
	sbuf = list_entry_decap(&send_list, sbuf_t, list);
	OSEL_EXIT_CRITICAL(s);
    return sbuf;
}

void asyn_send_list_insert(sbuf_t *sbuf)
{
	uint8_t s = 0;
	OSEL_ENTER_CRITICAL(s);
    list_add_to_tail(&(sbuf->list), &send_list);
	OSEL_EXIT_CRITICAL(s);
}

static void send_list_node_clear(void)
{
    if (!list_empty(&send_list))
    {
        sbuf_t *pos1 = NULL;
        sbuf_t *pos2 = NULL;
        list_entry_for_each_safe( pos1, pos2 , &send_list, sbuf_t, list)
        {
            uint8_t s = 0;
            sbuf_t *sbuf = NULL;
            OSEL_ENTER_CRITICAL(s);
            sbuf = pos1;
            list_del(&(pos1->list));
            OSEL_EXIT_CRITICAL(s);
            pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
            sbuf_free(&sbuf __SLINE2);
        }
    }
}

void asyn_general_init(void)
{
    if (!list_empty(&send_list))
    {
        send_list_node_clear();
    }
    list_init(&send_list);
}