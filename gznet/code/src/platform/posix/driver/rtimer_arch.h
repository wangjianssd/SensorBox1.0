/**
 * @brief       : this
 * @file        : rtimer_arch.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-11
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-11  v0.0.1  gang.cheng    first version
 */
#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#define TIMER_DBG_EN            (0u)
#define OVERFLOW_INT_EN         (0u)
 
#define MS_TO_TICK(time_ms)     ((uint32_t)((time_ms) * 32.768 + 0.5))
#define US_TO_TICK(time_us)     ((uint32_t)(((time_us) / 1000.0) * 32.768 + 0.5))
#define TICK_TO_US(tick_num)    ((uint32_t)((tick_num) * 30.518 + 0.5))


#if TIMER_DBG_EN > 0
#define DBG_TIMER(id)            DBG_LOG(DBG_LEVEL_TRACE, "%d\r\n", id)
#else
#define DBG_TIMER(id)
#endif

#define TIMER_START()               rtimer_start()

#define HTIMER_HWCOUNT(x)           hardware_cnt 

#define HTIMER_SET_COMPREG(x)       do{     \
    compare_cnt = (x)->s.hw;                \
}while(__LINE__ == -1)

#define HTIMER_GET_COMPREG()        compare_cnt
#define HTIMER_COMPINT_ENABLE()     
#define HTIMER_COMPINT_DISABLE()    
#define HTIMER_CMP_FLG()            compare_flag
#define HTIMER_CLEAR_CMP_FLG()      compare_flag = 0

#if OVERFLOW_INT_EN == 0
#define HTIMER_OF_FLG()             compare_flag
#define HTIMER_CLEAR_OF_FLG()       compare_flag = 0
#else
#define HTIMER_OF_FLG()             
#define HTIMER_CLEAR_OF_FLG()       
#endif

#define TIMER_SET_TH                0x0E
#define TIMER_SET_TH2               0x03    // 判断硬件时间是否即将翻转的阈值
#define TIMER_SET_TH3               0x07    // 允许返回定时(时间已过期)的阈值
#define TIMER_SET_TH4               0x00    // 硬件定时的设置点与当前点的最小间隔 ： 5 +det 2


extern uint16_t hardware_cnt;
extern uint16_t compare_cnt;
extern bool_t compare_flag;

void rtimer_start(void);

#endif
