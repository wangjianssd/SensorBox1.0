/**
 * @brief       : this
 * @file        : clock.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-15
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-15  v0.0.1  gang.cheng    first version
 */
#ifndef __CLOCK_H__
#define __CLOCK_H__


#define SYSCLK_1MHZ 			0
#define SYSCLK_4MHZ 			1
#define SYSCLK_8MHZ 			2
#define SYSCLK_12MHZ			3
#define SYSCLK_16MHZ			4
#define SYSCLK_20MHZ			5
#define SYSCLK_25MHZ			6


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
