#include "pbuf.h"
#include "sbuf.h"
#include "lib.h"
#include "hal.h"

#include "osel_arch.h"

#include "mac_frame.h"
#include "mac.h"

#include "nwk_route.h"
#include "nwk_frames.h"
#include "nwk_interface.h"
#include "nwk_handles.h"

DBG_THIS_MODULE("terminal nwk")

typedef struct
{
    nwk_dependent_t dependent;

    uint16_t nwk_addr;
    uint64_t node_nui;

    osel_etimer_t cycle_etimer;
    osel_etimer_t join_etimer;
    uint8_t join_cnt;
    uint8_t join_status;
    uint8_t hop_num;
    uint8_t nwk_seq;

    bool_t first_startup;

    uint16_t father_id;

    uint8_t live_cnt;
} nwk_t;

static nwk_t nwk;
static uint16_t sink_nwk_address = 0x0001;

static list_head_t query_head;

uint32_t send_frames_cnt = 0;
uint32_t send_success_cnt = 0;


#define NWK_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t nwk_event_store[NWK_EVENT_MAX];
static osel_task_t *nwk_task_tcb = NULL;

PROCESS_NAME(nwk_cycle_process);
PROCESS_NAME(nwk_join_process);
PROCESS_NAME(app2nwk_process);

static void nwk_send_data_packet(sbuf_t *sbuf,
                                 mac_node_addr_t next_hop,
                                 uint8_t len);

uint8_t nwk_prim_success_rate_get(void)
{
    fp32_t rate = ((fp32_t)send_success_cnt) / send_frames_cnt;
    uint8_t tran_succ_rate = (send_frames_cnt > 0) ? (uint8_t)(rate * 100) : (100);
    tran_succ_rate = tran_succ_rate > 100 ? 100 : tran_succ_rate;
    send_success_cnt = 0; send_frames_cnt = 0;
    return tran_succ_rate;
}

void nwk_dependent_cfg(const nwk_dependent_t *cfg)
{
    nwk.dependent.nwk_data_confirm  = cfg->nwk_data_confirm;
    nwk.dependent.nwk_data_indicate = cfg->nwk_data_indicate;
    nwk.dependent.nwk_join_indicate = cfg->nwk_join_indicate;
}

static void mac_data_recv(sbuf_t *const sbuf)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    sbuf->primtype = M2N_DATA_INDICATION;

    osel_event_t event;
    event.sig = MAC2NWK_PRIM_EVENT;
    event.param = sbuf;
    osel_post(NULL, &mac2nwk_process, &event);
}

static void mac_data_sent(sbuf_t *const sbuf, bool_t res)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    sbuf->primtype = M2N_DATA_CONFIRM;

    osel_event_t event;
    event.sig = MAC2NWK_PRIM_EVENT;
    event.param = sbuf;
    osel_post(NULL, &mac2nwk_process, &event);
}

static void mac_assoc_confirm(bool_t res)
{
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    sbuf->primtype = M2N_ASSOC_CONFIRM;
    sbuf->primargs.prim_arg.mac_prim_arg.status = res;

    osel_event_t event;
    event.sig = MAC2NWK_PRIM_EVENT;
    event.param = sbuf;
    osel_post(NULL, &mac2nwk_process, &event);
}

static void nwk_mac_init(void)
{
    //@todo 添加mac的注册接口实现
    mac_dependent_t mac_cfg;
    mac_cfg.recv_cb    = mac_data_recv;
    mac_cfg.send_cb    = mac_data_sent;
    mac_cfg.mac_assoc_cb = mac_assoc_confirm;

    mac_dependent_cfg(&mac_cfg);
}

static int8_t nwk_heartbeat_send(uint8_t status)
{
    nwk_heart_alarm_enum_t alarm_enum = NWK_HEART_ALARM_NONE;
    if (status)   //*< STARTUP
    {
        if (nwk.first_startup)
        {
            nwk.first_startup = FALSE;
            alarm_enum = NWK_HEART_ALARM_RESTART;
        }
        else
        {
            alarm_enum = NWK_HEART_ALARM_REJOIN;
        }
    }

    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    if (sbuf == NULL)
    {
        return -1;
    }

    pbuf_t *pbuf = pbuf_alloc(MEDIUM_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        sbuf_free(&sbuf __SLINE2);
        return -2;
    }
    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = NWK_LAYER;

    nwk_hd_addr_t nwk_addr;
    nwk_addr.nwk_hd_ctl.frm_ctrl = NWK_FRM_TYPE_HEART_BEAT;
    nwk_addr.nwk_hd_ctl.dst_mode = NWK_ADDR_MODE_SHORT;
    nwk_addr.nwk_hd_ctl.src_mode = NWK_ADDR_MODE_SHORT;
    nwk_addr.nwk_hd_ctl.reserved = 0x00;
    nwk_addr.seq_num  = nwk.nwk_seq++;
    nwk_addr.dst_addr = sink_nwk_address;
    nwk_addr.src_addr = nwk.nwk_addr;

    uint8_t len1 = nwk_frames_hd_addr_fill(pbuf, &nwk_addr);
    if (len1 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    nwk_heartbeat_t nwk_hb;
    nwk_hb.device_status.device_type     = NWK_HEART_DEV_TYPE_TAG;
    nwk_hb.device_status.energy_support  = NWK_HEART_ENERGY_BATTERY;
    nwk_hb.device_status.transmission_en = FALSE;
    nwk_hb.device_status.alarm_info      = alarm_enum;
    nwk_hb.device_status.localization    = FALSE;

    nwk_hb.father_id = nwk.father_id;
    //@todo:
//    nwk_hb.residual_energy = hal_energy_get();
    if (nwk_hb.device_status.transmission_en)
    {
        nwk_hb.transmission = nwk_prim_success_rate_get();
    }

    if (nwk_hb.device_status.alarm_info != NWK_HEART_ALARM_NONE)
    {
        nwk_hb.alarm_info = 0xAA55;
    }
    uint8_t len2 = nwk_frames_heartbeat_fill(pbuf, &nwk_hb);
    if (len2 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    mac_node_addr_t next_hop;
    next_hop.mode = MAC_MHR_ADDR_MODE_SHORT;
    next_hop.short_addr = mac_pib_coord_short_addr_get();
    sbuf->up_down_link = UP_LINK;
    nwk_send_data_packet(sbuf, next_hop, len1 + len2);

    return 0;
}


static int8_t nwk_frm_join_response_event(sbuf_t *sbuf, nwk_hd_addr_t *nwk_hd)
{
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    nwk_join_resp_t nwk_join_resp;
    if (nwk_frames_join_resp_get(pbuf, &nwk_join_resp) == 0)
    {
        return -3;
    }

    if (nwk_join_resp.nui == nwk.node_nui)   //*< resp to local
    {
        osel_pthread_exit(nwk_task_tcb, &nwk_join_process, PROCESS_CURRENT());

        if (nwk_join_resp.join_res == NET_JOIN_OK)
        {
            nwk.join_cnt    = 0;
            nwk.father_id   = nwk_join_resp.father_id;
            nwk.nwk_addr    = nwk_join_resp.nwk_id;
            nwk.hop_num     = nwk_join_resp.hop_num;
            nwk.join_status = NET_JOIN_OK;

            route_entry_t route_entry;
            route_entry.hop_num = nwk_join_resp.hop_num;
            route_entry.next_hop = sbuf->primargs.prim_arg.mac_prim_arg.src_addr;
            route_entry.nui = (uint16_t)nwk_hd->src_addr;   //*< 网关的NUI用网络短地址替代
            route_entry.nwk_addr = (uint16_t)nwk_hd->src_addr;
            nwk_route_module_insert(&route_entry);

            osel_pthread_create(nwk_task_tcb, &nwk_cycle_process, NULL);

            if (nwk.dependent.nwk_join_indicate != NULL)
            {
                nwk.dependent.nwk_join_indicate(TRUE);
            }
        }
        else
        {
            /** 入网如果失败，需要把关联表的上级节点信息置为不可关联 */
            nwk.join_cnt    = 0;
            nwk.nwk_addr    = 0;
            nwk.hop_num     = 0;
            nwk.join_status = nwk_join_resp.join_res;

            osel_pthread_exit(nwk_task_tcb, &nwk_cycle_process, PROCESS_CURRENT());
            mac_assoc_again(pbuf->attri.src_id);
        }
    }

    pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
    sbuf_free(&sbuf __SLINE2);
    return 0;
}



static bool_t nwk_is_point_to_local(nwk_hd_addr_t *nwk_hd)
{
    if ((nwk_hd->nwk_hd_ctl.dst_mode == NWK_ADDR_MODE_LONG)
            && (nwk_hd->dst_addr == nwk.node_nui))
    {
        return TRUE;
    }
    else if ((nwk_hd->nwk_hd_ctl.dst_mode == NWK_ADDR_MODE_SHORT)
             && ((uint16_t)(nwk_hd->dst_addr) == nwk.nwk_addr))
    {
        return TRUE;
    }

    return FALSE;
}


static void nwk_frm_join_request_event(sbuf_t *sbuf, nwk_hd_addr_t *nwk_hd)
{
    pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
    sbuf_free(&sbuf __SLINE2);
}

static void nwk_frm_route_response_event(sbuf_t *sbuf, nwk_hd_addr_t *nwk_hd)
{
    nwk_route_resp_t nwk_route_resp;
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    nwk_frames_route_resp_get(pbuf, &nwk_route_resp);
    route_entry_t route_entry;
    if (!nwk_is_point_to_local(nwk_hd))
    {
        //*< 只保留中继的路由
        ;
    }
    else
    {
        route_entry.nui = nwk_route_resp.dst_id;
        route_entry.nwk_addr = nwk_route_resp.dst_nwk_id;
        route_entry.next_hop = sbuf->primargs.prim_arg.mac_prim_arg.src_addr;

        osel_int8_t s = 0;
        s = osel_mutex_lock(OSEL_MAX_PRIO);
        bool_t res =  nwk_route_module_insert(&route_entry);
        osel_mutex_unlock(s);

        DBG_ASSERT(res == TRUE __DBG_LINE);

        osel_event_t event;
        event.sig = NWK_QUERY_RESP_EVENT;
        event.param = NULL;
        osel_post(NULL, &app2nwk_process, &event);
    }

    pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
    sbuf_free(&sbuf __SLINE2);
}

static void nwk_frm_data_event(sbuf_t *sbuf, nwk_hd_addr_t *nwk_hd)
{
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    if (nwk.join_status != NET_JOIN_OK)
    {
        pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return;
    }

    if (nwk_is_point_to_local(nwk_hd))
    {
        route_entry_t route_entry;
        route_entry.nui = nwk_hd->src_addr;
        route_entry.nwk_addr = nwk_hd->src_addr;
        route_entry.next_hop = sbuf->primargs.prim_arg.mac_prim_arg.src_addr;

        osel_int8_t s = 0;
        s = osel_mutex_lock(OSEL_MAX_PRIO);
        bool_t res =  nwk_route_module_insert(&route_entry);
        osel_mutex_unlock(s);

        DBG_ASSERT(res == TRUE __DBG_LINE);

        nwk_prim_arg_t *nwk_prim_arg = &(sbuf->primargs.prim_arg.nwk_prim_arg);
        nwk_prim_arg->nsdu = pbuf->data_p;
        nwk_prim_arg->nsdu_length = pbuf->data_len;

        sbuf->primtype = N2A_DATA_INDICATION;

        if (nwk.dependent.nwk_data_indicate != NULL)
        {
            nwk.dependent.nwk_data_indicate(pbuf->data_p, pbuf->data_len);
        }
    }

    pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
    sbuf_free(&sbuf __SLINE2);
}


static void nwk_frame_parse(sbuf_t *sbuf, nwk_hd_addr_t *nwk_hd)
{
    switch (nwk_hd->nwk_hd_ctl.frm_ctrl)
    {
    case NWK_FRM_TYPE_DATA:
        nwk_frm_data_event(sbuf, nwk_hd);
        break;

    case NWK_FRM_TYPE_JOIN_REQ:
        nwk_frm_join_request_event(sbuf, nwk_hd);
        break;

    case NWK_FRM_TYPE_JOIN_RESP:
        nwk_frm_join_response_event(sbuf, nwk_hd);
        break;

    case NWK_FRM_TYPE_HEART_BEAT:
        pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        break;

    case NWK_FRM_TYPE_ROUTE_REQ:
        pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        break;

    case NWK_FRM_TYPE_ROUTE_RESP:
        nwk_frm_route_response_event(sbuf, nwk_hd);
        break;

    default:
        pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
        sbuf_free(&sbuf __SLINE2);
        break;
    }
}

static int8_t mac2nwk_data_indication(sbuf_t *sbuf)
{
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    nwk_hd_addr_t nwk_hd;
    // 1，帧头地址，类型解析
    if (!nwk_frames_hd_addr_get(pbuf, &nwk_hd))
    {
        return -3;
    }

    nwk_frame_parse(sbuf, &nwk_hd);
    return 0;
}

static bool_t nwk_join_req_struct(nwk_hd_addr_t *nwk_addr,
                                  nwk_join_req_t *join_req)
{
    nwk_addr->nwk_hd_ctl.frm_ctrl = NWK_FRM_TYPE_JOIN_REQ;
    nwk_addr->nwk_hd_ctl.dst_mode = NWK_ADDR_MODE_SHORT;
    nwk_addr->nwk_hd_ctl.src_mode = NWK_ADDR_MODE_SHORT;
    nwk_addr->nwk_hd_ctl.reserved = 0x00;
    nwk_addr->seq_num  = nwk.nwk_seq++;
    nwk_addr->dst_addr = sink_nwk_address;
    nwk_addr->src_addr = 0x0000;

    join_req->device_type = NWK_HEART_DEV_TYPE_TAG;
    join_req->father_id   = sink_nwk_address;
    join_req->nui         = nwk.node_nui;

    hal_time_t now = hal_timer_now();
    m_sync_l2g(&now);
    join_req->join_apply_time = now.w;

    return TRUE;
}


static void nwk_send_data_packet(sbuf_t *sbuf,
                                 mac_node_addr_t next_hop,
                                 uint8_t len)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);

    mac_prim_arg_t *mac_prim_arg = &(sbuf->primargs.prim_arg.mac_prim_arg);
    mac_prim_arg->src_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_prim_arg->dst_mode = next_hop.mode;

    if (next_hop.mode == MAC_MHR_ADDR_MODE_SHORT)
    {
        mac_prim_arg->dst_addr = next_hop.short_addr;
    }
    else if (next_hop.mode == MAC_MHR_ADDR_MODE_LONG)
    {
        mac_prim_arg->dst_addr = next_hop.long_addr;
    }

    pbuf_t *pbuf = sbuf->primargs.pbuf;
    pbuf->attri.already_send_times.nwk_send_times++;
    sbuf->primtype = N2M_DATA_REQUEST;

    pbuf->data_p = pbuf->head + MAC_OFFSET_SIZE;

    mac_prim_arg->msdu = pbuf->data_p;
    mac_prim_arg->msdu_length = len;
    send_frames_cnt++;

    mac_send(sbuf);
}

static int8_t nwk_join_request_packed(void)
{
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    if (sbuf == NULL)
    {
        return -1;
    }

    pbuf_t *pbuf = pbuf_alloc((MAC_OFFSET_SIZE + sizeof(nwk_hd_addr_t)
                               + sizeof(nwk_join_req_t)) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        sbuf_free(&sbuf __SLINE2);
        return -2;
    }

    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = NWK_LAYER;

    nwk_hd_addr_t nwk_hd_addr;
    nwk_join_req_t nwk_join_req;
    nwk_join_req_struct(&nwk_hd_addr, &nwk_join_req);

    // 如果按照设计规范，应该申请一个pbuf，填充数据以后发送mac，由mac来重新申请新的pbuf
    pbuf->data_p = pbuf->head + MAC_OFFSET_SIZE;

    uint8_t len1 = nwk_frames_hd_addr_fill(pbuf, &nwk_hd_addr);
    if (len1 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    uint8_t len2 = nwk_frames_join_req_fill(pbuf, &nwk_join_req);
    if (len2 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    mac_node_addr_t next_hop;
    next_hop.mode = MAC_MHR_ADDR_MODE_SHORT;
    next_hop.short_addr = mac_pib_coord_short_addr_get();
    sbuf->up_down_link = UP_LINK;
    nwk_send_data_packet(sbuf, next_hop, len1 + len2);
    return 0;
}

static void mac2nwk_assoc_confirm(sbuf_t *sbuf)
{
    if (sbuf->primargs.prim_arg.mac_prim_arg.status)
    {
        osel_pthread_create(nwk_task_tcb, &nwk_join_process, NULL);
    }
    else
    {
        osel_pthread_exit(nwk_task_tcb, &nwk_join_process, PROCESS_CURRENT());
        nwk_stop();
        nwk_run();
    }

    sbuf_free(&sbuf __SLINE2);
}

static void nwk_resend_data_packet(sbuf_t *sbuf)
{
    DBG_LOG(DBG_LEVEL_INFO, "nwk data resend\r\n");
    uint8_t len =
        sbuf->primargs.prim_arg.mac_prim_arg.msdu_length;
    mac_node_addr_t next_hop;
    next_hop.mode = MAC_MHR_ADDR_MODE_SHORT;
    next_hop.short_addr = sbuf->primargs.prim_arg.mac_prim_arg.dst_addr;
    sbuf->up_down_link = UP_LINK;

    nwk_send_data_packet(sbuf, next_hop, len);
}

static void nwk_data_confirm_to_app(sbuf_t *sbuf)
{
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    nwk_prim_arg_t *nwk_prim_arg = &(sbuf->primargs.prim_arg.nwk_prim_arg);

    nwk_hd_addr_t nwk_hd;
    nwk_frames_hd_addr_get(pbuf, &nwk_hd);
    nwk_prim_arg->status = sbuf->primargs.prim_arg.mac_prim_arg.status;
    nwk_prim_arg->nsdu = pbuf->data_p;
    nwk_prim_arg->nsdu_length = pbuf->data_len;

    if (nwk.dependent.nwk_data_confirm != NULL)
    {
        nwk.dependent.nwk_data_confirm(sbuf);
    }
    else
    {
        pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
        sbuf_free(&sbuf __SLINE2);
    }
}


static int8_t mac2nwk_data_confirm_layer_handle(sbuf_t *sbuf)
{
    if (sbuf->orig_layer == APP_LAYER)
    {
        nwk_data_confirm_to_app(sbuf);
    }
    else
    {
        if (sbuf->primargs.pbuf != NULL)
        {
            pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
        }
        sbuf_free(&sbuf __SLINE2);
    }

    return 0;
}

static int8_t mac2nwk_data_confirm(sbuf_t *sbuf)
{
    DBG_ASSERT(NULL != sbuf __DBG_LINE);
    int8_t ret = 0;
    bool_t res =
        sbuf->primargs.prim_arg.mac_prim_arg.status;
    uint8_t cnt =
        sbuf->primargs.pbuf->attri.already_send_times.nwk_send_times;

    if (res)
    {
        send_success_cnt++;
        ret = mac2nwk_data_confirm_layer_handle(sbuf);
    }
    else
    {
        if (cnt >= NWK_MAX_SEND_TIMES)
        {
            ret = mac2nwk_data_confirm_layer_handle(sbuf);
        }
        else
        {
            if (nwk.join_status != NET_JOIN_OK)
            {
                ret = mac2nwk_data_confirm_layer_handle(sbuf);
            }
            else
            {
                nwk_resend_data_packet(sbuf);
            }
        }
    }

    return ret;
}

static void mac2nwk_prim_event(sbuf_t *sbuf)
{
    DBG_ASSERT(NULL != sbuf __DBG_LINE);

    switch (sbuf->primtype)
    {
    case M2N_DATA_INDICATION:
        mac2nwk_data_indication(sbuf);
        break;

    case M2N_DATA_CONFIRM:
        mac2nwk_data_confirm(sbuf);
        break;

    case M2N_ASSOC_CONFIRM:
        mac2nwk_assoc_confirm(sbuf);
        break;

    default:
        pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
        sbuf_free(&sbuf __SLINE2);
        break;
    }
}

static int8_t nwk_route_query_discovery(uint64_t key)
{
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    if (sbuf == NULL)
    {
        return -1;
    }
    uint8_t len = NWK_OFFSET_SIZE + sizeof(key);
    pbuf_t *pbuf = pbuf_alloc(len __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        sbuf_free(&sbuf __SLINE2);
        return -2;
    }
    sbuf->primargs.pbuf = pbuf;

    nwk_hd_addr_t nwk_hd;
    nwk_hd.nwk_hd_ctl.frm_ctrl = NWK_FRM_TYPE_ROUTE_REQ;
    nwk_hd.nwk_hd_ctl.dst_mode = NWK_ADDR_MODE_SHORT;
    nwk_hd.nwk_hd_ctl.src_mode = NWK_ADDR_MODE_SHORT;
    nwk_hd.qos_level = QOS_LEVEL_DATA;
    nwk_hd.seq_num   = nwk.nwk_seq++;
    nwk_hd.dst_addr  = (nwk.nwk_addr & 0xFF00) | 0x0001;
    nwk_hd.src_addr  = nwk.nwk_addr;

    uint8_t len1 = nwk_frames_hd_addr_fill(pbuf, &nwk_hd);
    if (len1 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    nwk_route_req_t route_req;
    route_req.dst_id = key;

    uint8_t len2 = nwk_frames_route_req_fill(pbuf, &route_req);
    if (len2 == 0)
    {
        pbuf_free(&pbuf __PLINE2);
        sbuf_free(&sbuf __SLINE2);
        return -3;
    }

    mac_node_addr_t next_hop;
    next_hop.mode = MAC_MHR_ADDR_MODE_SHORT;
    next_hop.short_addr = mac_pib_coord_short_addr_get();
    sbuf->up_down_link = UP_LINK;
    nwk_send_data_packet(sbuf, next_hop, pbuf->data_len);

    return 0;
}

static void nwk_query_timeout_event(sbuf_t *sbuf)
{
    if (sbuf->was_armed == TRUE)
    {
        return;
    }

    list_del(&(sbuf->list));
    if (nwk.dependent.nwk_data_confirm != NULL)
    {
        nwk.dependent.nwk_data_confirm(sbuf);
    }
    else
    {
        pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
        sbuf_free(&sbuf __SLINE2);
    }
}


PROCESS(nwk_join_process, "nwk_join_process");
PROCESS_THREAD(nwk_join_process, ev, data)
{
    PROCESS_BEGIN();

    nwk.join_cnt = 0;

    while (1)
    {
        for (; nwk.join_cnt < MAX_JION_NET_CNT; nwk.join_cnt++)
        {
            nwk_join_request_packed();
            OSEL_ETIMER_DELAY(&(nwk.join_etimer), 500 * mac_pib_hops_get() * 2); // 500*10ms*hops*2
        }

        nwk.join_cnt = 0;
        nwk_stop();
        nwk_run();

        PROCESS_EXIT();
    }

    PROCESS_END();
}

PROCESS(nwk_cycle_process, "nwk_cycle_process");
PROCESS_THREAD(nwk_cycle_process, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
        OSEL_ETIMER_DELAY(&(nwk.cycle_etimer), NWK_CYCLE_MAX_TICKS);

        //@note 终端设备路由建立以后不主动剔除
        //更新网关的心跳
//        nwk_route_module_refresh(sink_nwk_address);
//        nwk_route_module_update();

        nwk_heartbeat_send(TRUE);
    }

    PROCESS_END();
}

PROCESS(mac2nwk_process, "nwk recv up process");
PROCESS_THREAD(mac2nwk_process, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
        if (ev == MAC2NWK_PRIM_EVENT)
        {
            mac2nwk_prim_event((sbuf_t *)(data));
        }

        PROCESS_YIELD();
    }

    PROCESS_END();
}

static void nwk_send_app_data_handle(sbuf_t *sbuf, route_entry_t *entry)
{
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    nwk_hd_addr_t nwk_hd;
    nwk_hd.nwk_hd_ctl.frm_ctrl = NWK_FRM_TYPE_DATA;
    nwk_hd.nwk_hd_ctl.dst_mode = NWK_ADDR_MODE_SHORT;
    nwk_hd.nwk_hd_ctl.src_mode = NWK_ADDR_MODE_SHORT;
    nwk_hd.nwk_hd_ctl.reserved = 0x00;
    nwk_hd.qos_level           = QOS_LEVEL_DATA;
    nwk_hd.seq_num             = nwk.nwk_seq++;
    nwk_hd.dst_addr            = entry->nwk_addr;
    nwk_hd.src_addr            = nwk.nwk_addr;

    uint8_t len1 = nwk_frames_hd_addr_fill(pbuf, &nwk_hd);
    DBG_ASSERT(len1 != 0 __DBG_LINE);
    if (len1 == 0)
    {
        DBG_LOG(DBG_LEVEL_INFO, "pbuf small\n");
        sbuf->primargs.prim_arg.nwk_prim_arg.status = FALSE;
        if (nwk.dependent.nwk_data_confirm != NULL)
        {
            nwk.dependent.nwk_data_confirm(sbuf);
        }
        else
        {
            pbuf_free(&pbuf __PLINE2);
            sbuf_free(&sbuf __SLINE2);
        }
    }
    else
    {
        mac_node_addr_t next_hop;
        next_hop.mode = MAC_MHR_ADDR_MODE_SHORT;
        next_hop.short_addr = entry->next_hop;
        sbuf->up_down_link = UP_LINK;
        nwk_send_data_packet(sbuf, next_hop, pbuf->data_len);
    }
}


PROCESS(app2nwk_process, "app2nwk_process");
PROCESS_THREAD(app2nwk_process, ev, data)
{
    route_entry_t *route_entry = NULL;
    osel_int8_t s = 0;

    PROCESS_BEGIN();

    static sbuf_t *sbuf = NULL;
    static uint64_t key = 0;

    while (1)
    {
        if (!list_empty(&query_head))
        {
            sbuf = (sbuf_t *)list_next_elem_get(&query_head);
            if (nwk.join_status != NET_JOIN_OK)
            {
                pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
                sbuf_free(&sbuf __SLINE2);
            }
            else if (sbuf->primtype == A2N_DATA_REQUEST)
            {
                key = sbuf->primargs.prim_arg.nwk_prim_arg.key;

                s = osel_mutex_lock(OSEL_MAX_PRIO);
                route_entry = nwk_route_modele_get(key);
                osel_mutex_unlock(s);

                if (route_entry == NULL)
                {
                    DBG_LOG(DBG_LEVEL_INFO, "can't find nodedata with this NUI,\
                        need startup route_query\r\n");
                    nwk_route_query_discovery(key);
                    osel_etimer_ctor(&(sbuf->etimer), PROCESS_CURRENT(),
                                     PROCESS_EVENT_TIMER, NULL);
                    osel_etimer_arm(&(sbuf->etimer), 2000, 0);  //*< ONE shot for 20 sec
                    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) ||
                                             (ev == NWK_QUERY_RESP_EVENT));
                    if (ev == PROCESS_EVENT_TIMER)
                    {
                        nwk_query_timeout_event(sbuf);
                    }
                    else if (ev == NWK_QUERY_RESP_EVENT)
                    {
                        s = osel_mutex_lock(OSEL_MAX_PRIO);
                        route_entry = nwk_route_modele_get(key);
                        osel_mutex_unlock(s);

                        if (route_entry != NULL)
                        {
                            nwk_send_app_data_handle(sbuf, route_entry);
                        }
                    }
                }
                else
                {
                    nwk_send_app_data_handle(sbuf, route_entry);
                }
            }
            else
            {
                pbuf_free(&(sbuf->primargs.pbuf)__PLINE2);
                sbuf_free(&sbuf __SLINE2);
            }
        }
        PROCESS_YIELD();
    }
    PROCESS_END();
}


void nwk_init(void)
{
    osel_memset(&nwk, 0x00, sizeof(nwk_t));

    nwk_task_tcb = osel_task_create(NULL, NWK_TASK_PRIO, nwk_event_store, NWK_EVENT_MAX);

    osel_pthread_create(nwk_task_tcb, &mac2nwk_process, NULL);
    osel_pthread_create(nwk_task_tcb, &app2nwk_process, NULL);

    nwk.live_cnt = 10;
    nwk.join_status = NET_JOIN_LIC_ADDR;
    nwk_mac_init();
    nwk_route_module_init(nwk.live_cnt);

    nwk.first_startup = TRUE;

    list_init(&query_head);
}


void nwk_run(void)
{
    //@todo 保证网络打开
    if (nwk.join_status == NET_JOIN_OK)
    {
        return;
    }

    osel_int8_t s = 0;
    s = osel_mutex_lock(OSEL_MAX_PRIO);
    osel_etimer_disarm(&(nwk.join_etimer));

    nwk.nwk_addr    = 0x0000;
    //@TODO : add mac.h interface
    nwk.node_nui    = NODE_ID;
    nwk.join_cnt    = 0;
    nwk.join_status = NET_JOIN_LIC_ADDR;
    nwk.hop_num     = 0;
    nwk.live_cnt    = 0;
    nwk.nwk_seq     = 0;
    osel_mutex_unlock(s);
    mac_run();
}


void nwk_stop(void)
{
    //@todo 保证网络关闭
    osel_int8_t s = 0;
    s = osel_mutex_lock(OSEL_MAX_PRIO);
    osel_pthread_exit(nwk_task_tcb, &nwk_cycle_process, PROCESS_CURRENT());
    osel_etimer_disarm(&(nwk.join_etimer));

    nwk_route_module_init(nwk.live_cnt);

    nwk.nwk_addr    = 0x0000;
    nwk.join_cnt    = 0;
    nwk.join_status = NET_JOIN_LIC_ADDR;
    nwk.hop_num     = 0;
    nwk.live_cnt    = 0;
    nwk.nwk_seq     = 0;

    sbuf_t *pos1 = NULL;
    sbuf_t *pos2 = NULL;
    list_entry_for_each_safe(pos1, pos2 , &query_head, sbuf_t, list)
    {
        list_del(&(pos1->list));

        pbuf_t *pbuf = pos1->primargs.pbuf;
        nwk_prim_arg_t *nwk_prim_arg = &(pos1->primargs.prim_arg.nwk_prim_arg);

        nwk_hd_addr_t nwk_hd;
        nwk_frames_hd_addr_get(pbuf, &nwk_hd);
        nwk_prim_arg->status = pos1->primargs.prim_arg.mac_prim_arg.status;
        nwk_prim_arg->nsdu = pbuf->data_p;
        nwk_prim_arg->nsdu_length = pbuf->data_len;

        if (nwk.dependent.nwk_data_confirm != NULL)
        {
            nwk.dependent.nwk_data_confirm(pos1);
        }
        else
        {
            pbuf_free(&pbuf __PLINE2);
            sbuf_free(&pos1 __SLINE2);
        }
    }
    osel_mutex_unlock(s);

    mac_stop();
}

int8_t nwk_send(uint64_t nui, uint8_t *const data, uint8_t len)
{
   if (nwk.join_status != NET_JOIN_OK) {
       return -1;
   }

    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    if (sbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_INFO, "sbuf is null\n");
        return -1;
    }

    pbuf_t *pbuf = pbuf_alloc(NWK_OFFSET_SIZE + len __PLINE1);
    if (pbuf == NULL)
    {
        sbuf_free(&sbuf __SLINE2);
        return -1;
    }
    sbuf->primargs.pbuf = pbuf;
    sbuf->primargs.prim_arg.nwk_prim_arg.key = nui;

    pbuf->data_p = pbuf->head + NWK_OFFSET_SIZE;
    osel_memcpy(pbuf->data_p, (uint8_t *)data, len);
    pbuf->data_len = len;

    sbuf->primtype = A2N_DATA_REQUEST;

    osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    list_add_to_head(&(sbuf->list), &query_head);
    OSEL_EXIT_CRITICAL(s);

    osel_event_t event;
    event.sig = APP2NWK_PRIM_EVENT;
    event.param = sbuf;
    osel_post(NULL, &app2nwk_process, &event);

    return 0;
}









