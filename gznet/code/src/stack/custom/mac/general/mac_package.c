#include "mac_package.h"
#include "neighbors.h"
static mac_package_t info;

void mac_ack_fill_package(pbuf_t *pbuf, uint8_t seqno, uint16_t des_addr)
{
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t frm_ctrl;
    frm_ctrl.frm_type = MAC_FRAME_TYPE_ACK;
    frm_ctrl.sec_enable = FALSE;
    frm_ctrl.frm_pending = FALSE;
    frm_ctrl.des_addr_mode = 0;
    frm_ctrl.src_addr_mode = 0;
    frm_ctrl.reseverd = 0;
    frm_ctrl.ack_req = FALSE;
	
    osel_memcpy(pbuf->data_p, &frm_ctrl, MAC_HEAD_CTRL_SIZE);
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    /* sequence num */
    osel_memcpy(pbuf->data_p, &seqno, MAC_HEAD_SEQ_SIZE);
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
	
    pbuf->data_len = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE;
	
    pbuf->attri.seq = seqno;
    pbuf->attri.is_ack = TRUE;
    pbuf->attri.need_ack = FALSE;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id = des_addr;
}

void mac_data_fill_package(sbuf_t *sbuf)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
	
    mac_prim_arg_t *mac_prim_arg = &(sbuf->primargs.prim_arg.mac_prim_arg);
    pbuf_t *pbuf = sbuf->primargs.pbuf;
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
	
    mac_frm_ctrl_t mac_frm_ctrl;
    mac_frm_ctrl.frm_type        = MAC_FRAME_TYPE_DATA;
    mac_frm_ctrl.sec_enable      = FALSE;    //这个根据要求设置
    mac_frm_ctrl.frm_pending     = FALSE;
    mac_frm_ctrl.ack_req         = TRUE;
    mac_frm_ctrl.reseverd        = 0;
    mac_frm_ctrl.des_addr_mode   = mac_prim_arg->dst_mode;
    mac_frm_ctrl.src_addr_mode   = mac_prim_arg->src_mode;
	
    osel_memcpy(pbuf->data_p, &mac_frm_ctrl, sizeof(mac_frm_ctrl_t));
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
	
    osel_memcpy(pbuf->data_p, &info.mac_pib->mac_seq_num, sizeof(uint8_t));
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
	
    if (mac_prim_arg->dst_mode == MAC_MHR_ADDR_MODE_SHORT)
    {
        osel_memcpy(pbuf->data_p, &(mac_prim_arg->dst_addr), MAC_ADDR_SHORT_SIZE);
        pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    }
    else if (mac_prim_arg->dst_mode == MAC_MHR_ADDR_MODE_LONG)
    {
        osel_memcpy(pbuf->data_p, &(mac_prim_arg->dst_addr), MAC_ADDR_LONG_SIZE);
        pbuf->data_p += MAC_ADDR_LONG_SIZE;
    }
	
    if (mac_prim_arg->src_mode == MAC_MHR_ADDR_MODE_SHORT)
    {
        uint16_t self_saddr = info.mac_pib->self_saddr;
        osel_memcpy(pbuf->data_p, &self_saddr, MAC_ADDR_SHORT_SIZE);
        pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    }
    else if (mac_prim_arg->src_mode == MAC_MHR_ADDR_MODE_LONG)
    {
        uint64_t mac_addr = info.mac_pib->mac_addr;
        osel_memcpy(pbuf->data_p, &mac_addr, MAC_ADDR_LONG_SIZE);
        pbuf->data_p += MAC_ADDR_LONG_SIZE;
    }
	
    pbuf->attri.seq = info.mac_pib->mac_seq_num++;
    pbuf->attri.already_send_times.mac_send_times = 0;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id = (uint16_t)mac_prim_arg->dst_addr;
	pbuf->attri.mac_length = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE + MAC_FCS_SIZE;
    pbuf->data_len += pbuf->attri.mac_length;
    pbuf->attri.need_ack = TRUE;
}

sbuf_t *mac_assoc_respond_package(mac_assoc_state_e state, uint16_t slot_seq,
                                  uint64_t des_addr, uint8_t cluster_index)
{
    pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(NULL != pbuf __DBG_LINE);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(NULL != sbuf __DBG_LINE);
    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = MAC_LAYER;
    sbuf->slot_seq = slot_seq + 1;
	
    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t mac_frm_ctrl;
    mac_frm_ctrl.frm_type        = MAC_FRAME_TYPE_COMMAND;
    mac_frm_ctrl.sec_enable      = FALSE;
    mac_frm_ctrl.frm_pending     = FALSE;
    mac_frm_ctrl.ack_req         = TRUE;
    mac_frm_ctrl.des_addr_mode   = MAC_MHR_ADDR_MODE_LONG;
    mac_frm_ctrl.src_addr_mode   = MAC_MHR_ADDR_MODE_SHORT;
    mac_frm_ctrl.reseverd 		 = 0;
    osel_memcpy(pbuf->data_p, &mac_frm_ctrl, sizeof(mac_frm_ctrl_t));
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    osel_memcpy(pbuf->data_p, &info.mac_pib->mac_seq_num, sizeof(uint8_t));
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    osel_memcpy(pbuf->data_p, &des_addr, MAC_ADDR_LONG_SIZE);
    pbuf->data_p += MAC_ADDR_LONG_SIZE;
    uint16_t mac_short = mac_short_addr_get(info.mac_pib->mac_addr);
    osel_memcpy(pbuf->data_p, &mac_short, MAC_ADDR_SHORT_SIZE);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    //载荷
    *(uint8_t *)pbuf->data_p = MAC_CMD_ASSOC_RESP;
    pbuf->data_p += sizeof(uint8_t);
    mac_assoc_res_arg_t assoc_res_arg;
    assoc_res_arg.status = state;
    assoc_res_arg.index = cluster_index;
    osel_memcpy(pbuf->data_p, &assoc_res_arg, sizeof(mac_assoc_res_arg_t));
    pbuf->data_p += sizeof(mac_assoc_res_arg_t);
	
    pbuf->attri.seq = info.mac_pib->mac_seq_num++;;
    pbuf->attri.need_ack = TRUE;
    pbuf->attri.already_send_times.mac_send_times= 0;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id = des_addr;                    //设备入网前phy给广播
	pbuf->attri.is_pending = FALSE;
    pbuf->data_len = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE;
    return sbuf;
}

sbuf_t *mac_assoc_request_package(void)
{
    pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(NULL != pbuf __DBG_LINE);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(NULL != sbuf __DBG_LINE);
    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = MAC_LAYER;

    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t mac_ctrl;
    mac_ctrl.frm_type      = MAC_FRAME_TYPE_COMMAND;
    mac_ctrl.sec_enable    = FALSE;
    mac_ctrl.frm_pending   = FALSE;
    mac_ctrl.ack_req       = FALSE;
    mac_ctrl.des_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_ctrl.src_addr_mode = MAC_MHR_ADDR_MODE_LONG;
    mac_ctrl.reseverd      = 0x00;
    osel_memcpy(pbuf->data_p, &mac_ctrl, sizeof(mac_frm_ctrl_t));
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    osel_memcpy(pbuf->data_p, &info.mac_pib->mac_seq_num, sizeof(uint8_t));
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    uint16_t corrd_saddr = info.mac_pib->coord_saddr;
    osel_memcpy(pbuf->data_p, (uint8_t *)(&corrd_saddr), MAC_ADDR_SHORT_SIZE);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    uint64_t mac_addr = info.mac_pib->mac_addr;
    osel_memcpy(pbuf->data_p, (void *)(&mac_addr), MAC_ADDR_LONG_SIZE);
    pbuf->data_p += MAC_ADDR_LONG_SIZE;
    //mac payload
    *(uint8_t *)pbuf->data_p = MAC_CMD_ASSOC_REQ;
    pbuf->data_p += sizeof(uint8_t);
    mac_assoc_req_arg_t associ_req_args;
    associ_req_args.device_type   = *info.drivce_type;
    associ_req_args.sec_cap       = FALSE;
    associ_req_args.beacon_bitmap = info.sync_attribute->local_beacon_map;
    hal_time_t now = hal_timer_now();
    m_sync_l2g(&now);
    associ_req_args.assoc_apply_time = now.w;
    osel_memcpy(pbuf->data_p, (uint8_t *)&associ_req_args, sizeof(mac_assoc_req_arg_t));
    pbuf->data_p += sizeof(mac_assoc_req_arg_t);
    if(associ_req_args.sec_cap)
    {
        osel_memcpy(pbuf->data_p, info.mac_pib->license, ARRAY_LEN(info.mac_pib->license));
        pbuf->data_p += ARRAY_LEN(info.mac_pib->license);
    }
    //pbuf attri
    pbuf->attri.seq       = info.mac_pib->mac_seq_num++;
    pbuf->attri.need_ack  = mac_ctrl.ack_req;
    pbuf->attri.already_send_times.mac_send_times = 0;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id    = info.mac_pib->coord_saddr;
	pbuf->attri.is_pending = TRUE;
    pbuf->data_len        = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE + MAC_FCS_SIZE;
    return sbuf;
}

sbuf_t *mac_auery_request_package(uint16_t des_addr, bool_t pending, uint32_t next_recv_time)
{
	pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(NULL != pbuf __DBG_LINE);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(NULL != sbuf __DBG_LINE);
    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = MAC_LAYER;

    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t mac_ctrl;
    mac_ctrl.frm_type      = MAC_FRAME_TYPE_COMMAND;
    mac_ctrl.sec_enable    = FALSE;
    mac_ctrl.frm_pending   = FALSE;
    mac_ctrl.ack_req       = FALSE;
    mac_ctrl.des_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_ctrl.src_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_ctrl.reseverd      = 0x00;
    osel_memcpy(pbuf->data_p, &mac_ctrl, sizeof(mac_frm_ctrl_t));
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    osel_memcpy(pbuf->data_p, &info.mac_pib->mac_seq_num, sizeof(uint8_t));
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    osel_memcpy(pbuf->data_p, (uint8_t *)&des_addr, MAC_ADDR_SHORT_SIZE);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    uint16_t self_saddr = info.mac_pib->self_saddr;
    osel_memcpy(pbuf->data_p, (void *)(&self_saddr), MAC_MHR_ADDR_MODE_SHORT);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    //mac payload
    *(uint8_t *)pbuf->data_p = MAC_CMD_WAKEUP_RESP;
    pbuf->data_p += sizeof(uint8_t);
	uint8_t ReqCount = 0;
	*(uint8_t *)pbuf->data_p = ReqCount;
	 pbuf->data_p += sizeof(uint8_t);
	 
	bcn_payload_t beacon_payload;
    beacon_payload.index = 0;
    beacon_payload.hops = info.mac_pib->hops;
    beacon_payload.time_stamp = next_recv_time;
    osel_memcpy(pbuf->data_p, &beacon_payload, sizeof(bcn_payload_t));
    pbuf->data_p += sizeof(bcn_payload_t);
		
    //pbuf attri
    pbuf->attri.seq       = info.mac_pib->mac_seq_num++;
    pbuf->attri.need_ack  = mac_ctrl.ack_req;
    pbuf->attri.already_send_times.mac_send_times = 0;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id    = des_addr;
	pbuf->attri.is_pending = pending;
    pbuf->data_len        = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE + MAC_FCS_SIZE;
    return sbuf;
}

sbuf_t *mac_auery_package(uint16_t des_addr, bool_t frm_pending, uint32_t next_recv_time)
{
	pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
    DBG_ASSERT(NULL != pbuf __DBG_LINE);
    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    DBG_ASSERT(NULL != sbuf __DBG_LINE);
    sbuf->primargs.pbuf = pbuf;
    sbuf->orig_layer = MAC_LAYER;

    pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t mac_ctrl;
    mac_ctrl.frm_type      = MAC_FRAME_TYPE_COMMAND;
    mac_ctrl.sec_enable    = FALSE;
    mac_ctrl.frm_pending   = frm_pending;
    mac_ctrl.ack_req       = FALSE;
    mac_ctrl.des_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_ctrl.src_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    mac_ctrl.reseverd      = 0x00;
    osel_memcpy(pbuf->data_p, &mac_ctrl, sizeof(mac_frm_ctrl_t));
    pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    osel_memcpy(pbuf->data_p, &info.mac_pib->mac_seq_num, sizeof(uint8_t));
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    osel_memcpy(pbuf->data_p, (uint8_t *)&des_addr, MAC_ADDR_SHORT_SIZE);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    uint16_t self_saddr = info.mac_pib->self_saddr;
    osel_memcpy(pbuf->data_p, (void *)(&self_saddr), MAC_ADDR_SHORT_SIZE);
    pbuf->data_p += MAC_ADDR_SHORT_SIZE;
    //mac payload
    *(uint8_t *)pbuf->data_p = MAC_CMD_WAKEUP;
    pbuf->data_p += sizeof(uint8_t);
	uint8_t ReqCount = 0;
    if(des_addr == 0xffff)
	{
		ReqCount = mac_neighbors_count();
	}
	*(uint8_t *)pbuf->data_p = ReqCount;
	 pbuf->data_p += sizeof(uint8_t);
	 
	bcn_payload_t beacon_payload;
    beacon_payload.index = 0;
    beacon_payload.hops = info.mac_pib->hops;
    beacon_payload.time_stamp = next_recv_time;
    osel_memcpy(pbuf->data_p, &beacon_payload, sizeof(bcn_payload_t));
    pbuf->data_p += sizeof(bcn_payload_t);
    //pbuf attri
    pbuf->attri.seq       = info.mac_pib->mac_seq_num++;
    pbuf->attri.need_ack  = mac_ctrl.ack_req;
    pbuf->attri.already_send_times.mac_send_times = 0;
    pbuf->attri.send_mode = TDMA_SEND_MODE;
    pbuf->attri.dst_id    = des_addr;
	pbuf->attri.is_pending = frm_pending;
    pbuf->data_len        = pbuf->data_p - pbuf->head - PHY_HEAD_SIZE + MAC_FCS_SIZE;
    return sbuf;
}

void mac_package_init(mac_package_t *mac_package)
{
    info.drivce_type = mac_package->drivce_type;
    info.mode = mac_package->mode;
    info.mac_pib = mac_package->mac_pib;
	info.sync_attribute = mac_package->sync_attribute;
}
