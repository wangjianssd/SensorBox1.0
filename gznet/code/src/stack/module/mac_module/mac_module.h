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

#include "hal_timer.h"
#include "m_tran.h"
#include "m_slot.h"
#include "m_sync.h"

#include "sys_arch/osel_arch.h"

void m_sync_en(bool_t state);
bool_t m_sync_get(void);
#define M_TRAN_EN                           (1u)
//#define M_SLOT_EN                           (1u)
//#define M_SYNC_EN                           (1u)

#define SYN_CODE_COMPRESS_EN                (0u) // 压缩SYNC代码
#define TXOK_INT_SIMU_EN                    (0u) // 模拟TXOK中断

#endif


