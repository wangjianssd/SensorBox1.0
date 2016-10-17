/**
 * @brief       : this
 * @file        : wdt.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-12-03
 * change logs  :
 * Date       Version     Author        Note
 * 2015-12-03  v0.0.1  gang.cheng    first version
 */
#ifndef __WDT_H__
#define __WDT_H__

void wdt_start(uint32_t time);


void wdt_clear(uint32_t time);


void wdt_stop(uint32_t time);


#endif
