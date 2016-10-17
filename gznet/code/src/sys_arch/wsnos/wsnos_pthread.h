/***************************************************************************
* @brief        : this
* @file         : wsnos_pthread.h
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-09-11
* @change Logs  :
* Date        Version      Author      Notes
* 2015-09-11      v0.1      gang.cheng    first version
***************************************************************************/
#ifndef __WSNOS_PTHREAD_H__
#define __WSNOS_PTHREAD_H__

/** lc.h-->lc_switch.h */
typedef unsigned short lc_t;

#define LC_INIT(s) s = 0;

#define LC_RESUME(s) switch(s) { case 0:

#define LC_SET(s) s = __LINE__; case __LINE__:

#define LC_END(s) }


/** pt.h */
typedef struct pt {
    lc_t lc;
} osel_pt_t;

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

#define PT_INIT(pt)             LC_INIT((pt)->lc)

#define PT_THREAD(name_args)    char name_args

#define PT_BEGIN(pt) { char PT_YIELD_FLAG = 1; if (PT_YIELD_FLAG) {;} LC_RESUME((pt)->lc)

#define PT_END(pt) LC_END((pt)->lc); PT_YIELD_FLAG = 0;     \
                   PT_INIT(pt); return PT_ENDED; }

#define PT_WAIT_UNTIL(pt, condition)                        \
    do {                                                    \
        LC_SET((pt)->lc);                                   \
        if(!(condition)) {                                  \
            return PT_WAITING;                              \
        }                                                   \
    } while(0)

#define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))

#define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))

#define PT_SPAWN(pt, child, thread)                         \
    do {                                                    \
        PT_INIT((child));                                   \
        PT_WAIT_THREAD((pt), (thread));                     \
    } while(0)

#define PT_RESTART(pt)                                      \
    do {                                                    \
        PT_INIT(pt);                                        \
        return PT_WAITING;                                  \
    } while(0)

#define PT_EXIT(pt)                                         \
    do {                                                    \
        PT_INIT(pt);                                        \
        return PT_EXITED;                                   \
    } while(0)

#define PT_SCHEDULE(f) ((f) < PT_EXITED)

#define PT_YIELD(pt)                                        \
    do {                                                    \
        PT_YIELD_FLAG = 0;                                  \
        LC_SET((pt)->lc);                                   \
        if(PT_YIELD_FLAG == 0) {                            \
            return PT_YIELDED;                              \
        }                                                   \
    } while(0)


#define PT_YIELD_UNTIL(pt, cond)                            \
    do {                                                    \
        PT_YIELD_FLAG = 0;                                  \
        LC_SET((pt)->lc);                                   \
        if((PT_YIELD_FLAG == 0) || !(cond)) {               \
            return PT_YIELDED;                              \
        }                                                   \
    } while(0)


/** process.h */
typedef osel_uint16_t               process_event_t;
typedef void *                      process_data_t;

#define PROCESS_EVENT_NONE            0x8080
#define PROCESS_EVENT_INIT            0x8081
#define PROCESS_EVENT_POLL            0x8082
#define PROCESS_EVENT_EXIT            0x8083
#define PROCESS_EVENT_SERVICE_REMOVED 0x8084
#define PROCESS_EVENT_CONTINUE        0x8085
#define PROCESS_EVENT_MSG             0x8086
#define PROCESS_EVENT_EXITED          0x8087
#define PROCESS_EVENT_TIMER           0x8088
#define PROCESS_EVENT_COM             0x8089
#define PROCESS_EVENT_MAX             0x808a


#define PROCESS_BEGIN()             PT_BEGIN(process_pt)

#define PROCESS_END()               PT_END(process_pt)

#define PROCESS_WAIT_EVENT()        PROCESS_YIELD()

#define PROCESS_WAIT_EVENT_UNTIL(c) PROCESS_YIELD_UNTIL(c)

#define PROCESS_YIELD()             PT_YIELD(process_pt)

#define PROCESS_YIELD_UNTIL(c)      PT_YIELD_UNTIL(process_pt, c)

#define PROCESS_WAIT_UNTIL(c)       PT_WAIT_UNTIL(process_pt, c)
#define PROCESS_WAIT_WHILE(c)       PT_WAIT_WHILE(process_pt, c)

#define PROCESS_EXIT()              PT_EXIT(process_pt)

#define PROCESS_PT_SPAWN(pt, thread)   PT_SPAWN(process_pt, pt, thread)

#define PROCESS_PAUSE()             do {                            \
    osel_event_t event;                                             \
    event.sig = PROCESS_EVENT_CONTINUE;                             \
    event.param = NULL;                                             \
    osel_post(NULL, PROCESS_CURRENT(), &event);                     \
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);         \
} while(0)

#define PROCESS_POLLHANDLER(handler) if(ev == PROCESS_EVENT_POLL) { handler; }

#define PROCESS_EXITHANDLER(handler) if(ev == PROCESS_EVENT_EXIT) { handler; }

#define PROCESS_THREAD(name, ev, data)                              \
static PT_THREAD(process_thread_##name(struct pt *process_pt,       \
                process_event_t ev,                                 \
                process_data_t data))

#define PROCESS_NAME(name)          extern struct process name

#define PROCESS(name, strname)                                      \
    PROCESS_THREAD(name, ev, data);                                 \
        struct process name = {NULL,                                \
                          process_thread_##name }


typedef struct process {
    struct process *next;
    PT_THREAD((* thread)(struct pt *, process_event_t, process_data_t));
    struct pt pt;
    unsigned char state, needspoll;
    osel_task_t *tcb;
} osel_pthread_t;


#define PROCESS_CURRENT() process_current
extern struct process *process_current;

void osel_pthread_init(void);

/**
 * @brief 退出线程，把线程动态从任务的队列里面动态卸载
 * @param[in] tcb 指向任务控制块的指针
 * @param[in] p 指向线程控制块的指针
 * @param[in] fromprocess 对应执行了退出p线程的原始线程指针
 *
 * @return 退出线程成功还是失败
 *
 * @code
 * static uint8_t osel_heap_buf[4096];
 * static osel_task_t *task_tcb = NULL;
 * static osel_event_t task_event_store[10];
 *
 * PROCESS_NAME(task_thread1_process);
 * PROCESS_NAME(task_thread2_process);
 *
 * PROCESS(task_thread1_process, "task thread 1 process");
 * PROCESS_THREAD(task_thread1_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {   // 系统自定义事件
 *         if(ev == PROCESS_EVENT_INIT) 
 *         {
 *             ...
 *         }
 *         // 释放线程控制权，进行任务切换
 *         PROCESS_YIELD();             
 *     }
 *
 *     DBG_LOG(DBG_LEVEL_INFO, "task thread1 process exit\r\n");
 *
 *     PROCESS_END();
 * }
 *
 * PROCESS(task_thread2_process, "task thread 2 process");
 * PROCESS_THREAD(task_thread2_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *             // 从当前线程2里面把线程1从任务里面退出
 *             osel_pthread_exit(task_tcb, &task_thread1_process, PROCESS_CURRENT());
 *         }
 *         
 *         ...
 *
 *         PROCESS_YIELD();             // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 * 
 * int main(void)
 * {
 *     osel_env_init(osel_heap_buf, 4096, 8);
 *     task_tcb = osel_task_create(NULL, 6, task_event_store, 10);
 *
 *     osel_pthread_create(task_tcb, &task_thread1_process, NULL);
 *
 *     // 创建完线程2以后，退出线程1
 *     osel_pthread_create(task_tcb, &task_thread2_process, NULL);
 *
 *     osel_run();
 *     return 0;
 * }
 *
 * @endcode
 */
osel_bool_t osel_pthread_exit(osel_task_t *tcb, osel_pthread_t *p, osel_pthread_t *fromprocess);

/**
 * @brief 创建线程，把线程动态加载到任务控制体内
 * @param[in] tcb 指向任务控制块的指针
 * @param[in] p 指向线程控制块的指针
 * @param[in] data 线程初始化创建以后调用的参数
 *
 * @return 创建线程成功还是失败
 *
 * @code
 * static uint8_t osel_heap_buf[4096];
 * static osel_task_t *task_tcb = NULL;
 * static osel_event_t task_event_store[10];
 *
 * PROCESS_NAME(task_thread1_process);
 * PROCESS_NAME(task_thread2_process);
 *
 * PROCESS(task_thread1_process, "task thread 1 process");
 * PROCESS_THREAD(task_thread1_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *             ...
 *         }
 *         else if(ev == XXX_EVENT)     // 用户自定义事件
 *         {
 *             ...
 *         }
 *
 *         PROCESS_YIELD();             // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 *
 * PROCESS(task_thread2_process, "task thread 2 process");
 * PROCESS_THREAD(task_thread2_process, ev, data)
 * {
 *     PROCESS_BEGIN();
 *
 *     while(1)
 *     {
 *         if(ev == PROCESS_EVENT_INIT) // 系统自定义事件
 *         {
 *             ...
 *         }
 *
 *         PROCESS_YIELD();             // 释放线程控制权，进行任务切换
 *     }
 *
 *     PROCESS_END();
 * }
 * 
 * int main(void)
 * {
 *     osel_env_init(osel_heap_buf, 4096, 8);
 *     task_tcb = osel_task_create(NULL, 6, task_event_store, 10);
 *
 *     osel_pthread_create(task_tcb, &task_thread1_process, NULL);
 *     osel_pthread_create(task_tcb, &task_thread2_process, NULL);
 *
 *     osel_run();
 *     return 0;
 * }
 *
 * @endcode
 */
osel_bool_t osel_pthread_create(osel_task_t *tcb,  osel_pthread_t *p, osel_param_t data);

void osel_pthread_poll(osel_task_t *tcb, osel_event_t event);

#endif
