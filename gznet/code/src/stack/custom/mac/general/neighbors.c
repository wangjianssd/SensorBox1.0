#include "neighbors.h"
#include "sys_arch/osel_arch.h"
//#define RSSI_QUERY_FAILED               (RSSI_RECV_FAILED)	// 问询应答可靠通信的下限

#define INVAILD_ID                      0xFFFF
#define INDEX                           0xFF
#define MAX_HOP_NUMBER                  0xFF
#define MAX_UNASSOC_LIFE                4u
//#define DEV_COUNT                       (5u)    // neighbors num

static neighbor_node_t neighbor_node_array[DEV_COUNT];
static neighbor_node_t coord;

static void neighbors_clear(neighbor_node_t *neighbor_device)
{
    neighbor_device->dev_id = INVAILD_ID;
    neighbor_device->rssi   = -127;
    neighbor_device->hops   = MAX_HOP_NUMBER;
    neighbor_device->time_stamp = 0x00000000;
    neighbor_device->coord_index = INDEX;
    neighbor_device->intra_channel = 0;
    neighbor_device->inter_channel = 0;
    neighbor_device->unassoc_life_cycle = 0;  // can be assoc
}

bool_t mac_neighbors_node_set_state(uint16_t id, bool_t assoc_state)
{
    uint8_t i = 0;
    for (i = 0; i < DEV_COUNT; i++)
    {
        if (neighbor_node_array[i].dev_id == id)
        {
            if (assoc_state == TRUE)
            {
                neighbor_node_array[i].unassoc_life_cycle = 0;
            }
            else
            {
                neighbor_node_array[i].unassoc_life_cycle = MAX_UNASSOC_LIFE;
            }
            return TRUE;
        }
    }
	
    return FALSE;
}

uint8_t mac_neighbors_count(void)
{
	uint8_t i = 0,num = 0;
    for (i = 0; i < DEV_COUNT; i++)
    {
        if(neighbor_node_array[i].dev_id != INVAILD_ID)
			num++;
    }
	return num;
}

void mac_neighbors_init(void)
{
	memset(&coord,0,sizeof(neighbor_node_t));
    uint8_t i = 0;
    for (i = 0; i < DEV_COUNT; i++)
    {
        neighbors_clear(&neighbor_node_array[i]);
    }
}

void asyn_update_cycle(uint32_t time)
{
	for(uint8_t i = 0; i < DEV_COUNT; i++)
	{
		neighbor_node_t *p = &neighbor_node_array[i];
		if(p->dev_id != INVAILD_ID)
		{
			p->time_stamp += time;
		}
	}
}

bool_t mac_neighbor_node_add(neighbor_node_t *neighbor_device)
{
    if(neighbor_device->rssi < RSSI_QUERY_FAILED)	//把不满足rssi的上级设备先剔除
    {
        return FALSE;
    }
    for(uint8_t i = 0; i < DEV_COUNT; i++)
    {
        if(neighbor_node_array[i].dev_id == neighbor_device->dev_id)
        {
            neighbor_node_array[i].hops = neighbor_device->hops;
            neighbor_node_array[i].rssi = neighbor_device->rssi;
            neighbor_node_array[i].time_stamp = neighbor_device->time_stamp;
            neighbor_node_array[i].coord_index = neighbor_device->coord_index;
            neighbor_node_array[i].intra_channel = neighbor_device->intra_channel;
            neighbor_node_array[i].inter_channel = neighbor_device->inter_channel;
            return TRUE;
        }
    }
    for(uint8_t i = 0; i < DEV_COUNT; i++)
    {
        if(neighbor_node_array[i].dev_id == INVAILD_ID)
        {
            neighbor_node_array[i].dev_id = neighbor_device->dev_id;
            neighbor_node_array[i].hops = neighbor_device->hops;
            neighbor_node_array[i].rssi = neighbor_device->rssi;
            neighbor_node_array[i].time_stamp = neighbor_device->time_stamp;
            neighbor_node_array[i].coord_index = neighbor_device->coord_index;
            neighbor_node_array[i].intra_channel = neighbor_device->intra_channel;
            neighbor_node_array[i].inter_channel = neighbor_device->inter_channel;
            neighbor_node_array[i].unassoc_life_cycle = 0; // can be assoc
            osel_memcpy(&neighbor_node_array[i].supf_cfg_arg, &neighbor_device->supf_cfg_arg
                        ,sizeof(supf_spec_t));
            return TRUE;
        }
    }
    return FALSE;
}

bool_t mac_get_coord(neighbor_node_t *node)
{
    uint8_t best_index = 0xff;
    uint8_t best_hops = MAX_HOP_NUMBER;
    int8_t best_rssi = -127;
    for(uint8_t i = 0; i < DEV_COUNT; i++)
    {
        neighbor_node_t *p = &neighbor_node_array[i];
        if(p->unassoc_life_cycle > 0)
        {
            if( (--p->unassoc_life_cycle) == 0 )
            {
                neighbors_clear(p);
            }
            continue;
        }
        if(( p->dev_id != INVAILD_ID) && (p->unassoc_life_cycle == 0))
        {
            if(p->hops < best_hops)
            {
                best_hops = p->hops;
				best_rssi = p->rssi;
                best_index = i;
                continue;
            }
            else if(p->hops == best_hops)
            {
                if(p->rssi > best_rssi)
                {
                    best_rssi = p->rssi;
                    best_hops = p->hops;
                    best_index = i;
                }
                continue;
            }
            else
            {
                continue;
            }
        }
    }
    if(best_index != 0xff)
    {
        osel_memcpy(node, &neighbor_node_array[best_index], sizeof(neighbor_node_t));
        return TRUE;
    }
    return FALSE;
}
