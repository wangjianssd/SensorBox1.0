#pragma once
#include <data_type_def.h>
#include "general.h"

typedef struct
{
	uint64_t mac_addr;
	uint8_t 
cluster_index	:4,
life			:4;
}cluster_info_t;

typedef struct
{
    uint64_t mac_addr;
    uint8_t device_type:4,
			life	:4;	
}assoc_info_t;


void assos_table_init(void);
void assos_table_add(mac_assoc_req_arg_t *assoc_req, mac_assoc_state_e *state, 
					 uint8_t *cluster_index, uint16_t *local_beacon_map, uint64_t addr);
void cluster_node_update(uint8_t cluster_index,uint16_t src_addr);

bool_t assos_table_remove(uint64_t mac_addr,mac_addr_mode_e mode);
bool_t assoc_driver_find(uint64_t mac_addr);