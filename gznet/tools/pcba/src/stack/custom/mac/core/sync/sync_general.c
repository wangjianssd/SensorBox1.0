#include <mac_module.h>
#include <phy_packet.h>
#include "sync_general.h"
#include "../../general/neighbors.h"
#include "../../general/mac_package.h"
typedef void (*func_t)(void *args);
static uint8_t mac_offset = 0;
mac_head_t mac_head_info;
bool_t frm_pending = FALSE;
uint8_t pending_me_seq_array = 0;
static frame_switch_t frame_switch[MAC_FRAME_TYPE_RESERVED];
/** 顶层超帧配置 */
slot_cfg_t beacon_interval;
/** 簇内配置、簇间配置、CAP组配置、睡眠配置 */
slot_cfg_t intracom_slot, intercom_slot, capcom_slot, sleep_slot;
/** 簇内：SO超帧配置 */
static slot_cfg_t super_frame_slot;
/** 簇内：信标配置、簇内GTS交互单元配置 */
static slot_cfg_t beacon_slot, intra_gts_slot;
/** CAP组：子cap配置 */
static slot_cfg_t sub_cap_slot;
/** 簇间：簇间多跳组，几跳的设备对应于哪一组 */
static slot_cfg_t intercom_sub_slot[MAX_HOP_NUM];
static slot_cfg_t inter_gts_slot[MAX_HOP_NUM];       //father--intercom_sub_slot[i]

/**< 自定义函数 */

bool_t beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t local_map)
{
	uint8_t n = seq/(gts_num + 1);
	if((local_map & (1<<n)) != 0)	return TRUE;
	else return FALSE;
}

bool_t self_coord_beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t coord_cluster_index)
{
	uint8_t n = seq/(gts_num + 1);
	if(n == coord_cluster_index)	return TRUE;
	else return FALSE;
}

bool_t self_beacon_send_enable(uint16_t seq, uint8_t gts_num, uint16_t self_cluster_index)
{
	uint8_t n = seq/(gts_num + 1);
	if(n == self_cluster_index)	return TRUE;
	else return FALSE;
}

bool_t self_intra_gts_recv_enable(uint16_t seq , uint8_t gts_num, uint16_t self_cluster_index)
{
	uint8_t start = (self_cluster_index)*(gts_num+1);
	uint8_t	end = start + gts_num + 1;
	if(seq > start && seq < end)
		return TRUE;
	else
		return FALSE;
}

bool_t coord_intra_gts_enable(uint16_t seq , uint8_t gts_num, uint16_t coord_cluster_index)
{
	uint8_t start = (coord_cluster_index)*(gts_num+1);
	uint8_t	end = start + gts_num + 1;
	if(seq > start && seq < end)
		return TRUE;
	else
		return FALSE;
}

bool_t self_coord_inter_gts_enable(uint16_t seq , mac_pib_t *mac_pib)
{
	supf_spec_t *supf = &mac_pib->supf_cfg_arg;
	uint8_t beacon_cnt           	= 1;
    uint8_t intra_gts_number        = supf->intra_gts_number;
    uint8_t intra_cap_number        = supf->intra_cap_number;
    uint8_t cluster_number          = supf->cluster_number;
    uint8_t inter_unit_number       = supf->inter_unit_number;
	uint16_t inter_number = 0;
	uint8_t i=0;
    for(i = 0; i< mac_pib->hops-1; i++)	//父节点的簇间时隙个数
    {
		inter_number += supf->inter_gts_number[i];
    }
	uint8_t start = (beacon_cnt+intra_gts_number)*cluster_number + inter_number;
	uint8_t	end = start + supf->inter_gts_number[i] - 1;
	if(seq >= start && seq <= end)	return TRUE;
	else	return FALSE;
}

bool_t self_inter_gts_enable(uint16_t seq , mac_pib_t *mac_pib)
{
	supf_spec_t *supf = &mac_pib->supf_cfg_arg;
	uint8_t beacon_cnt           	= 1;
    uint8_t intra_gts_number        = supf->intra_gts_number;
    uint8_t intra_cap_number        = supf->intra_cap_number;
    uint8_t cluster_number          = supf->cluster_number;
    uint8_t inter_unit_number       = supf->inter_unit_number;
	uint16_t inter_number = 0;
	uint8_t i=0;
    for(i = 0; i< mac_pib->hops; i++)	//父节点的簇间时隙个数
    {
		inter_number += supf->inter_gts_number[i];
    }
	uint8_t start = (beacon_cnt+intra_gts_number)*cluster_number + inter_number;
	uint8_t	end = start + supf->inter_gts_number[i] - 1;
	if(seq >= start && seq <= end)	return TRUE;
	else	return FALSE;
}

void intra_gts_range(mac_pib_t *mac_pib, uint16_t *start,uint16_t *end)
{
	*start = *end = 0;
	uint8_t i = 0;
	supf_spec_t *supf = &mac_pib->supf_cfg_arg;
	uint8_t beacon_cnt           = 1;
    uint8_t intra_gts_number        = supf->intra_gts_number;
	*start = (beacon_cnt+intra_gts_number)*mac_pib->coord_cluster_index + 1;
	*end = *start + intra_gts_number -1;
}

void inter_gts_range(mac_pib_t *mac_pib, uint16_t *start,uint16_t *end)
{
	*start = *end = 0;
	uint8_t i = 0;
	supf_spec_t *supf = &mac_pib->supf_cfg_arg;
	uint8_t beacon_cnt           = 1;
    uint8_t intra_gts_number        = supf->intra_gts_number;
    uint8_t intra_cap_number        = supf->intra_cap_number;
    uint8_t cluster_number          = supf->cluster_number;
    uint8_t inter_unit_number       = supf->inter_unit_number;
	uint16_t inter_number = 0;
    for(i = 0; i<mac_pib->hops - 1; i++)
    {
		inter_number+=supf->inter_gts_number[i];
    }
    *start = (beacon_cnt+intra_gts_number)*cluster_number + inter_number;
	*end = *start + supf->inter_gts_number[i] -1;
}

/**< 超帧部分 */
void slot_node_cfg(slot_cfg_t *node,
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

void sync_config(uint16_t target_id)
{
    sync_cfg_t cfg;
    cfg.background_compute = FALSE;
    cfg.sync_source        = FALSE;
    cfg.sync_target        = target_id;
    cfg.flag_byte_pos      = 0x03;
    cfg.flag_byte_msk      = 0x07;
    cfg.flag_byte_val      = MAC_FRAME_TYPE_BEACON;
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

void super_frame_cfg(mac_pib_t *mac_pib, uint8_t type)
{
    supf_spec_t  *supf_cfg;
    supf_cfg = &mac_pib->supf_cfg_arg;
    uint32_t sleep_duration = 100;
    uint32_t beacon_duration = MS_TO_TICK(supf_cfg->beacon_duration_order);
    uint32_t beacon_interv = MS_TO_TICK(supf_cfg->beacon_interv_order);
    uint32_t gts_duration = MS_TO_TICK(supf_cfg->gts_duration);
	
    uint16_t intra_gts_number = supf_cfg->intra_gts_number;
    uint8_t cap_repeat_cnt = supf_cfg->intra_cap_number;
    uint8_t cluster_number = supf_cfg->cluster_number;
    uint8_t inter_unit_number = supf_cfg->inter_unit_number;
	
	
    uint8_t inter_gts_number[MAX_HOP_NUM];
    for(int i=0; i<MAX_HOP_NUM; i++)
    {
        inter_gts_number[i] = supf_cfg->inter_gts_number[i];
    }
	
    slot_node_cfg(&beacon_slot, beacon_duration, 1, mac_pib->time_slot[BEACON_SLOT],
                  &super_frame_slot, NULL, &intra_gts_slot);
    slot_node_cfg(&intra_gts_slot, gts_duration, intra_gts_number,
                  mac_pib->time_slot[INTRA_GTS_SLOT], &super_frame_slot, NULL, NULL);
    slot_node_cfg(&super_frame_slot, 0, cluster_number, mac_pib->time_slot[SUPER_FRAME_SLOT],
                  &intracom_slot, &beacon_slot, NULL);
    slot_node_cfg(&intracom_slot, 0, 1, mac_pib->time_slot[INTRACOM_SLOT],
                  &beacon_interval, &super_frame_slot, &intercom_slot);
    for (int i = 0; i < inter_unit_number; i++)
    {
        slot_node_cfg(&inter_gts_slot[i], gts_duration, inter_gts_number[i],
                      mac_pib->time_slot[INTER_GTS_SLOT], &intercom_sub_slot[i], NULL, NULL);
    }
    for (int i = 0; i < inter_unit_number; i++)
    {
        if (i == inter_unit_number - 1)
        {
            slot_node_cfg(&intercom_sub_slot[i], 0, 1, mac_pib->time_slot[INTERCOM_SUB_SLOT],
                          &intercom_slot, &inter_gts_slot[i], NULL);
        }
        else
        {
            slot_node_cfg(&intercom_sub_slot[i], 0, 1, mac_pib->time_slot[INTERCOM_SUB_SLOT],
                          &intercom_slot, &inter_gts_slot[i], &intercom_sub_slot[i + 1]);
        }
    }
    slot_node_cfg(&intercom_slot, 0, 1, mac_pib->time_slot[INTERCOM_SLOT],
                  &beacon_interval, &intercom_sub_slot[0], &capcom_slot);
    slot_node_cfg(&sub_cap_slot, gts_duration, cap_repeat_cnt,
                  mac_pib->time_slot[SUB_CAP_SLOT], &capcom_slot, NULL, NULL);
    slot_node_cfg(&capcom_slot, 0, 1, mac_pib->time_slot[CAPCOM_SLOT],
                  &beacon_interval, &sub_cap_slot, &sleep_slot);
    slot_node_cfg(&sleep_slot, sleep_duration, 1, mac_pib->time_slot[SLEEP_SLOT],
                  &beacon_interval, NULL, NULL);
    slot_node_cfg(&beacon_interval, 0, 0, mac_pib->time_slot[BEACON_INTERVAL],
                  NULL, &intracom_slot, NULL);
    m_slot_cfg(&beacon_interval, type);
    uint32_t beacon_interval_false = beacon_interval.slot_duration;
    if(beacon_interv > beacon_interval_false)
    {
        beacon_interval.slot_duration = beacon_interv;
        sleep_slot.slot_duration = beacon_interval.slot_duration - beacon_interval_false + sleep_duration;
    }
    else
    {
        slot_node_cfg(&sleep_slot, 0, 1, mac_pib->time_slot[SLEEP_SLOT], &beacon_interval, NULL, NULL);
        m_slot_cfg(&beacon_interval, type);              //recalc
        beacon_interval_false = beacon_interval.slot_duration;
    }
}
/**< 超帧部分end */

/**< 传输模块回调函数 */
static pbuf_t *mac_frame_get(void)
{
    pbuf_t *frame = NULL;
    frame = phy_get_packet();
    return frame;
}

static bool_t mac_frame_head_parse(pbuf_t *pbuf)
{
    mac_offset = 0;
    osel_memset(&mac_head_info, 0 , sizeof(mac_head_t));
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    osel_memcpy(&(mac_head_info.ctrl), pbuf->data_p, MAC_HEAD_CTRL_SIZE);
    frm_pending = mac_head_info.ctrl.frm_pending;
    //solt没有工作起来时只接收beacon
    if ( (!m_slot_get_state()) && (mac_head_info.ctrl.frm_type != MAC_FRAME_TYPE_BEACON))
    {
        pbuf_free(&pbuf __PLINE2);
        m_tran_recv();
        return FALSE;
    }
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    mac_offset += MAC_HEAD_CTRL_SIZE;
    mac_head_info.seq = (*pbuf->data_p);
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    mac_offset += MAC_HEAD_SEQ_SIZE;
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head_info.ctrl.des_addr_mode, &mac_head_info.addr.dst_addr);
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head_info.ctrl.src_addr_mode, &mac_head_info.addr.src_addr);
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
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + mac_offset;
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

static void mac_tx_finish(sbuf_t *const sbuf, bool_t result)
{
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

void mac_frames_cb_init(void)
{
    tran_cfg_t mac_tran_cb;
    mac_tran_cb.frm_get             = mac_frame_get;
    mac_tran_cb.frm_head_parse      = mac_frame_head_parse;
    mac_tran_cb.frm_payload_parse   = mac_frame_parse;
    mac_tran_cb.tx_finish           = mac_tx_finish;
    mac_tran_cb.send_ack            = mac_send_ack;
    m_tran_cfg(&mac_tran_cb);
}

void frame_switch_init(frame_switch_t mac_beacon,frame_switch_t mac_data,frame_switch_t mac_ack,
                       frame_switch_t mac_command)
{
    frame_switch[MAC_FRAME_TYPE_BEACON] = mac_beacon;
    frame_switch[MAC_FRAME_TYPE_DATA] = mac_data;
    frame_switch[MAC_FRAME_TYPE_ACK] = mac_ack;
    frame_switch[MAC_FRAME_TYPE_COMMAND] = mac_command;
}
/**< 传输模块回调函数end */

void mac_beacon_frame(pbuf_t *pbuf, sync_info_t *info)
{
	neighbor_node_t neighbor_node;
    //读取超帧配置
    osel_memcpy((uint8_t *)&neighbor_node.supf_cfg_arg, pbuf->data_p, sizeof(supf_spec_t));
    pbuf->data_p += sizeof(supf_spec_t);
	pending_me_seq_array = 0;
    //读取frm_pending
    if (frm_pending)
    {
        pend_addr_spec_t pend_addr;
        uint16_t short_addr = 0;
        uint64_t long_addr = 0;
        uint16_t gts_seq = 0;
        osel_memcpy((uint8_t *)&pend_addr, pbuf->data_p, sizeof(pend_addr_spec_t));
        pbuf->data_p += sizeof(pend_addr_spec_t);
        for (uint8_t i = 0; i < pend_addr.short_addr_num; i++)
        {
            osel_memcpy((uint8_t *)&short_addr, pbuf->data_p, sizeof(uint16_t));
            pbuf->data_p += MAC_ADDR_SHORT_SIZE;
            if (short_addr == mac_short_addr_get(info->mac_pib->mac_addr))
            {
				pending_me_seq_array = (1<< gts_seq);
            }
            gts_seq++;
        }
        for (uint8_t i = 0; i < pend_addr.long_addr_num; i++)
        {
            osel_memcpy((uint8_t *)&long_addr, pbuf->data_p, sizeof(uint64_t));
            pbuf->data_p += MAC_ADDR_LONG_SIZE;
            if (long_addr == info->mac_pib->mac_addr)
            {
				pending_me_seq_array = (1<< gts_seq);
            }
            gts_seq++;
        }
    }
    //读取超帧载荷
    bcn_payload_t *bcn_pld = (bcn_payload_t *)(pbuf->data_p);
    info->mac_pib->local_beacon_map |= ((uint16_t)0x01 << (bcn_pld->index));
#if DEBUG_COORD_HOP != 0
    if ((bcn_pld->hops) < DEBUG_COORD_HOP)
    {
		m_tran_recv();
        return;
    }
#endif
    neighbor_node.dev_id     = pbuf->attri.src_id;
    neighbor_node.rssi       = pbuf->attri.rssi_dbm;
    neighbor_node.hops       = bcn_pld->hops;
    neighbor_node.time_stamp = bcn_pld->time_stamp;
    neighbor_node.coord_index = bcn_pld->index;
    neighbor_node.intra_channel = neighbor_node.supf_cfg_arg.intra_channel;
	
    if (!mac_neighbor_node_add(&neighbor_node))
    {
        mac_neighbors_init();
        mac_neighbor_node_add(&neighbor_node);
    }
	if(pending_me_seq_array != 0)
	{
		m_tran_recv();
	}
	else
	{
		if (m_slot_get_state())
		{
			m_tran_sleep();
		}
		else
		{
			m_tran_recv();
		}
	}
}
