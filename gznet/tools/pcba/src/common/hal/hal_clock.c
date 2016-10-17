/**
 * @brief       : 
 *
 * @file        : hal_clock.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <driver.h>

void hal_wdt_start(uint32_t time_ms)
{
    wdt_start(time_ms);
}

void hal_wdt_clear(uint32_t time_ms)
{
    wdt_clear(time_ms);
}

void hal_wdt_stop(uint32_t time_ms)
{
    wdt_stop(time_ms);
}

void hal_clk_init(uint8_t system_clock_speed)
{
    clk_init(system_clock_speed);
}

void hal_clk_xt2_open(void)
{
    clk_xt2_open();
}

void hal_clk_xt2_open_without_wait(void)
{
    clk_xt2_open_without_wait();
}

void hal_clk_xt2_close(void)
{
    clk_xt2_close();
}




