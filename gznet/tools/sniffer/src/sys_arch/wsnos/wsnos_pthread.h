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

#define PROCESS_EVENT_NONE            0x8000
#define PROCESS_EVENT_INIT            0x8100
#define PROCESS_EVENT_POLL            0x8200
#define PROCESS_EVENT_EXIT            0x8300
#define PROCESS_EVENT_SERVICE_REMOVED 0x8400
#define PROCESS_EVENT_CONTINUE        0x8500
#define PROCESS_EVENT_MSG             0x8600
#define PROCESS_EVENT_EXITED          0x8700
#define PROCESS_EVENT_TIMER           0x8800
#define PROCESS_EVENT_COM             0x8900
#define PROCESS_EVENT_MAX             0x8a00


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
    process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL);  \
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

osel_bool_t osel_pthread_exit(osel_task_t *tcb, osel_pthread_t *p, osel_pthread_t *fromprocess);

osel_bool_t osel_pthread_create(osel_task_t *tcb,  osel_pthread_t *p, osel_param_t data);

void osel_pthread_poll(osel_task_t *tcb, osel_event_t event);

#endif
