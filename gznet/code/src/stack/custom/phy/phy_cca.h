/**
 * @brief       : Interface of Clear Channel Assessment and relative functions
 *
 * @file        : phy_cca.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __PHY_CCA_H__
#define __PHY_CCA_H__

#include "common/lib/lib.h"

/**
 * 检测信道是否空闲 clear channel assessment
 *
 * @param: 无
 *
 * @return: 如果信道空闲则返回TRUE，反之返回FALSE
 */
bool_t phy_cca(void);

/**
 * 停止CCA检测
 *
 * @param: 无
 *
 * @return: 始终返回TRUE
 */
bool_t phy_cca_stop(void);

/**
 * 使能芯片RSSI最大值获取功能
 *
 * @param: 无
 *
 * @return: 无
 */
void phy_get_rssi_largest_enable(void);

/**
 * 禁止芯片RSSI最大值获取功能
 *
 * @param: 无
 *
 * @return: 无
 */
void phy_get_rssi_largest_disable(void);

/**
 * 获取RSSI最大值
 *
 * @param: 无
 *
 * @return: RSSI值
 */
int8_t phy_get_rssi_largest(void);

/**
 * 获取一次RSSI值
 *
 * @param: 无
 *
 * @return: RSSI值
 */
int16_t phy_get_rssi(void);

/**
 * 多次获取RSSI值并返回平均值
 *
 * @param num: 获取RSSI值的次数
 *
 * @return: RSSI 平均值
 */
int8_t phy_rssi_average(uint8_t num);

#endif
