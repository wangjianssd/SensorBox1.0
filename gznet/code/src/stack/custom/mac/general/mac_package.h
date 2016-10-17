#pragma once
#include "common/lib/lib.h"
#include "general.h"
#include "../core/sync/sync_define.h"
#include "../core/asyn/asyn_define.h"
typedef struct
{
    uint8_t       *drivce_type;
	net_mode_e          *mode;
	mac_pib_t       	*mac_pib;
	sync_attribute_t	*sync_attribute;
} mac_package_t;

void mac_package_init(mac_package_t *mac_package);

/**
* @brief 组ACK帧的mac头
* @param 数据载荷
* @return NULL
*
*/
void mac_ack_fill_package(pbuf_t *pbuf, uint8_t seqno, uint16_t des_addr);

/**
* @brief 组数据帧的mac头
* @param 数据载荷
* @return NULL
*
*/
void mac_data_fill_package(sbuf_t *sbuf);

/**
* @brief 组关联请求帧
* @param[in] NULL
* @return	sbuf
*/
sbuf_t *mac_assoc_request_package(void);

/**
* @brief 组关联应答帧
* @param[in] 关联应答状态
* @param[in] 收到的cap时隙号
* @param[in] 目的地址
* @return	sbuf
*/
sbuf_t *mac_assoc_respond_package(mac_assoc_state_e state, uint16_t slot_seq,
								  uint64_t des_addr, uint8_t cluster_index);

/**
* @brief 组问询帧
* @param[in] 目的地址
* @return	sbuf
*/
sbuf_t *mac_auery_package(uint16_t des_addr, bool_t frm_pending, uint32_t next_recv_time);

/**
* @brief 组问询应答帧
* @param[in] 目的地址
* @return	sbuf
*/
sbuf_t *mac_auery_request_package(uint16_t des_addr, bool_t pending, uint32_t next_recv_time);