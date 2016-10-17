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

#include <node_cfg.h>

#define SMCLK           8000000

#define ACLK            32768

#define CPU_F           ((double)SMCLK)
#define delay_us(x)     __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x)     __delay_cycles((long)(CPU_F*(double)x/1000.0))

#include <msp430.h>
#include <board.h>
#include <clock.h>
#include <gpio.h>
#include <rf.h>
#include <timer.h>
#include <uart.h>
#include <sensor.h>
#include <socket.h>
#include <debug.h>
#include <energy.h>
#include <flash.h>

#if (NODE_TYPE == NODE_TYPE_HOST)
#include <sd.h>
#endif

#if (NODE_TYPE == NODE_TYPE_SMALL_HOST)
#include <sd.h>
#include <max823r.h>
#include <ad5252.h>
#endif


extern uint8_t volatile sst_int_nest;
#endif

