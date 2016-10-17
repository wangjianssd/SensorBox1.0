/**
 * @brief       :
 *
 * @file        : mac_neighbors.h
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/8/12
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/8/12    v0.0.1      shenghao.xu    some notes
 */
#pragma once
#include <data_type_def.h>
#include "../core/sync/sync_define.h"
typedef struct
{
    uint16_t    dev_id;
    int8_t      rssi;
    uint8_t     hops;
    uint32_t    time_stamp;
    uint8_t     coord_index;
    uint8_t     intra_channel;
    uint8_t     inter_channel;
    uint8_t     unassoc_life_cycle;
    supf_spec_t supf_cfg_arg;
} neighbor_node_t;

/*邻居表的初始化，ID为无效值，生存周期为最大值*/
void mac_neighbors_init(void);

/*邻居节点插入、更新*/
bool_t mac_neighbor_node_add(neighbor_node_t *neighbor_device);

/**
 * [mac_get_coord : the func of get coord]
 * @param
 * @return
 */
bool_t mac_get_coord(neighbor_node_t *node);

bool_t mac_neighbors_node_set_state(uint16_t id, bool_t assoc_state);
/**
* @brief 获取邻居个数
* @param[in] 
* @return	
*/
uint8_t mac_neighbors_count(void);
