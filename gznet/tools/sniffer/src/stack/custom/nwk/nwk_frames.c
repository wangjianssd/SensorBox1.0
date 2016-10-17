#include "pbuf.h"
#include "lib.h"

#include "mac.h"
#include "nwk_frames.h"

DBG_THIS_MODULE("nwk frames")

uint8_t nwk_frames_hd_addr_get(pbuf_t *pbuf, nwk_hd_addr_t *nwk_addr)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }

    pbuf->data_p = pbuf->head + MAC_OFFSET_SIZE;

    uint8_t *p_data = pbuf->data_p;
    memcpy(&(nwk_addr->nwk_hd_ctl), pbuf->data_p, sizeof(nwk_hd_ctl_t));
    pbuf->data_p += sizeof(nwk_hd_ctl_t);

    memcpy(&(nwk_addr->qos_level), pbuf->data_p, sizeof(qos_level_t));
    pbuf->data_p += sizeof(qos_level_t);
    
    osel_memcpy(&(nwk_addr->seq_num), pbuf->data_p, sizeof(uint8_t));
    pbuf->data_p += sizeof(uint8_t);

    if (nwk_addr->nwk_hd_ctl.dst_mode == NWK_ADDR_MODE_SHORT)
    {
        uint16_t short_addr = 0x00ull;
        osel_memcpy(&short_addr, pbuf->data_p, NWK_ADDR_SHORT_SIZE);
        nwk_addr->dst_addr = short_addr;
        pbuf->data_p += NWK_ADDR_SHORT_SIZE;
    }
    else if (nwk_addr->nwk_hd_ctl.dst_mode == NWK_ADDR_LONG_SIZE)
    {
        uint64_t long_addr = 0x00ull;
        osel_memcpy(&long_addr, pbuf->data_p, NWK_ADDR_LONG_SIZE);
        nwk_addr->dst_addr = long_addr;
        pbuf->data_p += NWK_ADDR_LONG_SIZE;
    }

    if (nwk_addr->nwk_hd_ctl.src_mode == NWK_ADDR_MODE_SHORT)
    {
        uint64_t short_addr = 0x00ull;
        osel_memcpy(&short_addr, pbuf->data_p, NWK_ADDR_SHORT_SIZE);
        nwk_addr->src_addr = (uint16_t)short_addr;
        pbuf->data_p += NWK_ADDR_SHORT_SIZE;
    }
    else if (nwk_addr->nwk_hd_ctl.src_mode == NWK_ADDR_LONG_SIZE)
    {
        uint64_t long_addr = 0x00ull;
        osel_memcpy(&long_addr, pbuf->data_p, NWK_ADDR_LONG_SIZE);
        nwk_addr->src_addr = long_addr;
        pbuf->data_p += NWK_ADDR_LONG_SIZE;
    }

    uint8_t len = pbuf->data_p - p_data;
    pbuf->data_len -= len;

    return len;
}


uint8_t nwk_frames_hd_addr_fill(pbuf_t *pbuf, nwk_hd_addr_t *nwk_addr)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    
    pbuf->data_p = pbuf->head + MAC_OFFSET_SIZE;

    uint8_t *p_data = pbuf->data_p;
    memcpy(pbuf->data_p, &(nwk_addr->nwk_hd_ctl), sizeof(nwk_hd_ctl_t));
    pbuf->data_p += sizeof(nwk_hd_ctl_t);

    memcpy(pbuf->data_p, &(nwk_addr->qos_level), sizeof(qos_level_t));
    pbuf->data_p += sizeof(qos_level_t);
    
    osel_memcpy(pbuf->data_p, &(nwk_addr->seq_num), sizeof(uint8_t));
    pbuf->data_p += sizeof(uint8_t);

    if (nwk_addr->nwk_hd_ctl.dst_mode == NWK_ADDR_MODE_SHORT)
    {
        uint64_t short_addr = nwk_addr->dst_addr;
        osel_memcpy(pbuf->data_p, &short_addr, NWK_ADDR_SHORT_SIZE);
        pbuf->data_p += NWK_ADDR_SHORT_SIZE;
    }
    else if (nwk_addr->nwk_hd_ctl.dst_mode == NWK_ADDR_MODE_LONG)
    {
        uint64_t long_addr = nwk_addr->dst_addr;
        osel_memcpy(pbuf->data_p, &long_addr, NWK_ADDR_LONG_SIZE);
        pbuf->data_p += NWK_ADDR_LONG_SIZE;
    }

    if (nwk_addr->nwk_hd_ctl.src_mode == NWK_ADDR_MODE_SHORT)
    {
        uint64_t short_addr = nwk_addr->src_addr;
        osel_memcpy(pbuf->data_p, &short_addr, NWK_ADDR_SHORT_SIZE);
        pbuf->data_p += NWK_ADDR_SHORT_SIZE;
    }
    else if (nwk_addr->nwk_hd_ctl.src_mode == NWK_ADDR_MODE_LONG)
    {
        uint64_t long_addr = nwk_addr->src_addr;
        osel_memcpy(pbuf->data_p, &long_addr, NWK_ADDR_LONG_SIZE);
        pbuf->data_p += NWK_ADDR_LONG_SIZE;
    }
    
    uint8_t len = pbuf->data_p - p_data;
    pbuf->data_len += len;
    
    return len;
}

uint8_t nwk_frames_join_req_get(pbuf_t *pbuf, nwk_join_req_t *req)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }

    uint8_t *p_data = pbuf->data_p;

	osel_memcpy(req, pbuf->data_p, sizeof(nwk_join_req_t));
	pbuf->data_p += sizeof(nwk_join_req_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len -= len;

	return len;
}

uint8_t nwk_frames_join_req_fill(pbuf_t *pbuf, nwk_join_req_t *req)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    
    uint8_t *p_data = pbuf->data_p;

    osel_memcpy(pbuf->data_p, req, sizeof(nwk_join_req_t));
	pbuf->data_p += sizeof(nwk_join_req_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len += len;

	return len;
}

uint8_t nwk_frames_join_resp_get(pbuf_t *pbuf, nwk_join_resp_t *resp)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }

    uint8_t *p_data = pbuf->data_p;

	osel_memcpy(resp, pbuf->data_p, sizeof(nwk_join_resp_t));
	pbuf->data_p += sizeof(nwk_join_resp_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len -= len;

	return len;
}

uint8_t nwk_frames_join_resp_fill(pbuf_t *pbuf, nwk_join_resp_t *resp)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    uint8_t *p_data = pbuf->data_p;

    osel_memcpy(pbuf->data_p, resp, sizeof(nwk_join_resp_t));
	pbuf->data_p += sizeof(nwk_join_resp_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len += len;

	return len;
}

uint8_t nwk_frames_heartbeat_fill(pbuf_t *pbuf, nwk_heartbeat_t *hb)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    
    uint8_t *p_data = pbuf->data_p;
    
    osel_memcpy(pbuf->data_p, &(hb->device_status), sizeof(nwk_hb_status_t));
	pbuf->data_p += sizeof(nwk_hb_status_t);
    
    uint16_t father_id = hb->father_id;
    osel_memcpy(pbuf->data_p, &father_id, sizeof(uint16_t));
	pbuf->data_p += sizeof(uint16_t);
    
    osel_memcpy(pbuf->data_p, &hb->residual_energy, sizeof(uint8_t));
	pbuf->data_p += sizeof(uint8_t);
    
    if(hb->device_status.transmission_en)
    {
        osel_memcpy(pbuf->data_p, &hb->transmission, sizeof(uint8_t));
        pbuf->data_p += sizeof(uint8_t);
    }
    
    if(hb->device_status.alarm_info == NWK_HEART_ALARM_RESTART)
    {
        uint16_t alarm_info = hb->alarm_info;
        osel_memcpy(pbuf->data_p, &alarm_info, sizeof(uint16_t));
        pbuf->data_p += sizeof(uint16_t);
    }
    
    uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len += len;

	return len;
}

uint8_t nwk_frames_route_resp_get(pbuf_t *pbuf, nwk_route_resp_t *resp)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }

    uint8_t *p_data = pbuf->data_p;

	osel_memcpy(resp, pbuf->data_p, sizeof(nwk_route_resp_t));
	pbuf->data_p += sizeof(nwk_route_resp_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len -= len;

	return len;
}

uint8_t nwk_frames_route_req_fill(pbuf_t *pbuf, nwk_route_req_t *req)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    uint8_t *p_data = pbuf->data_p;

    osel_memcpy(pbuf->data_p, req, sizeof(nwk_route_req_t));
	pbuf->data_p += sizeof(nwk_route_req_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len += len;

	return len;
}

uint8_t nwk_frames_route_req_get(pbuf_t *pbuf, nwk_route_req_t *req)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    uint8_t *p_data = pbuf->data_p;
    
    osel_memcpy(req, pbuf->data_p, sizeof(nwk_route_req_t));
	pbuf->data_p += sizeof(nwk_route_req_t);

	uint8_t len = pbuf->data_p - p_data;
	pbuf->data_len -= len;

	return len;
}


uint8_t nwk_frames_data_get(pbuf_t *pbuf, void *datap, uint8_t len)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    
    pbuf->data_p = pbuf->head + NWK_OFFSET_SIZE;
    
    if(len > pbuf->data_len)
    {
        return 0;
    }

    osel_memcpy(datap, pbuf->data_p, len);
    
    return len;
}

uint8_t nwk_frames_data_fill(pbuf_t *pbuf, void *datap, uint8_t len)
{
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf is null\n");
        return 0;
    }
    
    pbuf->data_p = pbuf->head + NWK_OFFSET_SIZE;
    
    uint8_t write_len = pbuf->end - pbuf->data_p;

    if(len > write_len)
    {
        return 0;
    }
    
    osel_memcpy(pbuf->data_p, datap, len);
    
    return len;
}




