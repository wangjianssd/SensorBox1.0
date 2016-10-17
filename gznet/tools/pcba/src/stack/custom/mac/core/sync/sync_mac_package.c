#include "sync_mac_package.h"
#include <stdlib.h>
static list_head_t down_list;
static list_head_t up_list;
static list_head_t cap_list;

static void diffarr(uint8_t *arr,uint8_t n,uint8_t start, uint8_t end)
{
	uint8_t index,*ptr=arr;
    uint8_t pfrom[50];
    uint8_t i = 0 ,j =0, k = 0;
    for(i = 0,j = start;j < end ; i++,j++)
        pfrom[i] =j;
    srand(TA0R);
    for(k=0;k<n;k++)
    {
        index=rand()% (end - start);
        if(pfrom[index]!=0)
        {
            *(ptr+k)=pfrom[index];
			pfrom[index]=0;
        }
    }
}

static uint16_t gts_get_seq(uint8_t cluster_index, supf_spec_t *param)
{
	uint16_t random_start = 0;
    uint16_t random_end = 0;
    uint16_t seq = 0;
    uint8_t beacon_cnt           = 1;
    uint8_t intra_gts_number        = param->intra_gts_number;
	random_start = ((intra_gts_number + beacon_cnt) * cluster_index) + 1;
	random_end = random_start + intra_gts_number - 1;
	seq = random_start;
    if(random_end > random_start)
    {
        seq = srandom(random_start, random_end);
    }
	return seq;
}

static uint16_t cap_get_seq(supf_spec_t *param)
{
    uint16_t random_start = 0;
    uint16_t random_end = 0;
    uint16_t seq = 0;
    uint8_t beacon_cnt           = 1;
    uint8_t intra_gts_number        = param->intra_gts_number;
    uint8_t intra_cap_number        = param->intra_cap_number;
    uint8_t cluster_number          = param->cluster_number;
    uint8_t inter_unit_number       = param->inter_unit_number;
    DBG_ASSERT(intra_cap_number != 0 __DBG_LINE);
    uint16_t inter_number = 0;
    for(uint8_t i = 0; i<inter_unit_number; i++)
    {
        inter_number+=param->inter_gts_number[i];
    }
    random_start = (beacon_cnt+intra_gts_number)*cluster_number + inter_number;
    random_end = random_start + intra_cap_number -2;	//最后一个CAP是不允许接收关联应答的，这里-1
    seq = random_start;
    if(random_end > random_start)
    {
        seq = srandom(random_start, random_end);
    }
    return seq;
}

void cap_list_insert(supf_spec_t *supf,sbuf_t *sbuf)
{
    if(sbuf->slot_seq == 0)
        sbuf->slot_seq = cap_get_seq(supf);
	uint8_t s = 0;
	OSEL_ENTER_CRITICAL(s);
    list_add_to_tail(&(sbuf->list), &cap_list);
	OSEL_EXIT_CRITICAL(s);
}

sbuf_t *cap_list_node_get(uint16_t index)
{
    sbuf_t *sbuf = NULL;
    if (list_empty(&cap_list))
    {
        return NULL;
    }
    sbuf_t *pos1 = NULL;
    sbuf_t *pos2 = NULL;
    list_entry_for_each_safe( pos1, pos2 , &cap_list, sbuf_t, list)
    {
        if (pos1->slot_seq == index)
        {
            uint8_t s = 0;
            OSEL_ENTER_CRITICAL(s);
            sbuf = pos1;
            list_del(&(pos1->list));
            OSEL_EXIT_CRITICAL(s);
            break;
        }
    }
    return sbuf;
}

void cap_list_node_clear(void)
{
    if (!list_empty(&cap_list))
    {
        sbuf_t *pos1 = NULL;
        sbuf_t *pos2 = NULL;
        list_entry_for_each_safe( pos1, pos2 , &cap_list, sbuf_t, list)
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

bool_t gts_list_empty(up_down_link_t mode)
{
	list_head_t *p_list;
	if(mode == UP_LINK)
	{	
		p_list = &up_list;
	}
	else
	{
		p_list = &down_list;
	}
	if (list_empty(p_list))
		return TRUE;
	else
		return FALSE;
}

void gts_list_sort(up_down_link_t mode,uint16_t start,uint16_t end)
{
	list_head_t *p_list;
	uint8_t count = 0;
	if(mode == UP_LINK)
	{	
		p_list = &up_list;
		if (!list_empty(p_list))
		{
			sbuf_t *pos1 = NULL;
			sbuf_t *pos2 = NULL;
			uint8_t index = 0;
			list_count(p_list,count);
			uint8_t array[50];
			diffarr(array, count, start,end);
			list_entry_for_each_safe( pos1, pos2 , p_list, sbuf_t, list)
			{
				uint8_t s = 0;
				OSEL_ENTER_CRITICAL(s);
				pos1->slot_seq = array[index++];
				OSEL_EXIT_CRITICAL(s);
			}
		}
	}
	else
	{
		p_list = &down_list;
	}
	
}

void gts_list_insert(uint8_t cluster_index, supf_spec_t *supf, sbuf_t *sbuf)
{
	sbuf->slot_seq = 0xFFFF;
	uint8_t s = 0;
	OSEL_ENTER_CRITICAL(s);
	if(sbuf->up_down_link == UP_LINK)
		list_add_to_tail(&(sbuf->list), &up_list);
	else
		list_add_to_tail(&(sbuf->list), &down_list);
	OSEL_EXIT_CRITICAL(s);
}

sbuf_t *gts_list_node_get(uint16_t index, up_down_link_t mode)
{
    sbuf_t *sbuf = NULL;
	sbuf_t *pos1 = NULL;
    sbuf_t *pos2 = NULL;
	bool_t empty = TRUE;
	list_head_t *p_list;
	if(mode == UP_LINK)
	{	
		empty = list_empty(&up_list);
		p_list = &up_list;
	}
	else
	{
		empty = list_empty(&down_list);
		p_list = &down_list;
	}
	if(empty)
	{
		return NULL;
	}
    list_entry_for_each_safe( pos1, pos2 , p_list, sbuf_t, list)
    {
        if (pos1->slot_seq == index)
        {
            uint8_t s = 0;
            OSEL_ENTER_CRITICAL(s);
            sbuf = pos1;
            list_del(&(pos1->list));
            OSEL_EXIT_CRITICAL(s);
            break;
        }
    }
    return sbuf;
}

static void gts_list_node_clear(up_down_link_t mode)
{
	list_head_t *p_list;
	if(mode == UP_LINK)
	{	
		p_list = &up_list;
	}
	else
	{
		p_list = &down_list;
	}
    if (!list_empty(p_list))
    {
        sbuf_t *pos1 = NULL;
        sbuf_t *pos2 = NULL;
        list_entry_for_each_safe( pos1, pos2 , p_list, sbuf_t, list)
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

void mac_beacon_package(sbuf_t *sbuf, mac_pib_t *mac_pib)
{
    pbuf_t *beacon_packet = sbuf->primargs.pbuf;
    beacon_packet->data_p = beacon_packet->head + PHY_HEAD_SIZE;
    mac_frm_ctrl_t *frm_ctrl = (mac_frm_ctrl_t *)(beacon_packet->data_p);
    frm_ctrl->frm_type = MAC_FRAME_TYPE_BEACON;
    frm_ctrl->sec_enable = FALSE;
    if (!list_empty(&down_list))
    {
        frm_ctrl->frm_pending = TRUE;
    }
    else
    {
        frm_ctrl->frm_pending = FALSE;
    }
    frm_ctrl->ack_req = FALSE;
    frm_ctrl->reseverd = 0;
    frm_ctrl->des_addr_mode = MAC_MHR_ADDR_MODE_FALSE;
    frm_ctrl->src_addr_mode = MAC_MHR_ADDR_MODE_SHORT;
    beacon_packet->data_p += sizeof(mac_frm_ctrl_t);
    *(beacon_packet->data_p) = mac_pib->mac_seq_num++;
    beacon_packet->data_p += MAC_HEAD_SEQ_SIZE;
    uint16_t src_addr = mac_short_addr_get( mac_pib->mac_addr);
    osel_memcpy(beacon_packet->data_p, &src_addr, MAC_ADDR_SHORT_SIZE);
    beacon_packet->data_p += MAC_ADDR_SHORT_SIZE;
	
    mac_pib->supf_cfg_arg.intra_channel = mac_pib->intra_channel;
    supf_spec_t supf_config =  mac_pib->supf_cfg_arg;
    uint16_t count = 0,short_addr_num = 0, long_addr_num= 0;
    if (!list_empty(&down_list))
    {
        list_count(&down_list,count);
        sbuf_t *pos1 = NULL;
        sbuf_t *pos2 = NULL;
        list_entry_for_each_safe( pos1, pos2 , &down_list, sbuf_t, list)
        {
            if (pos1->primargs.prim_arg.mac_prim_arg.dst_mode == MAC_MHR_ADDR_MODE_LONG)
            {
                long_addr_num++;
            }
        }
        short_addr_num = count - long_addr_num;
    }
    supf_config.down_link_slot_length = count> MAX_DOWN_GTS_NUM ? MAX_DOWN_GTS_NUM : count;
    osel_memcpy(beacon_packet->data_p, &supf_config, sizeof(supf_spec_t));
    beacon_packet->data_p += sizeof(supf_spec_t);
	
    if(count != 0)
    {
        long_addr_num = long_addr_num > 2 ? 2 : long_addr_num;
        short_addr_num = short_addr_num > (MAX_DOWN_GTS_NUM - long_addr_num) ? (MAX_DOWN_GTS_NUM - long_addr_num) : short_addr_num;
		
        pend_addr_spec_t pend_addr = {0,0};
        pend_addr.short_addr_num = short_addr_num;
        pend_addr.long_addr_num = long_addr_num;
        osel_memcpy(beacon_packet->data_p, &pend_addr, sizeof(pend_addr_spec_t));
        beacon_packet->data_p += sizeof(pend_addr_spec_t);
		
        mac_prim_arg_t *mac_prim_arg = NULL;
        uint16_t slot_seq = 0;
        uint8_t s;
        while(short_addr_num--)
        {
            if(!list_empty(&down_list))
            {
                sbuf_t *pos1 = NULL;
                sbuf_t *pos2 = NULL;
                list_entry_for_each_safe( pos1, pos2 , &down_list, sbuf_t, list)
                {
                    mac_prim_arg = &(pos1->primargs.prim_arg.mac_prim_arg);
                    if (mac_prim_arg->dst_mode == MAC_MHR_ADDR_MODE_SHORT)
                    {
                        osel_memcpy(beacon_packet->data_p, (uint8_t *)&(mac_prim_arg->dst_addr), MAC_ADDR_SHORT_SIZE);
                        beacon_packet->data_p += MAC_ADDR_SHORT_SIZE;
                        OSEL_ENTER_CRITICAL(s);
                        list_del(&(pos1->list));
                        OSEL_EXIT_CRITICAL(s);
                        pos1->slot_seq = slot_seq++;
                        list_add_to_tail(&pos1->list, &down_list);
                    }
                }
            }
        }
        while(long_addr_num--)
        {
            if(!list_empty(&down_list))
            {
                sbuf_t *pos1 = NULL;
                sbuf_t *pos2 = NULL;
                list_entry_for_each_safe( pos1, pos2 , &down_list, sbuf_t, list)
                {
                    mac_prim_arg = &(pos1->primargs.prim_arg.mac_prim_arg);
                    if (mac_prim_arg->dst_mode == MAC_MHR_ADDR_MODE_SHORT)
                    {
                        osel_memcpy(beacon_packet->data_p, (uint8_t *)&(mac_prim_arg->dst_addr), MAC_MHR_ADDR_MODE_LONG);
                        beacon_packet->data_p += MAC_MHR_ADDR_MODE_LONG;
                        OSEL_ENTER_CRITICAL(s);
                        list_del(&(pos1->list));
                        OSEL_EXIT_CRITICAL(s);
                        pos1->slot_seq = slot_seq++;
                        list_add_to_tail(&pos1->list, &down_list);
                    }
                }
            }
        }
    }
    bcn_payload_t beacon_payload;
    beacon_payload.index = mac_pib->self_cluster_index;
    beacon_payload.hops = mac_pib->hops;
    beacon_payload.time_stamp = m_slot_get_root_begin();
    osel_memcpy(beacon_packet->data_p, &beacon_payload, sizeof(bcn_payload_t));
    beacon_packet->data_p += sizeof(bcn_payload_t);
	
    beacon_packet->attri.need_ack = FALSE;
    beacon_packet->attri.already_send_times.mac_send_times = 0;
    beacon_packet->attri.send_mode = TDMA_SEND_MODE;
    beacon_packet->attri.dst_id = MAC_BROADCAST_ADDR;
    beacon_packet->data_len =  beacon_packet->data_p - beacon_packet->head - PHY_HEAD_SIZE;
}

void sync_mac_package_init(void)
{
    if (!list_empty(&up_list))
    {
        gts_list_node_clear(UP_LINK);
    }
    if (!list_empty(&down_list))
    {
        gts_list_node_clear(DOWN_LINK);
    }
    if (!list_empty(&cap_list))
    {
        cap_list_node_clear();
    }
    list_init(&up_list);
    list_init(&down_list);
    list_init(&cap_list);
}
