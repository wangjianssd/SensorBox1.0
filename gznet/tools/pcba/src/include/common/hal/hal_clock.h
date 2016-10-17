/**
 * provides an abstraction for nvmem.
 *
 * @file hal_clock.h
 * @author wanger
 *
 * @addtogroup HAL_CLOCK HAL system clock and power operation
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_CLOCK_H
#define __HAL_CLOCK_H

#include <data_type_def.h>
/**
 * start xt1 clock , set mcu frequency
 *
 * @param frequency mcu frequency
 */
void hal_clk_init(uint8_t frequency);
void hal_wdt_start(uint32_t time_ms);
void hal_wdt_clear(uint32_t time_ms);
void hal_wdt_stop(uint32_t time_ms);

void hal_clk_xt2_open(void);
void hal_clk_xt2_open_without_wait(void);
void hal_clk_xt2_close(void);

#endif

/**
 * @}
 */

