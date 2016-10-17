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

#include <hal_board.h>

typedef uint8_t hal_int_state_t;

#if (((__TID__ >> 8) & 0x7F) == 0x2b)     /* 0x2b = 43 dec */
#define HAL_ENTER_CRITICAL(x)                   \
do                                              \
{                                               \
    if (__get_SR_register()&GIE)                \
    {                                           \
        x = 1;                                  \
        _DINT();                                \
    }                                           \
    else                                        \
    {                                           \
        x = 0;                                  \
    }                                           \
} while(__LINE__ == -1)

#define HAL_EXIT_CRITICAL(x)    st(if ((x) == 1) _EINT();)

#else
#define HAL_ENTER_CRITICAL(x)                   \
do                                              \
{                                               \
    x = 0;                                      \
} while(__LINE__ == -1)

#define HAL_EXIT_CRITICAL(x)    x=x
#endif

#include <hal_clock.h>
#include <hal_timer.h>
#include <hal_uart.h>
#include <hal_nvmem.h>
#include <hal_low_power.h>
#include <hal_gpio.h>
#include <hal_nvmem.h>
#include <hal_sensor.h>
#include <hal_energy.h>

#if (NODE_TYPE == NODE_TYPE_DETECTOR || NODE_TYPE == NODE_TYPE_ROUTER)
//#include <hal_black_list.h>
#endif

#if ((NODE_TYPE == NODE_TYPE_HOST) || (NODE_TYPE == NODE_TYPE_SMALL_HOST))
// #include <hal_sd.h>
#include <hal_rtc.h>
#endif

#include <debug.h>

#endif
