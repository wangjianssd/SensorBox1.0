/**
 * @brief       : 
 *
 * @file        : hal.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __HAL_H
#define __HAL_H

#include "hal_board.h"

typedef uint8_t hal_int_state_t;

#define HAL_ENTER_CRITICAL(x)                   OSEL_ENTER_CRITICAL(x)

#define HAL_EXIT_CRITICAL(x)                    OSEL_EXIT_CRITICAL(x)


#include "common/hal/hal_clock.h"
#include "common/hal/hal_uart.h"
#include "common/hal/hal_nvmem.h"
#include "common/hal/hal_low_power.h"
#include "common/hal/hal_gpio.h"
#include "common/hal/hal_nvmem.h"
#include "common/hal/hal_sensor.h"
#include "common/hal/hal_energy.h"


#include "common/lib/debug.h"

#endif
