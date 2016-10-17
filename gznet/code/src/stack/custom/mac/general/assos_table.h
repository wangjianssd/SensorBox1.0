#pragma once
#include "common/lib/lib.h"
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
	uint32_t time_stamp;
	uint32_t new_time_stamp;
    uint8_t device_type:4,
			life	:4;	
}assoc_info_t;


void assos_table_init(void);
assoc_info_t *asyn_assos_table_find(uint16_t mac_addr);
void asyn_time_update(uint32_t ticks);
void asyn_assos_table_add(assoc_info_t id, mac_assoc_state_e *state);
void assos_table_add(mac_assoc_req_arg_t *assoc_req, mac_assoc_state_e *state, 
					 uint8_t *cluster_index, uint16_t *local_beacon_map, uint64_t addr, uint8_t cluster_number);
void cluster_node_update(uint8_t cluster_index,uint16_t src_addr);

bool_t assos_table_remove(uint64_t mac_addr,mac_addr_mode_e mode);
bool_t assoc_driver_find(uint64_t mac_addr);
bool_t assos_sid_find(uint16_t mac_saddr);