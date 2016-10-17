/**
 * @brief       : this
 * @file        : rtimer_arch.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-12-07
 * change logs  :
 * Date       Version     Author        Note
 * 2015-12-07  v0.0.1  gang.cheng    first version
 */
#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#define TIMER_DBG_EN            (0u)
#define OVERFLOW_INT_EN			(0u)
 
#define MS_TO_TICK(time_ms)     ((uint32_t)((time_ms) * 32.768 + 0.5))
#define US_TO_TICK(time_us)     ((uint32_t)(((time_us) / 1000.0) * 32.768 + 0.5))
#define TICK_TO_US(tick_num)    ((uint32_t)((tick_num) * 30.518 + 0.5))


#if TIMER_DBG_EN > 0
#define DBG_TIMER(id)            DBG_LOG(DBG_LEVEL_TRACE, "%d\r\n", id)
#else
#define DBG_TIMER(id)
#endif

#define TIMER_START()               rtimer_start()

#define HTIMER_HWCOUNT(x)           (x = __HAL_TIM_GET_COUNTER(&rtim))

#define HTIMER_SET_COMPREG(x)       do{                         \
    __HAL_TIM_SET_COMPARE(&rtim, TIM_CHANNEL_1, (x)->s.hw);     \
    HTIMER_COMPINT_ENABLE();                                    \
}while(__LINE__ == -1)

#define HTIMER_GET_COMPREG()        __HAL_TIM_GET_COMPARE(&rtim, TIM_CHANNEL_1)
#define HTIMER_COMPINT_ENABLE()     __HAL_TIM_ENABLE_IT(&rtim, TIM_IT_CC1)
#define HTIMER_COMPINT_DISABLE()    __HAL_TIM_DISABLE_IT(&rtim, TIM_IT_CC1)
#define HTIMER_CMP_FLG()            __HAL_TIM_GET_FLAG(&rtim, TIM_IT_CC1)
#define HTIMER_CLEAR_CMP_FLG()      __HAL_TIM_CLEAR_FLAG(&rtim, TIM_IT_CC1);

#if OVERFLOW_INT_EN == 0
#define HTIMER_OF_FLG()             __HAL_TIM_GET_FLAG(&rtim, TIM_IT_CC1)
#define HTIMER_CLEAR_OF_FLG()       __HAL_TIM_CLEAR_FLAG(&rtim, TIM_IT_CC1);
#else
#define HTIMER_OF_FLG()             __HAL_TIM_GET_FLAG(&rtim, TIM_IT_UPDATE)
#define HTIMER_CLEAR_OF_FLG()       __HAL_TIM_CLEAR_FLAG(&rtim, TIM_IT_UPDATE);
#endif

#define TIMER_SET_TH                0x0E
#define TIMER_SET_TH2               0x03    // 判断硬件时间是否即将翻转的阈值
#define TIMER_SET_TH3               0x07    // 允许返回定时(时间已过期)的阈值
#define TIMER_SET_TH4               0x00    // 硬件定时的设置点与当前点的最小间隔 ： 5 +det 2

extern TIM_HandleTypeDef  rtim;

void rtimer_start(void);

#endif
