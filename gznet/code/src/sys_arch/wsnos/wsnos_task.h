/**
 * @brief       : 
 *
 * @file        : wsnos_task.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/29
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/29    v0.0.1      gang.cheng    first version
 */

#ifndef __WSNOS_TASK_H__
#define __WSNOS_TASK_H__

typedef void (*osel_task_handler)(void *e); //*< 调度器


typedef struct {
    osel_task_handler entry;                //*< 任务的主函数
    osel_int8_t     prio;                   //*< 任务的优先级
    osel_equeue_t   equeue;                 //*< 任务的事件队列
    struct process  *process_list;          //*< 任务的线程队列
} osel_task_t;

extern osel_uint8_t osel_max_prio;
extern osel_task_t *g_task_list;

#if OSEL_DBG_EN > 0
extern osel_uint8_t osel_register_grp;
extern osel_uint8_t *osel_register_tbl;
#endif


/**
 * @brief 创建一个任务，该版本里面，任务不进行实际任务操作，只是作为线程的载体，
 *        为线程调度的主体。
 * @param[in] entry 任务执行的函数指针,如果为空则不执行
 * @param[in] prio  任务的优先级
 * @param[in] event_sto[] 任务的消息队列实际存储空间的起始地址
 * @param[in] event_len 任务的消息队列实际存储空间的长度
 *
 * @return 指向任务控制块内存的地址
 * @code
 * static uint8_t osel_heap_buf[4096];
 * static osel_task_t *task_tcb = NULL;
 * static osel_event_t task_event_store[10];
 *
 * void task_tcb_entry(void *param)
 * {
 * 	  // 这里可以添加任务每次进入的日志打印信息，用于调试
 *    ...
 * }
 * 
 * int main(void)
 * {
 *     osel_env_init(osel_heap_buf, 4096, 8);
 *     // 这里创建了任务，但没有相关事件触发，任务的主体并不会得到执行
 *     task_tcb = osel_task_create(&task_tcb_entry, 6, task_event_store, 10);
 *
 *     osel_run();
 *     return 0;
 * }
 * @endcode
 */
osel_task_t *osel_task_create(void (*entry)(void *param), 
                              osel_uint8_t prio,
                              osel_event_t event_sto[],
                              osel_uint8_t event_len);

/**
 * @brief 提供系统空闲任务的钩子函数，这个接口多用于低功耗应用。
 * @param[in] idle 空闲任务执行的钩子函数主体
 *
 * @code 
 * static uint8_t osel_heap_buf[4096];
 * 
 * static void sys_enter_lpm_handler(void *p)
 * {
 *     debug_info_printf();	// 延时打印日志信息
 *	   LPM3;
 * }
 *
 * int main(void)
 * {
 *     osel_env_init(osel_heap_buf, 4096, 8);
 *     osel_idle_hook(&sys_enter_lpm_handler);
 *
 *     osel_run();	// 启动系统
 *     return 0;
 * }
 * @endcode
 */
void osel_idle_hook(osel_task_handler idle);

/**
 * @brief 系统启动
 * @see osel_idle_hook
 */
void osel_run(void);

#endif
