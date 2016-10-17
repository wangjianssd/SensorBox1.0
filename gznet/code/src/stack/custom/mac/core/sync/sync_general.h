#pragma once
#include "common/lib/data_type_def.h"
#include "sync_define.h"

extern bool_t frm_pending;
extern uint8_t pending_me_seq_array;
extern slot_cfg_t beacon_interval;

typedef void (*frame_switch_t)(pbuf_t *const pbuf);
void sync_config(uint16_t target_id);
void super_frame_cfg(sync_attribute_t *sync_attribute, uint8_t type);

void mac_frames_sync_cb_init(void);
void frame_switch_init(frame_switch_t mac_beacon,frame_switch_t mac_data,frame_switch_t mac_ack,
                       frame_switch_t mac_command);

bool_t beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t local_map);
bool_t self_coord_beacon_recv_enable(uint16_t seq, uint8_t gts_num, uint16_t coord_cluster_index);
bool_t self_beacon_send_enable(uint16_t seq, uint8_t gts_num, uint16_t self_cluster_index);

bool_t self_intra_gts_recv_enable(uint16_t seq , uint8_t gts_num, uint16_t self_cluster_index);
bool_t coord_intra_gts_enable(uint16_t seq , uint8_t gts_num, uint16_t coord_cluster_index);

bool_t self_coord_inter_gts_enable(uint16_t seq , sync_attribute_t *sync_attribute, uint8_t hops);
bool_t self_inter_gts_enable(uint16_t seq , sync_attribute_t *sync_attribute, uint8_t hops);

void intra_gts_range(sync_attribute_t *sync_attribute, uint16_t *start,uint16_t *end);
void inter_gts_range(sync_attribute_t *sync_attribute, uint16_t *start,uint16_t *end, uint8_t hops);


void mac_beacon_frame(pbuf_t *pbuf, sync_info_t *info);
