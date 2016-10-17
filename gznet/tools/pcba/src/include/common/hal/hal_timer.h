/**
 * @brief       : 在一个定时器上面申请多个软件定时
 *
 * @file        : hal_timer.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */


#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

#include <lib.h>


typedef enum _hard_timer_state_t {
    TIMER_EXPECT_OFINT = 0,
    TIMER_EXPECT_CMPINT = 1,
} hard_timer_state_t;

typedef enum {
    TIMER_SUCCESS,
    TIMER_ERROR_INVALID_INPUT,
    TIMER_ERROR_FULL,
} hal_timer_err_t;

typedef uint16_t hal_timer_sw_t;
typedef uint16_t hal_timer_hw_t;
typedef struct spliced_time {
    hal_timer_hw_t hw;
    hal_timer_sw_t sw;
} spliced_time_t;

typedef union _hal_time_union {
    spliced_time_t s;
    uint32_t w;
} hal_time_t;

typedef void (*hal_timer_cb_t)(void *arg);

typedef struct htimer {
    list_head_t list;
    hal_time_t ftime;
    hal_timer_cb_t func;
    void *arg;
} hal_timer_t;

typedef struct htimer_controler {
    hal_time_t (*now)(void);
    hal_timer_t *(*set)(hal_time_t ftime,
                        hal_timer_cb_t func,
                        void *arg,
                        hal_timer_err_t *err);
    bool_t (*cancel)(hal_timer_t **tidd);
} htimer_controler_t;

extern htimer_controler_t htimer;

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

#if (((__TID__ >> 8) & 0x7F) == 0x2b)     /* 0x2b = 43 dec */

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

#else //for cc2530

extern uint8_t st2_temp;
#define TIMER_START()           st(STIE = 1; STIF = 0; T1CTL |= (3 << 2) | (1 << 0); T1CNTL = 0x00;)

#define HTIMER_HWCOUNT(x)                                   \
do                                                          \
{                                                           \
    x = 0x0u;                                               \
    x |= ST0;                                               \
    x |= (uint16_t)ST1 <<  8;                               \
    st2_temp = ST2;                                         \
} while(__LINE__ == -1);


extern hal_time_t time_now;
extern bool_t fisrt_set_cmp;
#define HTIMER_SET_COMPREG(x)                                       \
do                                                                  \
{                                                                   \
    time_now = hal_timer_now();                                     \
    DBG_ASSERT((x)->s.sw - time_now.s.sw < HTIMER_HALF_TURNOVER __DBG_LINE);  \
    if((x)->w  < TIMER_SET_TH4 + time_now.w )                       \
    {                                                               \
        (x)->w = time_now.w + TIMER_SET_TH4;                        \
        DBG_TIMER(0x2A);                                            \
    }                                                               \
    while(!(STLOAD& 0x01));                                         \
    ST2 = (uint8_t)((x)->s.sw & 0xFF);                              \
    ST1 = (uint8_t)((x)->s.hw >> 8);                                \
    ST0 = (uint8_t)((x)->s.hw);                                     \
    while(!(STLOAD& 0x01));                                         \
    HTIMER_COMPINT_ENABLE();                                        \
} while(__LINE__ == -1);

#define HTIMER_GET_COMPREG()
#define HTIMER_COMPINT_ENABLE()     (STIE = 1)
#define HTIMER_COMPINT_DISABLE()    (STIE = 0)
#define HTIMER_CMP_FLG()            (STIF)
#define HTIMER_OF_FLG()             (STIF)
#define HTIMER_CLEAR_OF_FLG()       st(STIF = 0;)
#define HTIMER_CLEAR_CMP_FLG()      st(STIF = 0;)

//htimer_at函数的执行时间的阈值22:定时本身操作最多需要21个tick
#define TIMER_SET_TH                0x16
#define TIMER_SET_TH2               0x09        // 判断硬件时间是否即将翻转的阈值
#define TIMER_SET_TH3               0x0E        // 允许返回定时(时间已过期)的阈值
#define TIMER_SET_TH4               0x07        // 硬件定时的设置点与当前点的最小间隔 ： 5 +det 2

#endif

#define hwtime_cmp(x,op,y)          ((*(x))op(*(y)))
#define hwtime_add(x,y)             st(*x += *y;)
#define hwtime_min(x,y)             st(*x -= *y;)
#define hwtime_cmp_num(x,op,y)      ((*(x))op(y))
#define hwtime_add_num(x,y)         st(*x += y;)
#define hwtime_min_num(x,y)         st(*x -= y;)
#define hwtime_set(x,y)             st(x = y;)

#define HTIMER_HALF_TURNOVER        0x8000


/**
 * 定时器控制器初始化函数
 *
 * @htimer_controler 指向控制器的指针
 *
 * @return 无
 */
void hal_timer_controler_init(htimer_controler_t *htimer_controler);

/**
 * 定时器模块初始化函数
 *
 */
void hal_timer_init(void);

/**
 * 绝对时间定时函数
 *
 * @param ftime  到期的时间
 * @param func   定时器到期触发的回调
 * @param *arg   回调函数的参数
 * @param *err   错误指示代码
 *
 * @return 定时器句柄
 */
hal_timer_t *hal_timer_set(hal_time_t ftime,
                           hal_timer_cb_t func,
                           void *arg,
                           hal_timer_err_t *err);



/**
 * 相对时间定时函数
 *
 * @param expires   从执行该函数起的相对时间
 * @param func      定时器到期触发的回调
 * @param *arg      回调函数的参数
 * @param *timer    定时器句柄
 *
 * @return 定时器句柄
 */
#define HAL_TIMER_SET_REL(expires, func, arg, timer)                 \
do                                                                   \
{                                                                    \
    if(NULL!=timer)                                                  \
    {                                                                \
        hal_timer_cancel(&timer);                                    \
    }                                                                \
    hal_time_t now = hal_timer_now();                                \
    now.w += (expires);                                              \
    (timer) = hal_timer_set(now, (func), (arg), NULL);               \
} while(__LINE__ == -1)

/**
 * 取消指定的定时器
 *
 * @param **tidd  指向待取消的定时器句柄
 *
 * @return 是否成功
 */
bool_t hal_timer_cancel(hal_timer_t **tidd);


/**
 * 获取当前时间
 *
 * @return 当前时间结构体实例化的对象
 */
hal_time_t hal_timer_now(void);


void hal_timer_overflow_interrupt_handler(uint16_t line);
void htimer_int_handler(void);


#endif

/**
 * @}
 */


