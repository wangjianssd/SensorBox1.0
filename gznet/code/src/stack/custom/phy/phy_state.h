/**
 * @brief       : Configuration and interface of PHY state control
 *
 * @file        : phy_state.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __PHY_STATE_H__
#define __PHY_STATE_H__

#include "common/lib/lib.h"

enum
{
    PHY_SLEEP_STATE = 0x01,
    PHY_IDLE_STATE,
    PHY_TX_STATE,
    PHY_RX_STATE,
    PHY_INVALID_STATE,
};

enum _PHY_CHANNEL_INDEX
{
    PHY_CHANNEL_1 = 11,
    PHY_CHANNEL_2,
    PHY_CHANNEL_3,
    PHY_CHANNEL_4,
    PHY_CHANNEL_5,
    PHY_CHANNEL_6,
    PHY_CHANNEL_7,
    PHY_CHANNEL_8,
    PHY_CHANNEL_9,
    PHY_CHANNEL_10,
    PHY_CHANNEL_11,
    PHY_CHANNEL_12,
    PHY_CHANNEL_13,
    PHY_CHANNEL_14,
    PHY_CHANNEL_15,
    PHY_CHANNEL_16,
};

/**
 * 获取RF的当前状态
 *
 * @param: 无
 *
 * @return: 当前RF的状态
 */
uint8_t phy_get_state(void);

/**
 * 设置RF的状态
 *
 * @param: RF的状态
 *
 * @return: 无
 */
bool_t phy_set_state(uint8_t rf_state);

/**
 * 获取当前的工作信道
 *
 * @param: 无
 *
 * @return: 当前信道值
 */
uint8_t phy_get_channel(void);

/**
 * 设置射频芯片的工作信道
 *
 * @param channel: 需要设定的信道值
 *
 * @return: 始终返回TRUE
 */
bool_t phy_set_channel(uint8_t channel_index);

/**
 * 获取射频芯片当前的功率
 *
 * @param: 无
 *
 * @return: 功率值的索引
 */
uint8_t phy_get_power(void);

/**
 * 设置射频芯片发射功率
 *
 * @param: 功率值的索引
 *
 * @return: 始終返回TRUE
 */
bool_t phy_set_power(uint8_t power_index);

#endif
