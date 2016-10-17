#include "assos_table.h"
static cluster_info_t cluster_table[CLUSTER_NUMBER];
static assoc_info_t assoc_table[ASSOC_NUM];
const uint8_t LIFE_CYCLE = 3;
const uint8_t CLUSTER_LIFE = 7;
static void cluster_node_clear(uint16_t index)
{
	cluster_table[index].mac_addr = 0;
	cluster_table[index].life = 0;
	cluster_table[index].cluster_index = 0;
}

static void cluster_table_remove(uint64_t addr, uint16_t *local_beacon_map,uint8_t cluster_number)
{
	uint16_t i = 0;
	for(i=0;i<cluster_number;i++)
	{	
		if(cluster_table[i].mac_addr == addr)
		{
			*local_beacon_map &=~(1<<(i+1));
			cluster_node_clear(i);
			return;
		}
	}
}

static bool_t cluster_table_add(uint64_t addr, uint16_t subordinate_beacon_map, 
								uint16_t *local_beacon_map,uint8_t *cluster_index,
								uint8_t cluster_number)
{
	uint16_t i = 0;
    uint16_t beacon_map = subordinate_beacon_map | *local_beacon_map;
    for(i=0; i<cluster_number; i++)
    {
        if(!(beacon_map &((uint16_t)0x01<<i)))
        {
            *local_beacon_map |= ((uint16_t)0x01<<i);
            break;
        }
    }
    if(i == cluster_number)
    {
		*cluster_index = 0xff;
        return FALSE;
    }
    else
    {
        *cluster_index = i;
		cluster_table[i-1].mac_addr = addr;
		cluster_table[i-1].cluster_index = i;
		cluster_table[i-1].life = CLUSTER_LIFE;
        return TRUE;
    }
}

static void assos_node_clear(uint16_t index)
{
	assoc_table[index].device_type = 0x0f;
	assoc_table[index].mac_addr = 0;
	assoc_table[index].life = 0;
	assoc_table[index].time_stamp = 0;
	assoc_table[index].new_time_stamp = 0;
}

void cluster_node_update(uint8_t cluster_index,uint16_t src_addr)
{
	uint8_t index = cluster_index - 1;
	uint16_t saddr = mac_short_addr_get(cluster_table[index].mac_addr);
	if(src_addr == saddr)
	{
		cluster_table[index].life = CLUSTER_LIFE;
	}
}

void assos_table_add(mac_assoc_req_arg_t *assoc_req, mac_assoc_state_e *state, 
					 uint8_t *cluster_index, uint16_t *local_beacon_map, uint64_t addr, uint8_t cluster_number)
{
	*cluster_index = 0;
	if(assoc_req->device_type == NODE_TYPE_ROUTER)
	{
		cluster_table_remove(addr, local_beacon_map, cluster_number);
		if(!cluster_table_add(addr, assoc_req->beacon_bitmap, local_beacon_map, cluster_index , cluster_number))
		{
			*state = ASSOC_STATUS_REFUSE;
			return;
		}
	}	
#if ASSOC_TABLE == 0
	*state = ASSOC_STATUS_SUCCESS;
	return;
#else
	uint16_t i=0;
	for(i=0; i<ASSOC_NUM; i++)
	{	
		if(assoc_table[i].mac_addr == addr)
		{
			assoc_table[i].device_type    = assoc_req->device_type;
            assoc_table[i].mac_addr  = addr;
			assoc_table[i].life = LIFE_CYCLE;
			*state = ASSOC_STATUS_SUCCESS;
			return;
		}
		else
		{//如果mac短地址相同拒绝关联
			if(mac_short_addr_get(assoc_table[i].mac_addr) == mac_short_addr_get(addr))
			{
				*state = ASSOC_STATUS_FULL;
				return;
			}
		}
	}
	for(i=0; i<ASSOC_NUM; i++)
	{
		if(assoc_table[i].device_type == 0x0f)
		{
			assoc_table[i].device_type    = assoc_req->device_type;
            assoc_table[i].mac_addr  = addr;
			assoc_table[i].life = LIFE_CYCLE;
			*state = ASSOC_STATUS_SUCCESS;
			return;
		}
	}
	*state = ASSOC_STATUS_FULL;
#endif	
}

void asyn_assos_table_add(assoc_info_t id, mac_assoc_state_e *state)
{
	uint16_t i=0;
	for(i=0;i<ASSOC_NUM;i++)
	{
		if(assoc_table[i].mac_addr == id.mac_addr)
		{
			assoc_table[i].device_type    = id.device_type;
            assoc_table[i].mac_addr  = id.mac_addr;
			assoc_table[i].life = LIFE_CYCLE;
			assoc_table[i].time_stamp = id.time_stamp;
			assoc_table[i].new_time_stamp = id.new_time_stamp;
			*state = ASSOC_STATUS_SUCCESS;
			return;
		}
		else
		{//如果mac短地址相同拒绝关联
			if(mac_short_addr_get(assoc_table[i].mac_addr) == mac_short_addr_get(id.mac_addr))
			{
				*state = ASSOC_STATUS_FULL;
				return;
			}
		}
	}
	for(i=0; i<ASSOC_NUM; i++)
	{
		if(assoc_table[i].device_type == 0x0f)
		{
			assoc_table[i].device_type    = id.device_type;
            assoc_table[i].mac_addr  = id.mac_addr;
			assoc_table[i].life = LIFE_CYCLE;
			assoc_table[i].time_stamp = id.time_stamp;
			assoc_table[i].new_time_stamp = id.new_time_stamp;
			*state = ASSOC_STATUS_SUCCESS;
			return;
		}
	}
	*state = ASSOC_STATUS_FULL;
}

assoc_info_t *asyn_assos_table_find(uint16_t mac_addr)
{
	for(uint16_t i=0;i<ASSOC_NUM;i++)
	{
		if(mac_short_addr_get(assoc_table[i].mac_addr) == mac_addr)
		{
			return &assoc_table[i];
		}
	}
	return NULL;
}

bool_t assos_sid_find(uint16_t mac_saddr)
{
	for(uint16_t i=0;i<ASSOC_NUM;i++)
	{
		if(mac_short_addr_get(assoc_table[i].mac_addr) == mac_saddr)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void asyn_time_update(uint32_t ticks)
{
	for(uint16_t i=0;i<ASSOC_NUM;i++)
	{
		if(assoc_table[i].device_type != 0x0f)
		{
			if(assoc_table[i].time_stamp < assoc_table[i].new_time_stamp)
			{
				if(assoc_table[i].new_time_stamp != 0)
				{
					assoc_table[i].time_stamp = assoc_table[i].new_time_stamp;
					assoc_table[i].new_time_stamp = 0;
					continue;
				}
			}
			assoc_table[i].new_time_stamp = 0;
			assoc_table[i].time_stamp += ticks; 
		}
	}
}

bool_t assoc_driver_find(uint64_t mac_addr)
{
	uint16_t i=0;
	for(i=0;i<ASSOC_NUM;i++)
	{
		if(assoc_table[i].mac_addr == mac_addr)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void assos_table_init(void)
{
	uint16_t i=0;
	for(i=0; i<ASSOC_NUM; i++)
	{
		assos_node_clear(i);
	}
}