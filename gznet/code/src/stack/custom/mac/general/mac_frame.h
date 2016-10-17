#pragma once
#include "common/lib/lib.h"
#include "general.h"
#include "../core/sync/sync_define.h"

/**
* @brief 关联请求解析
* @param[in] 数据载荷
* @return	NULL
*/
void assoc_request_frame(pbuf_t *pbuf, mac_assoc_req_arg_t *assoc_req);

/**
* @brief 关联应答解析
* @param[in] 数据载荷
* @param[in] 属性
* @return	关联成功状态
*/
bool_t assoc_response_frame(pbuf_t *pbuf, mac_pib_t *mac_pib,sync_attribute_t *sync_attribute);

/**
* @brief 南向sbuf属性填充
* @param[in] 数据载荷
* @return	NULL
*/
void south_sbuf_fill(sbuf_t *sbuf, uint16_t saddr);