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
#define OVERFLOW_INT_EN         (0u)

#define MS_TO_TICK(time_ms)     ((uint32_t)((time_ms) * 32.768 + 0.5))
#define US_TO_TICK(time_us)     ((uint32_t)(((time_us) / 1000.0) * 32.768 + 0.5))
#define TICK_TO_US(tick_num)    ((uint32_t)((tick_num) * 30.518 + 0.5))

#if TIMER_DBG_EN > 0
#define DBG_TIMER(id)            DBG_FLOW_PRINT(DBG_LEVEL_TRACE, id)
#else
#define DBG_TIMER(id)
#endif

#if OVERFLOW_INT_EN == 0
#define TIMER_START()           st(TA0CTL = TASSEL_1 + MC_2 + TACLR;)
#else
#define TIMER_START()           st(TA0CTL = TASSEL_1 + MC_2 + TACLR + TAIE;)
#endif

#define HTIMER_HWCOUNT(x)                                           \
do                                                                  \
{                                                                   \
    uint16_t last_ta0r = 0;                                         \
    uint8_t i = 0;                                                  \
    bool_t flag = FALSE;                                            \
    while (!flag)                                                   \
    {                                                               \
        last_ta0r = TA0R;                                           \
        for (i=0; i<2; i++)                                         \
        {                                                           \
            if (last_ta0r != TA0R)                                  \
            {                                                       \
                break;                                              \
            }                                                       \
        }                                                           \
        if (i == 2)                                                 \
        {                                                           \
            flag = TRUE;                                            \
        }                                                           \
    }                                                               \
    x = last_ta0r;                                                  \
}  while(__LINE__ == -1)

#define HTIMER_SET_COMPREG(x)       st(TA0CCR4 = (x)->s.hw; HTIMER_COMPINT_ENABLE();)
#define HTIMER_GET_COMPREG()        TA0CCR4
#define HTIMER_COMPINT_ENABLE()     (TA0CCTL4 |= CCIE)
#define HTIMER_COMPINT_DISABLE()    (TA0CCTL4 &= ~CCIE)
#define HTIMER_CMP_FLG()            (TA0CCTL4 & CCIFG)
#define HTIMER_CLEAR_CMP_FLG()      (TA0CCTL4 &= ~CCIFG)
#if OVERFLOW_INT_EN == 0
#define HTIMER_OF_FLG()             (TA0CCTL4 & CCIFG)
#define HTIMER_CLEAR_OF_FLG()       (TA0CCTL4 &= ~CCIFG)
#else
#define HTIMER_OF_FLG()             (TA0CTL & TAIFG)
#define HTIMER_CLEAR_OF_FLG()       (TA0CTL &= ~TAIFG)
#endif

//htimer_at函数的执行时间的阈值13:定时本身操作需要13个tick
#define TIMER_SET_TH                0x0E
#define TIMER_SET_TH2               0x03    // 判断硬件时间是否即将翻转的阈值
#define TIMER_SET_TH3               0x07    // 允许返回定时(时间已过期)的阈值
#define TIMER_SET_TH4               0x00    // 硬件定时的设置点与当前点的最小间隔 ： 5 +det 2




#endif
