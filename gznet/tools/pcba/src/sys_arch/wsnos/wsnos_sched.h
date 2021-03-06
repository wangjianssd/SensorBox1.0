/**
 * @brief       : 
 *
 * @file        : wsnos_sched.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/29
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/29    v0.0.1      gang.cheng    first version
 */
#ifndef __WSNOS_SCHED_H__
#define __WSNOS_SCHED_H__

extern osel_uint8_t osel_rdy_grp;
extern osel_uint8_t *osel_rdy_tbl;
extern osel_int8_t osel_curr_prio;

extern osel_uint8_t const CODE osel_map_tbl[];

#define TASK_CURRENT()                          \
    &g_task_list[osel_curr_prio]

/**
 * @brief 互斥锁，对临界区代码进行保护，在互斥锁锁定到解锁之间的代码是不能被任务
 *        切换打断的，但是能进行中断响应
 * @code
 *      ...
 *      osel_int8_t s = 0;
 *      s = osel_mutex_lock(OSEL_MAX_PRIO);
 *      // 这里就是临界区代码，需要用互斥锁进行保护
 *      osel_mutex_unlock(s);
 *
 */
osel_int8_t osel_mutex_lock(osel_int8_t prio_ceiling);

/**
 * @brief 互斥锁解锁
 * @see osel_mutex_lock()
 */
void osel_mutex_unlock(osel_int8_t org_prio);

/**
 * @brief 消息发送，把事件块发送到某个任务的某个线程，如果任务非空，则发送到任务所在
 *        的所有线程，如果任务为空，线程不为空，则消息发送给指定的线程，
 * @param[in] task 指定的任务控制块指针
 * @param[in] p 指定的线程控制块指针
 * @param[in] event 指向任务控制块的指针
 *
 * @return 
 */
void osel_post(osel_task_t *task, osel_pthread_t *p, osel_event_t *event);

/**
 * @brief 该函数供WSNOS内部使用，提供任务的调度切换（用户不需要调用）
 */
void osel_schedule(void);

#endif

