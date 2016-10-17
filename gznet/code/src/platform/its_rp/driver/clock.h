/**
 * @brief       : provides an abstraction for clock.
 *
 * @file        : clock.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __CLOCK_H
#define __CLOCK_H

#include "common/lib/data_type_def.h"

#define SYSCLK_1MHZ 			0
#define SYSCLK_4MHZ 			1
#define SYSCLK_8MHZ 			2
#define SYSCLK_12MHZ			3
#define SYSCLK_16MHZ			4
#define SYSCLK_20MHZ			5
#define SYSCLK_25MHZ			6
#define DCO_MULT_1MHZ   		30
#define DCO_MULT_4MHZ   		122
#define DCO_MULT_8MHZ   		243
#define DCO_MULT_12MHZ  		366
#define DCO_MULT_16MHZ  		488
#define DCO_MULT_20MHZ  		610
#define DCO_MULT_25MHZ  		763

#define DCORSEL_1MHZ			DCORSEL_2
#define DCORSEL_4MHZ			DCORSEL_3
#define DCORSEL_8MHZ			DCORSEL_4
#define DCORSEL_12MHZ   		DCORSEL_5
#define DCORSEL_16MHZ   		DCORSEL_5
#define DCORSEL_20MHZ   		DCORSEL_6
#define DCORSEL_25MHZ   		DCORSEL_7

#define VCORE_1MHZ  			PMMCOREV_0
#define VCORE_4MHZ  			PMMCOREV_0
#define VCORE_8MHZ  			PMMCOREV_0
#define VCORE_12MHZ 			PMMCOREV_0
#define VCORE_16MHZ 			PMMCOREV_1
#define VCORE_20MHZ 			PMMCOREV_2
#define VCORE_25MHZ 			PMMCOREV_3

#define VCORE_1_35V 			PMMCOREV_0
#define VCORE_1_55V 			PMMCOREV_1
#define VCORE_1_75V 			PMMCOREV_2
#define VCORE_1_85V 			PMMCOREV_3

/**
 * start xt1 clock , set mcu frequency
 *
 * @param frequency mcu frequency
 */
void clk_init(uint8_t frequency);

void wdt_start(uint32_t time_ms);
void wdt_clear(uint32_t time_ms);
void wdt_stop(uint32_t time_ms);

void clk_xt1_open(void);
void clk_xt1_open_without_wait(void);
void clk_xt1_close(void);

void clk_xt2_open(void);
void clk_xt2_open_without_wait(void);
void clk_xt2_close(void);

#endif

/**
 * @}
 */

