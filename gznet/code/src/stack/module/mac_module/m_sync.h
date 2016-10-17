/**
 * @brief       : MAC slot module for slot scheduling
 *
 * @file        : m_sync.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
/**
 * @addtogroup LIB_MACSLOT Link list operations
 * @ingroup LIB
 * @{
 */

#ifndef __M_SYNC_H
#define __M_SYNC_H

#include "common/lib/lib.h"
#include "hal_timer.h"
#include "stack/common/pbuf.h"

#define SFD_TIMESTAMP_SIZE			(4u)

typedef struct
{
	uint8_t flag_byte_pos;      // 同步帧标志在帧中的位置
	uint8_t flag_byte_msk;      // 同步帧标志的MASK
	uint8_t flag_byte_val;	    // 同步帧标志的值

	uint8_t stamp_byte_pos;     // 时戳字节所在的起始位置 值为0表示在帧的末尾
	uint8_t stamp_len;          // 时戳的长度

	uint8_t len_pos;            // 帧长度域所在的位置
	uint8_t len_modfiy;         // 帧长度域是否允许被修改

	uint8_t tx_sfd_cap;         // 芯片是否支持txsfd中断
	uint8_t rx_sfd_cap;	        // 芯片是否支持rxsfd中断
	int16_t tx_offset;          // 芯片不支持txsfd中断时准备发送到实际发送的偏移
	int16_t rx_offset;          // 芯片不支持rxsfd中断时准备解析到实际接收的偏移

	uint8_t sync_source;        // 是否是同步源
	uint16_t sync_target;       // 同步目标节点的ID
	uint8_t background_compute; // 空闲时刻同步运算还是实时运算   FALSE 表示非实时运算
} sync_cfg_t;

void m_sync_enable(bool_t enable);

/**
 * 配置同步模块信息；
 *
 * @param cfg: 配置信息的指针；
 */
void m_sync_cfg(const sync_cfg_t *const cfg);


/**
 * 同步模块初始化；
 */
void m_sync_init(osel_task_t *task);


/**
 * 将全局时间转换为本地时间
 *
 * @param gtime: 全局时间
 *
 */
void m_sync_g2l(hal_time_t *gtime);


/**
 * 将本地时间转换为全局时间
 *
 * @param ltime: 本地时间
 *
 */
void m_sync_l2g(hal_time_t *ltime);


/**
 * 对发送的帧进行过滤，如果是同步帧，则加上时间戳
 *
 * @param tpac: 同步帧
 *
 */
uint8_t m_sync_txfilter(pbuf_t *tpac);


/**
 * 对接收的帧进行过滤，如果是同步帧时，则获取同步时间戳
 *
 * @param tpac: 同步帧
 *
 */
void m_sync_rxfilter(pbuf_t *rpac);


#endif

/**@}*/

