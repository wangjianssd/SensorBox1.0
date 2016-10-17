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

/**
 * @brief 初始化定时器
 * @param me 指向定时器的指针
 * @param p  定时器绑定的线程指针
 * @param sig  定时器响应以后发送的消息
 * @param param 定时器响应以后发送的参数
 */
void osel_etimer_ctor(osel_etimer_t *const me, 
                      osel_pthread_t *p,
                      osel_signal_t sig,
                      osel_param_t param);

etimer_ctr osel_etimer_ctr_get(osel_etimer_t *const me);

/**
 * @brief 启动某一个定时器
 * @param me 指向定时器的指针
 * @param ticks 定时器在多少个tick以后启动（每个tick由硬件设定）
 * @param interval 指定定时器触发以后下次启动的间隔时间
 *  - 0 定时器只触发一次，启动以后就释放
 *  - !0 定时器周期性启动，在第一次触发以后，以该值作为下次启动的周期
 */
void osel_etimer_arm(osel_etimer_t *const me,
                     etimer_ctr const ticks,
                     etimer_ctr const interval);

/**
 * @brief 取消定时器执行
 * @param  me 指向要取消的定时器指针
 * @return 定时器是否取消成功
 *  - TRUE 定时器被完全取消
 *  - FALSE 定时器已经执行过了，取消失败
 */
osel_bool_t osel_etimer_disarm(osel_etimer_t *const me);


/**
 * @brief 定时器重新设定，如果定时器之前是一次有效的，重新定时以后还是一次有效；
 * 多次有效的还是多次有效；
 * @param  me 指向要取消的定时器指针
 * @param  ticks 定时器在多少个tick以后启动（每个tick由硬件设定）
 * @return  定时器是否已经被触发过
 *  - TRUE 定时器没有被触发，且重新定时成功
 *  - FALSE 定时器已经被触发，且重新定时成功
 */
osel_bool_t osel_etimer_rearm(osel_etimer_t *const me, etimer_ctr const ticks);

void wsnos_ticks(void);

#endif

