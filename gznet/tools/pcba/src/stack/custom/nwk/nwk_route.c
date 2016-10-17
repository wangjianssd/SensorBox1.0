/**
 * @brief       : 
 *
 * @file        : nwk_route.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/14
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/14    v0.0.1      gang.cheng    first version
 */
#include "lib.h"
#include "osel_arch.h"
#include "nwk_route.h"

typedef uint64_t hash_key_t;

typedef struct hash_table {
    uint16_t size;
    uint16_t nr;
    route_entry_t *array;
} hash_table_t;

static int32_t route_node_live_cnt = 0;
static hash_table_t route_table;
static route_entry_t route_table_entry[ROUTE_NODE_MAX];

int8_t nwk_route_module_init(int32_t live_cnt)
{
    route_node_live_cnt = live_cnt;
    
    route_table.size = ROUTE_NODE_MAX;
    route_table.nr = 0;
    
    route_table.array = route_table_entry;
    
    return 0;
}

int8_t nwk_route_module_deinit(void)
{
    route_table.size = 0;
    route_table.nr = 0;
    
    route_table.array = NULL;
    
    return 0;
}

void nwk_route_module_update(void)
{
    route_entry_t *route_entry = route_table.array;
    for(uint16_t i=0;i<route_table.size;i++)
    {
        if(route_entry->live_time != 0)
        {
            if(--route_entry->live_time == 0)
            {
                osel_memset(&route_entry[0], 0x00, sizeof(route_entry_t));
                route_table.nr--;
            }
            route_entry++;
        }
    }
}
/**
 * @brief 找到hash表里面的元素，如果有对应的元素返回元素的地址，如果没有对应的元素，
 *        但是有空闲的地址也返回，只有在hash表满的情况下返回NULL
 * 
 * @param[in] key 要查找元素的键值
 * @param[in] table 对应hash表的地址
 *
 * @return 找到hash表对应键值的元素
 */
static route_entry_t *lookup_hash_entry(hash_key_t key, const hash_table_t *table)
{
    uint16_t size = table->size;
    uint16_t offset = key % size;
    route_entry_t *array = table->array;
    
    for(uint16_t i = 0; i < ROUTE_NODE_MAX; i++) {
        if(array[offset].live_time != 0 ) {
            if(array[offset].nui == key)
                goto find_entry;
            offset++;
            if(offset > size)
                offset = 0;
        }
        else
        {
            goto find_entry;
        }
    }
    
    return NULL;
    
find_entry:
    return &array[offset];
}
     
bool_t nwk_route_module_insert(route_entry_t *entry)
{
    if(entry == NULL) {
        return FALSE;
    }
    
    if(route_table.nr == ROUTE_NODE_MAX)
    {
        return FALSE;
    }
    
    entry->live_time = route_node_live_cnt;
    
    route_entry_t *route_node = lookup_hash_entry(entry->nui, &route_table);
    if(route_node == NULL)
    {
        return FALSE;
    }
    
    osel_memcpy(route_node, entry, sizeof(route_entry_t));
    route_table.nr++;
    return TRUE;
}

bool_t nwk_route_module_refresh(uint16_t nwk_addr)
{
    for(uint16_t i = 0; i< ROUTE_NODE_MAX; i++)
    {
        if(route_table.array[i].live_time != 0)
        {
            if(route_table.array[i].nwk_addr == nwk_addr)
            {
                route_table.array[i].live_time = route_node_live_cnt;
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

route_entry_t *nwk_route_module_nexthop(uint16_t nwk_addr)
{
    for(uint16_t i = 0; i< ROUTE_NODE_MAX; i++)
    {
        if(route_table.array[i].live_time != 0)
        {
            if(route_table.array[i].nwk_addr == nwk_addr)
            {
                return (&route_table.array[i]);
            }
        }
    }
    
    return NULL;
}

bool_t nwk_route_moduel_fresh(route_entry_t *entry)
{
    entry->live_time = route_node_live_cnt;
    return TRUE;
}


route_entry_t * nwk_route_modele_get(uint64_t key)
{   
    for(uint16_t i = 0; i< ROUTE_NODE_MAX; i++)
    {
        if(route_table.array[i].nui == key)
        {
            return &route_table.array[i];
        }
    }
    
    return NULL;
}










