/**
 * @brief       : 
 *
 * @file        : wsnos_etimer.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/29
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/29    v0.0.1      gang.cheng    first version
 */
#ifndef __WSNOS_ETIMER_H__
#define __WSNOS_ETIMER_H__

typedef osel_uint32_t etimer_ctr;

typedef struct _osel_etimer_t_
{
    osel_event_t super;
    
    osel_task_t *task;
    osel_pthread_t *p;

    /** link to the next ime event in the list */
    struct _osel_etimer_t_ * volatile next;

    /** the active object that recevies the time events */
    void * volatile act;

    /** the internal down-counter of the time event. */
    osel_uint16_t volatile ctr;

    /** the interval for periodic time event (zero for one-short time event) */
    osel_uint16_t interval;
    
} osel_etimer_t;

/**
 * @brief 该宏只能与pthread库一起使用
 */
#define OSEL_ETIMER_DELAY(et, time)                                         \
    osel_etimer_ctor(et,                                                    \
                     PROCESS_CURRENT(),                                     \
                     PROCESS_EVENT_TIMER,                                   \
                     NULL);                                                 \
    osel_etimer_arm(et, time, 0);                                           \
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);


void osel_etimer_ctor(osel_etimer_t *const me, 
                      osel_pthread_t *p,
                      osel_signal_t sig,
                      osel_param_t param);

etimer_ctr osel_etimer_ctr_get(osel_etimer_t *const me);

void osel_etimer_arm(osel_etimer_t *const me,
                     etimer_ctr const ticks,
                     etimer_ctr const interval);

osel_bool_t osel_etimer_disarm(osel_etimer_t *const me);

osel_bool_t osel_etimer_rearm(osel_etimer_t *const me, etimer_ctr const ticks);

void wsnos_ticks(void);

#endif

