#pragma once
#include <data_type_def.h>
#include "sync_define.h"


extern mac_head_t mac_head_info;
extern bool_t frm_pending;
extern uint8_t pending_me_seq_array;
extern slot_cfg_t beacon_interval;

typedef void (*frame_switch_t)(pbuf_t *const pbuf);
void sync_config(uint16_t target_id);
void super_frame_cfg(mac_pib_t *mac_pib, uint8_t type);

void mac_frames_cb_init(void);
void frame_switch_init(frame_switch_t mac_beacon,frame_switch_t mac_data,frame_switch_t mac_ack,
                       frame_switch_t mac_command);

bool_t beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t local_map);
bool_t self_coord_beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t coord_cluster_index);
bool_t self_beacon_send_enable(uint16_t seq, uint8_t gts_num, uint16_t self_cluster_index);

bool_t self_intra_gts_recv_enable(uint16_t seq , uint8_t gts_num, uint16_t self_cluster_index);
bool_t coord_intra_gts_enable(uint16_t seq , uint8_t gts_num, uint16_t coord_cluster_index);

bool_t self_coord_inter_gts_enable(uint16_t seq , mac_pib_t *mac_pib);
bool_t self_inter_gts_enable(uint16_t seq , mac_pib_t *mac_pib);

void intra_gts_range(mac_pib_t *mac_pib, uint16_t *start,uint16_t *end);
void inter_gts_range(mac_pib_t *mac_pib, uint16_t *start,uint16_t *end);


void mac_beacon_frame(pbuf_t *pbuf, sync_info_t *info);
