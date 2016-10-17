/**
 * @brief       : 
 *
 * @file        : hal_low_power.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __HAL_LOW_POWER_H
#define __HAL_LOW_POWER_H

#define HAL_CHIP_ENTER_CPU_IDLE         0u
#define HAL_CHIP_POWER_MODE_1           1u
#define HAL_CHIP_POWER_MODE_2           2u
#define HAL_CHIP_POWER_MODE_3           3u

void hal_chip_set_lpm(uint8_t state);


#endif

