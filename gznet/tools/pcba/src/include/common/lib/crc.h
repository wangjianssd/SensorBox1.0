/**
 * @brief       : 
 *
 * @file        : crc.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __CRC8_H
#define __CRC8_H

#include <data_type_def.h>

/* 用户定义CRC类型 */
typedef uint8_t crc_t;


/**
 * 计算并返回指定数据区域crc的值
 *
 * @param uc_ptr:  待计算的数据区首地址
 * @param uc_len:  待计算的数据区长度
 *
 * @return crc计算的结果
 */
crc_t crc_compute(const uint8_t *const uc_ptr, uint16_t uc_len);

#endif




