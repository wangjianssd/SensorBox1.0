#pragma once
#include "common/lib/lib.h"
#include "stack/common/sbuf.h"
#include "../../general/general.h"

typedef void (*asyn_slot_cb)(void *seq_p);
PROCESS_NAME(asyn_process);
PROCESS_NAME(mac_event_process);
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
	uint8_t duration;
	uint8_t auery_duration;
	uint8_t auery_order;
	uint16_t asyn_cycle;
}asyn_cfg_t;

typedef struct
{	
	asyn_cfg_t	asyn_cfg_arg;
    uint8_t intra_channel;
    asyn_slot_cb time_slot[ASYN_SLOT_END];
	uint32_t 			recv_time;
} asyn_attribute_t;

typedef struct
{
    uint8_t       *drivce_type;
    mac_state_e       	*state;
	osel_task_t			*mac_task;
    asyn_attribute_t    *attribute;
	mac_pib_t			*mac_pib;
	volatile bool_t		*asyn_en;
} asyn_info_t;
#pragma pack()
