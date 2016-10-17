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
 * @brief 动态创建任务
 * @param[in] entry 指向任务处理的入口函数
 * @param[in] prio 任务的优先级
 * @param[in] event_sto[] 指定任务的事件队列地址
 * @param[in] equeue_size 指定任务的事件队列容量
 * 
 * @return 指向任务控制块内存的地址
 */
osel_task_t *osel_task_create(void (*entry)(void *param), 
                              osel_uint8_t prio,
                              osel_event_t event_sto[],
                              osel_uint8_t event_len);

void osel_idle_hook(osel_task_handler idle);

void osel_run(void);

#endif
