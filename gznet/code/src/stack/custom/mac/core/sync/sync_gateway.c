#include "sync_gateway.h"
#include "sync_general.h"
#include "sync_mac_package.h"
#include "sync_general.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include "../../general/assos_table.h"
#include "platform/platform.h"
#include "common/hal/hal.h"

DBG_THIS_MODULE("sync_gateway")

static sync_info_t info;
static sbuf_t beacon;
gateway_cb_t gateway_cb;
static hal_timer_t *recv_timer = NULL;

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
    if (pbuf != NULL && !pbuf->used)
        pbuf_free(&pbuf __PLINE2);
}

static void mac_beacon_init(void)
{
    pbuf_t *beacon_packet = pbuf_alloc(MEDIUM_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(beacon_packet != NULL __DBG_LINE);
    beacon.primargs.pbuf = beacon_packet;
}

static void beacon_tx_done(sbuf_t *packet, bool_t result)
{
    hal_led_close(HAL_LED_GREEN);
    pbuf_t *pbuf = packet->primargs.pbuf;
    pbuf->data_len = 0;
}
static void intra_gts_slot_txok_cb(sbuf_t *sbuf, bool_t res)
{
    m_tran_sleep();
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
    pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
    sbuf_free(&sbuf __SLINE2);
}
static void sub_cap_txok_cb(sbuf_t *sbuf, bool_t result)
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

static void recv_over_cb(void *p)
{
    recv_timer = NULL;
    if (!tran_rx_sfd_get())
    {
        m_tran_sleep();
    }
}

static void mac_sched_recv_timeout_set(bool_t state)
{
    uint16_t time = 3000;
    if (recv_timer != NULL)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
        hal_timer_cancel(&recv_timer);
    }
    if (!state) HAL_TIMER_SET_REL(US_TO_TICK(time), recv_over_cb, NULL, recv_timer);
    DBG_ASSERT(recv_timer != NULL __DBG_LINE);
}

static void beacon_slot_cb(void *seq_p)
{
    uint8_t slot = m_slot_get_seq();
//  supf_spec_t *supf = &info.sync_attribute->supf_cfg_arg;
    if (self_beacon_send_enable(slot,
                                info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                info.sync_attribute->self_cluster_index))
    {
        hal_led_open(HAL_LED_GREEN);
        set_state(WORK_ON);
        mac_beacon_package(&beacon, info.sync_attribute , info.mac_pib);
        delay_us(DELAY_TIME);
        m_tran_send(&beacon, beacon_tx_done, 1);
    }
    else
    {
        if (beacon_recv_enable(slot,
                               info.sync_attribute->supf_cfg_arg.intra_gts_number,
                               info.sync_attribute->local_beacon_map))
        {
            m_tran_recv();
            mac_sched_recv_timeout_set(FALSE);
        }
        else
        {
            m_tran_sleep();
        }
    }
}
static void intra_gts_slot_cb(void *seq_p)
{
    if (get_state() != WORK_ON)
    {
        m_tran_sleep(); return;
    }
    uint16_t slot = m_slot_get_seq();
//  supf_spec_t *supf = &info.sync_attribute->supf_cfg_arg;

    if (!gts_list_empty(DOWN_LINK))
    {
        /**< 有下行数据需要发送 */
        uint8_t seq  = *(uint8_t *)seq_p;
        sbuf_t *sbuf = gts_list_node_get(seq, DOWN_LINK);
        if (NULL != sbuf)
        {
            delay_us(DELAY_TIME);
            m_tran_send(sbuf, intra_gts_slot_txok_cb, 1);
            return;
        }
        else
        {
            m_tran_recv();
            mac_sched_recv_timeout_set(FALSE);
        }
    }
    else
    {
        if (self_intra_gts_recv_enable(slot,
                                       info.sync_attribute->supf_cfg_arg.intra_gts_number,
                                       info.sync_attribute->self_cluster_index))//在自己的簇内时隙接收
        {
            m_tran_recv();
            mac_sched_recv_timeout_set(FALSE);
        }
        else
        {
            m_tran_sleep();
        }
    }
}
static void super_frame_slot_cb(void *seq_p)
{
    m_tran_sleep();
}
static void intracom_slot_cb(void *seq_p)
{
    m_tran_sleep();
}
static void inter_gts_slot_cb(void *seq_p)
{
    if (get_state() != WORK_ON)
    {
        m_tran_sleep(); return;
    }
    uint16_t slot = m_slot_get_seq();
//  supf_spec_t *supf = &info.sync_attribute->supf_cfg_arg;
    if (self_inter_gts_enable(slot, info.sync_attribute, info.mac_pib->hops))       //在自己的簇间时隙接收数据
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
    if (NULL != sbuf)
    {
        delay_us(DELAY_TIME);
        m_tran_send(sbuf, sub_cap_txok_cb, 1);
        return;
    }
    else
    {
        m_tran_recv();
        mac_sched_recv_timeout_set(FALSE);
    }
}
static void capcom_slot_cb(void *seq_p)
{
    cap_list_node_clear();  //清除队列中的数据
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
    if (get_state() == WORK_ON)
    {
        uint8_t cluster_index = (m_slot_get_seq() - 1) / (info.sync_attribute->supf_cfg_arg.intra_gts_number + 1);
        uint16_t addr = pbuf->attri.src_id;
        cluster_node_update(cluster_index, addr);
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
    pbuf_free(&pbuf __PLINE2);
}

static void mac_data(pbuf_t *pbuf)
{
    if (pbuf->attri.dst_id != info.mac_pib->self_saddr)
    {
        pbuf_free(&pbuf __PLINE2);
        return;
    }
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);

    mac_frm_ctrl_t mac_frm_ctrl;
    mac_prim_arg_t *mac_prim_arg = &(sbuf->primargs.prim_arg.mac_prim_arg);
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
    gateway_cb.recv_cb(sbuf);
}

static void mac_ack(pbuf_t *pbuf)
{
    pbuf_free(&pbuf __PLINE2);
}

static void mac_command(pbuf_t *pbuf)
{
    if (pbuf->attri.dst_id != info.mac_pib->self_saddr)
    {
        pbuf_free(&pbuf __PLINE2);
        return;
    }

    uint8_t frm_type = 0;
    frm_type = *(uint8_t *)(pbuf->data_p);
    pbuf->data_p++;
    if (frm_type == MAC_CMD_ASSOC_REQ)
    {
        uint64_t src_addr = mac_head_info.addr.src_addr;
        mac_assoc_req_arg_t assoc_req;
        mac_assoc_state_e state = ASSOC_STATUS_RESERVED;
        uint8_t cluster_index = 0;
        uint16_t slot_seq = m_slot_get_seq();
        assoc_request_frame(pbuf, &assoc_req);  //关联解析
        uint16_t local_beacon_map = info.sync_attribute->local_beacon_map;
        assos_table_add(&assoc_req, &state, &cluster_index,
                        &local_beacon_map,
                        src_addr,
                        info.sync_attribute->supf_cfg_arg.cluster_number);//加入关联表
        sbuf_t *sbuf = mac_assoc_respond_package(state, --slot_seq , src_addr, cluster_index); //组关联应答
        supf_spec_t supf_cfg_arg = info.sync_attribute->supf_cfg_arg;
        cap_list_insert(&supf_cfg_arg, sbuf);
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

static void mac_schedule_init(void)
{
    info.sync_attribute->self_cluster_index = 0;
    info.sync_attribute->local_beacon_map = 1;          //先把自己的簇时隙算上
    sync_config(0x00);
    super_frame_cfg(info.sync_attribute, SLOT_LOCAL_TIME);
    hal_time_t now;
    now = hal_timer_now();
    now.w += MS_TO_TICK(1000);
    m_slot_run(&now);
}

static bool_t run(void)
{
    set_state(SLOT_READY);
    mac_beacon_init();
    mac_frames_sync_cb_init();
    frame_switch_init(mac_beacon, mac_data, mac_ack, mac_command);
    slot_cb_init();
    mac_schedule_init();
    return TRUE;
}
static bool_t stop(void)
{
    set_state(WORK_DOWN);
    m_slot_stop();
    mac_beacon_deinit();
    return TRUE;
}
static bool_t gateway_send(sbuf_t *sbuf)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    switch (sbuf->primtype)
    {
    case N2M_DATA_REQUEST:
    {
#ifdef NODE_TYPE_COLLECTOR_STD
        south_sbuf_fill(sbuf, info.mac_pib->self_saddr);
#else
        if(sbuf->orig_layer != NWK_LAYER)
        {
            pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
            sbuf_free(&sbuf __SLINE2);
            return FALSE;
        }
		mac_data_fill_package(sbuf);
#endif
        supf_spec_t supf_cfg_arg = info.sync_attribute->supf_cfg_arg;
        gts_list_insert(info.sync_attribute->self_cluster_index,
                        &supf_cfg_arg, sbuf);
    }
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

const struct gateway_t gateway =
{
    run,
    stop,
    gateway_send,
    get,
};
