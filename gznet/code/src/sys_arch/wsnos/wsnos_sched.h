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
 * @code
 *
 * #define TASK_A_LED_EVENT			(0x3000)
 *
 * static uint8_t osel_heap_buf[4096];
 * static osel_task_t *task_a_tcb = NULL;
 * static osel_event_t task_a_event_store[10];
 *
 * static osel_task_t *task_b_tcb = NULL;
 * static osel_event_t task_b_event_store[10];
 *
 * PROCESS_NAME(task_a_thread1_process);
 * PROCESS_NAME(task_a_thread2_process);
 * PROCESS_NAME(task_b_thread1_process);
 *
 * PROCESS(task_a_thread1_process, "task a thread 1 process");
 * PROCESS_THREAD(task_a_thread1_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *             ...
 *         }
 *         else if(ev == TASK_A_LED_EVENT)
 *         {
 *             DBG_LOG(DBG_LEVEL_INFO, "task a thread 1 led handle\r\n");
 *         }
 *
 *         PROCESS_YIELD();     // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 *
 * PROCESS(task_a_thread2_process, "task a thread 2 process");
 * PROCESS_THREAD(task_a_thread2_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *             ...
 *         }
 *         else if(ev == TASK_A_LED_EVENT)
 *         {
 *             DBG_LOG(DBG_LEVEL_INFO, "task a thread 2 led handle\r\n");
 *         }
 *
 *         PROCESS_YIELD();       // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 *
 * PROCESS(task_b_thread1_process, "task b thread 1 process");
 * PROCESS_THREAD(task_a_thread1_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *         	   osel_event_t event;
 *         	   event.sig   = TASK_A_LED_EVENT;
 *         	   event.param = NULL;
 *             osel_post(task_a_tcb, NULL, &event);	// 把消息发送给任务A
 *         }
 *
 *         PROCESS_YIELD();       // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 *
 * int main(void)
 * {
 *     osel_env_init(osel_heap_buf, 4096, 8);
 *     debug_init(DBG_LEVEL_INFO);
 *     task_a_tcb = osel_task_create(NULL, 6, task_a_event_store, 10);
 *     osel_pthread_create(task_a_tcb, &task_a_thread1_process, NULL);
 *     osel_pthread_create(task_a_tcb, &task_a_thread2_process, NULL);
 *
 *     task_b_tcb = osel_task_create(NULL, 7, task_b_event_store, 10);
 *     osel_pthread_create(task_b_tcb, &task_b_thread1_process, NULL);
 *
 *     osel_run();
 *     return 0;
 * }
 *
 * @endcode
 */
void osel_post(osel_task_t *task, osel_pthread_t *p, osel_event_t *event);

/**
 * @brief 该函数供WSNOS内部使用，提供任务的调度切换（用户不需要调用）
 */
void osel_schedule(void);

#endif


