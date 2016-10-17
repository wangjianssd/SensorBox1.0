/**
 * @brief       : 
 *
 * @file        : mac_module.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __MAC_MODULE_H
#define __MAC_MODULE_H

#include <m_tran.h>
#include <m_slot.h>
#include <m_sync.h>
#include <osel_arch.h>

#define M_TRAN_EN                           (1u)
#define M_SLOT_EN                           (m_slot_en)
#define M_SYNC_EN                           (1u)

#define SYN_CODE_COMPRESS_EN                (0u) // 压缩SYNC代码
#define TXOK_INT_SIMU_EN                    (0u) // 模拟TXOK中断

#endif


