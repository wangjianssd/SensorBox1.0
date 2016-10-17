/**
 * @brief       : this
 * @file        : clock.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-12-03
 * change logs  :
 * Date       Version     Author        Note
 * 2015-12-03  v0.0.1  gang.cheng    first version
 */
#ifndef __CLOCK_H__
#define __CLOCK_H__

void clk_init(uint8_t clock_speed);

void clk_xt2_open(void);

void clk_xt2_open_without_wait(void);

void clk_xt2_close(void);


#endif
