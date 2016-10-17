#pragma once
#include "sync_define.h"
void mac_beacon_package(sbuf_t *sbuf, sync_attribute_t *sync_attribute, mac_pib_t *mac_pib);

void gts_list_insert(uint8_t cluster_index, supf_spec_t *supf, sbuf_t *sbuf);
sbuf_t *gts_list_node_get(uint16_t index, up_down_link_t mode);
void gts_list_sort(up_down_link_t mode,uint16_t start,uint16_t end);
bool_t gts_list_empty(up_down_link_t mode);

void cap_list_insert(supf_spec_t *supf,sbuf_t *sbuf);
sbuf_t *cap_list_node_get(uint16_t index);
void cap_list_node_clear(void);

void sync_mac_package_init(void);
