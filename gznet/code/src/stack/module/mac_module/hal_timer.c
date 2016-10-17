/**
 * @brief       :
 *
 * @file        : hal_timer.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/hal/hal.h"
#include "hal_timer.h"
#include "sys_arch/osel_arch.h"
     
//DBG_THIS_MODULE("hal_timer")

/* 形参 */
#define _TIMER_OF_LINE  uint16_t line
#define _TIMER_CMP_LINE  uint16_t line
/* 实参 */
#define TIMER_OF_LINE  __LINE__
#define TIMER_CMP_LINE  __LINE__

#define ERR_SET(x)      st( if(err != NULL) *err = (x);)

static volatile spliced_time_t global_timer_var;
static hal_timer_t hal_timer_array[MAX_TIMERS];
static list_head_t free_hal_timer;
static list_head_t alloc_hal_timer;
static uint8_t hal_timer_count = 0;
volatile hard_timer_state_t  hard_timer_state;

static void hal_timer_set_overflow_interrupt(void);

uint8_t st2_temp;       //该变量用来读取ST2寄存器的值，不使用
static volatile uint16_t timer_of_line = 0;
static volatile uint16_t timer_cmp_line = 0;
hal_time_t time_now;

//ticks数转换成时间延时
static void delay_tick(uint16_t ticks)
{
    uint32_t delay_us = TICK_TO_US(ticks);
    uint32_t div_us = delay_us / 10; // 大约120us
    for (uint32_t i = 0; i < div_us; i++)
    {
        for (uint8_t j = 0; j < 9; j++) //9.8us
        {
            _NOP();
        }
    }
}

hal_time_t hal_timer_now(void)
{
    hal_time_t now;
    now.w = 0x0u;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    HTIMER_HWCOUNT(now.s.hw);
    now.s.sw = global_timer_var.sw;

    if (TIMER_SET_TH4 == 0)
    {
        if ((HTIMER_OF_FLG() == 1) && (hard_timer_state == TIMER_EXPECT_OFINT))
        {
            now.s.sw++;
            HTIMER_HWCOUNT(now.s.hw);
            DBG_TIMER(0x25);
        }
    }
    else //*< 2530平台中溢出中断的比较值可能不是0，所以一直等到溢出中断触发
    {
        while (now.s.hw <= TIMER_SET_TH4)
        {
            if ((HTIMER_OF_FLG() == 1) && (hard_timer_state == TIMER_EXPECT_OFINT))
            {
                now.s.sw++;
                HTIMER_HWCOUNT(now.s.hw);
                DBG_TIMER(0x25);
                break;
            }
            HTIMER_HWCOUNT(now.s.hw);
        }
    }

    HAL_EXIT_CRITICAL(s);
    return now;
}

static void hal_timer_queue_init(void)
{
    list_init(&free_hal_timer);
    list_init(&alloc_hal_timer);

    for (uint8_t i = 0; i < MAX_TIMERS; i++)
    {
        list_add_to_tail(&hal_timer_array[i].list, &free_hal_timer);
    }
    hal_timer_count = 0;

    hard_timer_state = TIMER_EXPECT_OFINT;

    global_timer_var.sw = 0x0u;
    global_timer_var.hw = 0x0u;
}

static void hal_timer_start(void)
{
    TIMER_START();
    hal_timer_set_overflow_interrupt();
    HTIMER_CLEAR_OF_FLG();
    HTIMER_CLEAR_CMP_FLG();
#if OVERFLOW_INT_EN == 0
    HTIMER_COMPINT_ENABLE();
#else
    HTIMER_COMPINT_DISABLE();
#endif
}

void hal_timer_init(void)
{
    hal_timer_queue_init();
    hal_timer_start();
}

static void hal_timer_set_overflow_interrupt(void)
{
    hal_time_t stime;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    stime.s.sw = global_timer_var.sw + 1;
    stime.s.hw = 0x0u;
    HTIMER_SET_COMPREG(&stime);

    hard_timer_state = TIMER_EXPECT_OFINT;

    HAL_EXIT_CRITICAL(s);
}

static hal_timer_t *hal_timer_resources_alloc(hal_time_t *stime,
        hal_timer_cb_t func,
        void *arg)
{
    DBG_ASSERT(stime != NULL __DBG_LINE);
    DBG_ASSERT(func != NULL __DBG_LINE);

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    hal_timer_t *alloc_timer = list_entry_decap(&free_hal_timer, hal_timer_t, list);
    HAL_EXIT_CRITICAL(s);

    if (alloc_timer == NULL)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
        return NULL;
    }

    if (alloc_timer != NULL)
    {
        hal_timer_count++;
    }

    alloc_timer->ftime = *stime;
    alloc_timer->func = func;
    alloc_timer->arg = arg;
    return alloc_timer;
}

/**
 *  比较两个本地时间的大小，前比后小返回TRUE
 */
static bool_t hal_timer_compare(hal_timer_t const *const ta,
                                hal_timer_t const *const tb)
{
    DBG_ASSERT(ta != NULL __DBG_LINE);
    DBG_ASSERT(tb != NULL __DBG_LINE);

    if (ta->ftime.s.sw == tb->ftime.s.sw)
    {
        if (ta->ftime.w < tb->ftime.w)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    if ((hal_timer_sw_t)(ta->ftime.s.sw - tb->ftime.s.sw) > HTIMER_HALF_TURNOVER)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void hal_timer_free(hal_timer_t *tid)
{
    DBG_ASSERT(tid != NULL __DBG_LINE);
    if (tid == NULL)
    {
        return;
    }

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    hal_timer_count--;
    list_add_to_tail(&(tid->list), &free_hal_timer);
    HAL_EXIT_CRITICAL(s);
}

static bool_t judge_hw_timer_is_vailed(hal_time_t *stime,
                                       hal_time_t *now,
                                       hal_timer_cb_t func,
                                       void *arg,
                                       hal_timer_err_t *err)
{
    if (stime->s.hw < now->s.hw)
    {
        if ((now->s.hw - stime->s.hw) < TIMER_SET_TH3)
        {
            func(arg);
            DBG_TIMER(0x01);
            ERR_SET(TIMER_SUCCESS);
        }
        else
        {
            DBG_TIMER(0x02);
            ERR_SET(TIMER_ERROR_INVALID_INPUT);
        }
        return FALSE;
    }
    else
    {
        if ((stime->s.hw - now->s.hw) < TIMER_SET_TH)
        {
            DBG_TIMER(0x03);
            func(arg);
            ERR_SET(TIMER_SUCCESS);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
}


hal_timer_t *hal_timer_set(hal_time_t stime,
                           hal_timer_cb_t func,
                           void *arg,
                           hal_timer_err_t *err)
{
    hal_time_t now;
    hal_timer_t *timer_handler = NULL;
    if (func == NULL)
    {
        ERR_SET(TIMER_ERROR_INVALID_INPUT);
        DBG_TIMER(0x04);
        return NULL;
    }

    hal_int_state_t s = 0;
    HAL_ENTER_CRITICAL(s);

    now = hal_timer_now();
    global_timer_var.hw = now.s.hw;

    if ((hal_timer_sw_t)(stime.s.sw - now.s.sw) > HTIMER_HALF_TURNOVER)// 过期
    {
        ERR_SET(TIMER_ERROR_INVALID_INPUT);
        HAL_EXIT_CRITICAL(s);
        DBG_TIMER(0x05);
        return NULL;
    }
    else if (stime.s.sw == now.s.sw)
    {
        if (judge_hw_timer_is_vailed(&stime, &now, func, arg, err) == FALSE)
        {
            HAL_EXIT_CRITICAL(s);
            return NULL;
        }
        else
        {
            ; //continue
        }
    }
    else
    {
        ; //continue
    }

    timer_handler = hal_timer_resources_alloc(&stime, func, arg);
    if (timer_handler == NULL)
    {
        ERR_SET(TIMER_ERROR_FULL);
        HAL_EXIT_CRITICAL(s);
        DBG_TIMER(0x06);
        return NULL;
    }

    list_head_t *pos = NULL;
    list_entry_sorted_add(&(timer_handler->list),
                          &alloc_hal_timer,
                          hal_timer_t, list,
                          hal_timer_compare, pos);

    /*插入的是第一个元素*/
    if (list_first_elem_look(&alloc_hal_timer) == &timer_handler->list)
    {
        if (stime.s.sw > global_timer_var.sw)
        {
            // 该处在定时器回调中又开始定时操作会报错
//            DBG_ASSERT(hard_timer_state == TIMER_EXPECT_OFINT __DBG_LINE);

            DBG_TIMER(0x07);
        }
        else if (stime.s.sw == global_timer_var.sw)
        {
            HTIMER_SET_COMPREG(&stime);
            hard_timer_state = TIMER_EXPECT_CMPINT;
            DBG_TIMER(0x08);
        }
        else if (stime.s.sw < global_timer_var.sw)
        {
            /* 因为上面已经判断过stime是否过期，所以这里出现这个分支的情况只能是发生了
               软件周期的翻转 */
            //DBG_ASSERT(FALSE __DBG_LINE);
        }
    }
    else
    {
        DBG_TIMER(0x09);
    }

    ERR_SET(TIMER_SUCCESS);

    HAL_EXIT_CRITICAL(s);
    return timer_handler;
}

void hal_timer_controler_init(htimer_controler_t *htimer_controler)
{
    uint8_t i = 0;

    DBG_ASSERT(htimer_controler != NULL __DBG_LINE);

    list_init(&free_hal_timer);
    list_init(&alloc_hal_timer);

    for (i = 0; i < MAX_TIMERS; i++)
    {
        list_add_to_tail(&(hal_timer_array[i].list), &free_hal_timer);
    }
    hal_timer_count = 0;

    hard_timer_state = TIMER_EXPECT_OFINT;

    global_timer_var.sw = 0xFA;

    htimer_controler->cancel = hal_timer_cancel;
    htimer_controler->now = hal_timer_now;
    htimer_controler->set = hal_timer_set;

    hal_timer_start();

    return;
}

/* 该函数调用者负责重入保护，自身不保护 */
static bool_t set_timer_within_overflow(hal_timer_t *next_timer)
{
    hal_timer_hw_t temp = 0;
    hal_timer_hw_t hard_ticks = 0x0u;

    HTIMER_HWCOUNT(hard_ticks);     //TODO:如果进入断言宏，这里需要进行修正
    temp = next_timer->ftime.s.hw - hard_ticks;
    if (next_timer->ftime.s.hw < hard_ticks)    //定时器过期
    {
        DBG_TIMER(0x0a);
        hal_timer_compare_interrupt_handler(TIMER_CMP_LINE);
        return FALSE;
    }
    else if (temp < TIMER_SET_TH)               //定时器在阀值内到期
    {
        DBG_TIMER(0x0b);

        delay_tick(temp);
        hal_timer_compare_interrupt_handler(TIMER_CMP_LINE);
        return FALSE;
    }

    DBG_TIMER(0x0c);
    HTIMER_SET_COMPREG((&next_timer->ftime));
    hard_timer_state = TIMER_EXPECT_CMPINT;
    return TRUE;
}

static void hal_timer_set_next_timer(bool_t result)
{
    hal_timer_t *next_timer = NULL;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    next_timer = list_entry_get_head(&alloc_hal_timer, hal_timer_t, list);

    if (next_timer == NULL)
    {
        hal_timer_set_overflow_interrupt();
        DBG_TIMER(0x0d);
        HAL_EXIT_CRITICAL(s);
    }
    else
    {
        hal_time_t now = hal_timer_now();
        DBG_ASSERT(now.s.sw == global_timer_var.sw __DBG_LINE);
        /* 定时器在以后的软件周期 */
        if ((hal_timer_sw_t)(now.s.sw - next_timer->ftime.s.sw)
                > HTIMER_HALF_TURNOVER)
        {
            hal_timer_set_overflow_interrupt();
            HAL_EXIT_CRITICAL(s);
            DBG_TIMER(0x0e);
        }
        else if (now.s.sw == next_timer->ftime.s.sw )
        {
            /* 定时器在当前软件周期 */
            set_timer_within_overflow(next_timer);
            HAL_EXIT_CRITICAL(s);
        }
        else if ((hal_timer_sw_t)(now.s.sw - next_timer->ftime.s.sw)
                 < HTIMER_HALF_TURNOVER)
        {
            /* 定时器在过去的软件周期 */
            DBG_TIMER(0x0f);
            hal_timer_compare_interrupt_handler(TIMER_CMP_LINE);
        }
    }

    HAL_EXIT_CRITICAL(s);
}

/* 该函数自身不进行重入保护 */
// hal_timer_cancel -> cancel_next_queue_head
static void compare_a_b(hal_timer_hw_t now1, hal_timer_hw_t now2)
{
    hal_timer_hw_t temp = 0;

    if (now2 < now1)        // 硬件时间在本函数内已翻转
    {
        hal_timer_overflow_interrupt_handler(TIMER_OF_LINE);
        DBG_TIMER(0x1a);
    }
    else
    {
        now2 += TIMER_SET_TH2;
        if (now2 < now1)    // 硬件时间在阈值内翻转
        {
            do
            {
                HTIMER_HWCOUNT(temp);
            }
            while (temp > (0xFFFF - TIMER_SET_TH2));

            hal_timer_overflow_interrupt_handler(TIMER_OF_LINE);
            DBG_TIMER(0x1b);
        }
        else
        {
            hal_timer_set_next_timer(FALSE);
            DBG_TIMER(0x1c);
        }
    }
}


/* 自身不进行重入保护 */
static void cancel_next_queue_head(hal_timer_hw_t now1,
                                   hal_timer_t *tid,
                                   hal_timer_t **tidd)
{
    hal_timer_hw_t now2 = 0;

    DBG_ASSERT(tid != NULL __DBG_LINE);
    list_del(&tid->list);
    hal_timer_free(tid);

    hal_time_t now = hal_timer_now();
    if (tid->ftime.s.sw < now.s.sw)
    {
        DBG_TIMER(0x19);
    }
    else if (tid->ftime.s.sw == now.s.sw)
    {
        /* 设置一个很久以后触发的时间，保证不会被触发; */
        hal_time_t stime;
        if (TIMER_SET_TH4 == 0)
        {
            HTIMER_HWCOUNT(stime.s.hw);
            hwtime_min_num(&stime.s.hw, TIMER_SET_TH2);
        }
        else    //*< cc2530
        {
            stime.s.sw = global_timer_var.sw + TIMER_SET_TH;
            stime.s.hw = 0;
        }

        HTIMER_SET_COMPREG(&stime);

        if (HTIMER_CMP_FLG() && hard_timer_state == TIMER_EXPECT_CMPINT)
        {
            HTIMER_CLEAR_CMP_FLG();
            DBG_TIMER(0x27);
        }

        HTIMER_HWCOUNT(now2);
        compare_a_b(now1, now2);
        DBG_TIMER(0x22);
    }
    else
    {
        DBG_ASSERT(hard_timer_state == TIMER_EXPECT_OFINT __DBG_LINE);
        DBG_TIMER(0x1d);
    }
}

/* 不自身进行重入保护 */
static bool_t cancel_next_queue_in(hal_timer_t *tid, hal_timer_t **tidd)
{
    list_head_t *pos = NULL;
    bool_t inlist = FALSE;

    list_for_each_forwards(pos, alloc_hal_timer.next)
    {
        if (pos == &(tid->list))
        {
            inlist = TRUE;
            break;
        }
    }

    if (inlist)
    {
        list_del(&(tid->list));
        hal_timer_free(tid);
        tid = NULL;
        *tidd = NULL;
        DBG_TIMER(0x1e);
        return TRUE;
    }
    else
    {
        tid = NULL;
        *tidd = NULL;
        DBG_TIMER(0x1f);
        return FALSE;
    }
}

bool_t hal_timer_cancel(hal_timer_t **tidd)
{
    hal_timer_hw_t now1 = 0;
    DBG_ASSERT(tidd != NULL __DBG_LINE);
    DBG_ASSERT(*tidd != NULL __DBG_LINE);
    hal_timer_t *tid = *tidd;
    hal_timer_t *next_timer = NULL;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    HTIMER_HWCOUNT(now1);
    /* 如果溢出在读取HW时活之前发生，为确保后继判断是否溢出正确，改变HW值 */
    if (hard_timer_state == TIMER_EXPECT_OFINT)
    {
        if ((now1 <= TIMER_SET_TH2) || (HTIMER_OF_FLG()))
        {
            DBG_TIMER(0x2B);
            now1 = 0xFFFF;
        }
    }

    if ((tidd == NULL) || (*tidd == NULL))
    {
        HAL_EXIT_CRITICAL(s);
        DBG_TIMER(0x17);
        return FALSE;
    }

    next_timer = list_entry_get_head(&alloc_hal_timer, hal_timer_t, list);
    if (next_timer == NULL)
    {
        tid = NULL;
        *tidd = NULL;
        HAL_EXIT_CRITICAL(s);
        DBG_TIMER(0x18);
        return FALSE;
    }

    if (tid == next_timer)
    {
        cancel_next_queue_head(now1, tid, tidd);
        tid = NULL;
        *tidd = NULL;
        HAL_EXIT_CRITICAL(s);
        return TRUE;
    }
    else
    {
        if (cancel_next_queue_in(tid, tidd))
        {
            HAL_EXIT_CRITICAL(s);
            return TRUE;
        }
        else
        {
            HAL_EXIT_CRITICAL(s);
            DBG_TIMER(0x20);
            return FALSE;
        }
    }
}

/* 本函数自身无重入保护 */
//hal_timer_compare_interrupt_handler -> ticks_compare_now
static void ticks_compare_now(hal_timer_t *first)
{
    hal_timer_hw_t temp = 0;
    hal_timer_hw_t now1_hw = 0;
    hal_timer_hw_t now2_hw = 0;
    DBG_ASSERT(first != NULL __DBG_LINE);

    now1_hw = first->ftime.s.hw;

    HTIMER_HWCOUNT(now2_hw);

    if (now2_hw < now1_hw)
    {
        if (first->ftime.s.sw < global_timer_var.sw) /* overflow is already dealed */
        {
            DBG_TIMER(0x26);
            hal_timer_set_next_timer(FALSE);
        }
        else
        {
            DBG_TIMER(0x10);
            hal_timer_overflow_interrupt_handler(TIMER_OF_LINE);
        }
    }
    else
    {
        now2_hw += TIMER_SET_TH2;

        if (now2_hw < now1_hw) /*即将翻转*/
        {
            do
            {
                HTIMER_HWCOUNT(temp);
            }
            while (temp > (0xFFFF - TIMER_SET_TH2)); /*等待翻转*/


            DBG_TIMER(0x11);
            hal_timer_overflow_interrupt_handler(TIMER_OF_LINE);
        }
        else
        {
            DBG_TIMER(0x12);

            hal_timer_set_next_timer(FALSE);
        }
    }
}

void hal_timer_overflow_interrupt_handler(uint16_t line)
{
    uint16_t now;

    timer_of_line = line;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    HTIMER_HWCOUNT(now);
    if (now > 30)       //for debug
    {
        DBG_TIMER(0x23);
    }

    global_timer_var.sw++;

    /* 设置一个很久以后触发的时间，保证不会被触发*/
    hal_time_t stime;
    if (TIMER_SET_TH4 == 0)
    {
        HTIMER_HWCOUNT(stime.s.hw);
        hwtime_min_num(&stime.s.hw, TIMER_SET_TH2);
    }
    else    //*< cc2530
    {
        stime.s.sw = global_timer_var.sw + TIMER_SET_TH;
        stime.s.hw = 0;
    }

    HTIMER_SET_COMPREG(&stime);

    if (HTIMER_OF_FLG() && (hard_timer_state == TIMER_EXPECT_OFINT))
    {
        HTIMER_CLEAR_OF_FLG();
        DBG_TIMER(0x24);
    }

    HAL_EXIT_CRITICAL(s);

    DBG_TIMER(0x21);
    hal_timer_set_next_timer(FALSE);
}

void hal_timer_compare_interrupt_handler(_TIMER_CMP_LINE)
{
    hal_timer_t *first = NULL;

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    first = list_entry_decap(&alloc_hal_timer, hal_timer_t, list);
    HAL_EXIT_CRITICAL(s);

    timer_cmp_line = line;

    DBG_TIMER(0x13);
    if ((first != NULL) && (first->func != NULL))
    {
        first->func(first->arg);
    }

    HAL_ENTER_CRITICAL(s);

    hal_timer_free(first);
    ticks_compare_now(first);

    HAL_EXIT_CRITICAL(s);
}


void htimer_int_handler(void)
{
    HTIMER_COMPINT_DISABLE();

    if (hard_timer_state == TIMER_EXPECT_OFINT)
    {
        HTIMER_CLEAR_OF_FLG();
        DBG_TIMER(0x14);
        hal_timer_overflow_interrupt_handler(TIMER_OF_LINE);
    }
    else if (hard_timer_state == TIMER_EXPECT_CMPINT)
    {
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11);
        HTIMER_CLEAR_CMP_FLG();
        hal_timer_compare_interrupt_handler(TIMER_CMP_LINE);
    }
    else
    {
        DBG_TIMER(0x16);
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}
#undef ERR_SET
