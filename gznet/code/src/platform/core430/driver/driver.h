/**
 * @brief       : 
 *
 * @file        : driver.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __DRIVER_H
#define __DRIVER_H
#define SMCLK           8000000

#define ACLK            32768

#define CPU_F           ((double)SMCLK)
#define delay_us(x)     __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x)     __delay_cycles((long)(CPU_F*(double)x/1000.0))

#include <msp430.h>
#include "common/lib/lib.h"
#include "board.h"
#include "flash.h"
#include "clock.h"
#include "uart.h"
#include "gpio.h"
#include "si4463_arch.h"
#include "rtimer_arch.h"
#include "energy.h"




extern uint8_t volatile sst_int_nest;
#endif

