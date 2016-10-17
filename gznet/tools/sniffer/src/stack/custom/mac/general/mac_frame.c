#include "mac_frame.h"
#include <wsnos.h>
#include <prim.h>
void assoc_request_frame(pbuf_t *pbuf, mac_assoc_req_arg_t *assoc_req)
{
	osel_memset(assoc_req,0,sizeof(mac_assoc_req_arg_t));
    osel_memcpy(assoc_req, pbuf->data_p, sizeof(mac_assoc_req_arg_t));
}

bool_t assoc_response_frame(pbuf_t *pbuf, mac_pib_t *mac_pib)
{
    mac_assoc_res_arg_t res_arg;
    osel_memcpy((uint8_t *)&res_arg, pbuf->data_p, sizeof(mac_assoc_res_arg_t));
    pbuf->data_p += sizeof(mac_assoc_res_arg_t);
    if (res_arg.status != ASSOC_STATUS_SUCCESS)
    {
        return FALSE;
    }
    mac_pib->self_cluster_index = res_arg.index;
    mac_pib->coord_saddr = pbuf->attri.src_id;
    return TRUE;
}

void south_sbuf_fill(sbuf_t *sbuf)
{
	mac_head_t mac_head;
	uint8_t mac_offset =0;
	pbuf_t *pbuf = sbuf->primargs.pbuf;
	mac_prim_arg_t *mac_prim = &sbuf->primargs.prim_arg.mac_prim_arg;
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
	osel_memset(&mac_head, 0 , sizeof(mac_head_t));
	osel_memcpy(&(mac_head.ctrl), pbuf->data_p, MAC_HEAD_CTRL_SIZE);
	pbuf->data_p += MAC_HEAD_CTRL_SIZE;
    mac_offset += MAC_HEAD_CTRL_SIZE;
    mac_head.seq = (*pbuf->data_p);
    pbuf->data_p += MAC_HEAD_SEQ_SIZE;
    mac_offset += MAC_HEAD_SEQ_SIZE;
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head.ctrl.des_addr_mode, &mac_head.addr.dst_addr);
    mac_offset += get_addr(pbuf, (mac_addr_mode_e)mac_head.ctrl.src_addr_mode, &mac_head.addr.src_addr);
	mac_prim->dst_mode = mac_head.ctrl.des_addr_mode;
	mac_prim->src_mode = mac_head.ctrl.src_addr_mode;
	mac_prim->dst_addr = mac_head.addr.dst_addr;
	mac_prim->src_addr = mac_head.addr.src_addr;
	
	pbuf->attri.seq 		= mac_head.seq;
    pbuf->attri.already_send_times.mac_send_times = 0;
    pbuf->attri.send_mode 	= TDMA_SEND_MODE;
	pbuf->attri.src_id      = mac_head.addr.src_addr;
    pbuf->attri.dst_id 		= mac_head.addr.dst_addr;
	pbuf->attri.need_ack    = mac_head.ctrl.ack_req;
	pbuf->attri.mac_length = mac_offset;
}