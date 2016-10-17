
#include "wsnos.h"

#include "assos_table.h"


static cluster_info_t cluster_table[CLUSTER_NUMBER-1];
static assoc_info_t assoc_table[ASSOC_NUM];
const uint8_t LIFE_CYCLE = 3;
const uint8_t CLUSTER_LIFE = 7;
static void cluster_node_clear(uint16_t index)
{
	cluster_table[index].mac_addr = 0;
	cluster_table[index].life = 0;
	cluster_table[index].cluster_index = 0;
}

static void cluster_table_remove(uint64_t addr, uint16_t *local_beacon_map)
{
	uint16_t i = 0;
	for(i=0;i<CLUSTER_NUMBER;i++)
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
								uint16_t *local_beacon_map,uint8_t *cluster_index)
{
	uint16_t i = 0;
    uint16_t beacon_map = subordinate_beacon_map | *local_beacon_map;
    for(i=0; i<CLUSTER_NUMBER; i++)
    {
        if(!(beacon_map &((uint16_t)0x01<<i)))
        {
            *local_beacon_map |= ((uint16_t)0x01<<i);
            break;
        }
    }
    if(i == CLUSTER_NUMBER)
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
	assoc_table[index].device_type = UNDEFINE;
	assoc_table[index].mac_addr = 0;
	assoc_table[index].life = 0;
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
					 uint8_t *cluster_index, uint16_t *local_beacon_map, uint64_t addr)
{
	*cluster_index = 0;
	if(assoc_req->device_type == ROUTER)
	{
		cluster_table_remove(addr, local_beacon_map);
		if(!cluster_table_add(addr, assoc_req->beacon_bitmap, local_beacon_map, cluster_index))
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
		if(assoc_table[i].device_type == UNDEFINE)
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