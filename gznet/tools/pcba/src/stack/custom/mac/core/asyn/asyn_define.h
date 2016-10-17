#pragma once
#include <data_type_def.h>
#include <sbuf.h>
#include "../../general/general.h"

#define BASE								(16u)
#define BASE_AUERY 							(10u)
#define DOCKING								(10u) 
#define ASYN_CYCLE							(BASE*100u)
typedef void (*asyn_slot_cb)(void *seq_p);

typedef enum
{
    ASYN_RECV,
    ASYN_SLEEP,
    ASYN_IDLE,
    ASYN_SLOT_END,
} asyn_slot_cb_e;

#pragma pack(1)
typedef struct
{
        uint8_t sleep_interval;                     /**< 睡眠间隔 */
}asyn_cfg_t;
typedef struct
{
    uint8_t mac_seq_num;
    uint8_t hops;
    int8_t down_rssi;                   //父节点下来的rssi值
    uint16_t coord_saddr;               //父节点短地址
    uint8_t intra_channel;
    uint64_t mac_addr;
    uint16_t self_saddr;
    asyn_cfg_t  asyn_cfg;
    asyn_slot_cb time_slot[ASYN_SLOT_END];
} asyn_pib_t;
typedef struct
{
    drivce_type_e       *drivce_type;
    mac_state_e       	*state;
    asyn_pib_t          *asyn_pib;
} asyn_info_t;
#pragma pack()
