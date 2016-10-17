#include "osel_arch.h"

#include "asyn_general.h"
#include <mac_module.h>
#include <hal.h>
typedef void (*func_t)(void *args);
static slot_cfg_t asyn_interval;
static slot_cfg_t recv_slot;
static slot_cfg_t sleep_slot;

static void slot_node_cfg(slot_cfg_t *node,
                          uint32_t duration,
                          uint8_t repeat_cnt,
                          func_t func,
                          slot_cfg_t *parent,
                          slot_cfg_t *first_child,
                          slot_cfg_t *next_sibling)
{
    node->slot_duration = duration;
    node->slot_repeat_cnt = repeat_cnt;
    node->func = func;
    node->parent = parent;
    node->first_child = first_child;
    node->next_sibling = next_sibling;
    node->slot_start = 0;
    node->slot_repeat_seq = 0;
}

void asyn_config(uint16_t target_id)
{
    sync_cfg_t cfg;
    cfg.background_compute = FALSE;
    cfg.sync_source        = FALSE;
    cfg.sync_target        = target_id;
    cfg.flag_byte_pos      = 0x03;
    cfg.flag_byte_msk      = 0x07;
    cfg.flag_byte_val      = 0;
    cfg.len_pos            = 0;
    cfg.len_modfiy         = TRUE;
    cfg.stamp_len          = 4;
    cfg.stamp_byte_pos     = 0;
    cfg.tx_sfd_cap         = FALSE;
    cfg.rx_sfd_cap         = TRUE;
    cfg.tx_offset          = 67;
    cfg.rx_offset          = 0;
    m_sync_cfg(&cfg);
}

void asyn_cfg(asyn_pib_t *asyn_pib, uint8_t type)
{
    uint32_t recv_duration = MS_TO_TICK(BASE);
    uint32_t sleep_duration = MS_TO_TICK(ASYN_CYCLE-BASE);
    slot_node_cfg(&recv_slot, recv_duration, 1, asyn_pib->time_slot[ASYN_RECV],
                  &asyn_interval, NULL, &sleep_slot);
    slot_node_cfg(&sleep_slot, sleep_duration, 1, asyn_pib->time_slot[ASYN_SLEEP],
                  &asyn_interval, NULL, NULL);
    slot_node_cfg(&asyn_interval, 0, 0, asyn_pib->time_slot[ASYN_IDLE],
                  NULL, &recv_slot, NULL);
    m_slot_cfg(&asyn_interval, type);
}
