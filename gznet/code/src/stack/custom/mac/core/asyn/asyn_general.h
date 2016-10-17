#pragma once
#include "common/lib/lib.h"
#include "asyn_define.h"
extern osel_etimer_t asyn_timer_ev;;
typedef void (*frame_switch_t)(pbuf_t *const pbuf);

void asyn_config(uint16_t target_id);

void asyn_cfg(asyn_attribute_t *asyn_attribute, uint8_t type);

void mac_frames_asyn_cb_init(void);

void asyn_frame_switch_init(frame_switch_t mac_beacon,frame_switch_t mac_data,frame_switch_t mac_ack,
                       frame_switch_t mac_command);

sbuf_t *asyn_send_list_node_get(void);

void asyn_send_list_insert(sbuf_t *sbuf);

bool_t asyn_send_list_empty(uint16_t *id);

void asyn_general_init(void);

void data_tx_done(sbuf_t *sbuf, bool_t result);
void auery_request_txok_cb(sbuf_t *sbuf, bool_t result);
void mac_sched_recv_timeout_set(void);
void mac_sched_sleep_timeout_set(void);
void send_idle(uint16_t *id);