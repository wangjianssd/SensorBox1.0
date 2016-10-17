 /**
 * @brief       : BH1750 中间层代码
 *
 * @file        : hal_bh1750.h
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/09/21
 *
 * Change Logs  : 
 *
 * Date        Version      Author      Notes
 * 2015/09/21  v0.0.1    cuihongpeng    first version
 */
#ifndef __HAL_BH1750_H
#define __HAL_BH1750_H
#include <gznet.h>
#include <bh1750.h>
#include <rfid.h>
//#include <data_type_def.h>     

typedef struct light_info_t_
{
    uint16_t current_light; // 当前结果
    uint16_t trigger_value; // 触发阀值
    uint16_t keep_value;    // 保持阀值
} light_info_t;

/**
 * @brief bh1750初始化
 */
bool_t hal_bh1750_init(void);

/**
 * @brief bh1750单次高精度读取
 * @return 光照强度
 */
float hal_bh1750_single_read(void);

/**
 * @brief bh1750连续高精度读取
 * @return 光照强度
 */
float hal_bh1750_continu_read(void);
/**
 * @brief bh1750写一个字节
 * @return 成功或失败
 */
bool_t hal_bh1750_write(uint8_t const value);

#endif
