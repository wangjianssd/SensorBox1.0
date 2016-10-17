/***************************************************************************
* @brief        : this
* @file         : wsnos_pthread.c
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-09-11
* @change Logs  :
* Date        Version      Author      Notes
* 2015-09-11      v0.1      gang.cheng    first version
***************************************************************************/
#include "common/lib/lib.h"
#include "wsnos.h"

#define PROCESS_STATE_NONE        0
#define PROCESS_STATE_RUNNING     1
#define PROCESS_STATE_CALLED      2

struct process *process_current = NULL;

static void osel_pthread_call(osel_task_t *tcb,
                              osel_pthread_t *p,
                              osel_signal_t sig,
                              osel_param_t param);

bool_t osel_pthread_create(osel_task_t *tcb,  osel_pthread_t *p, osel_param_t data)
{
    if (tcb == NULL) {
        return FALSE;
    }

    osel_pthread_t *q = NULL;
    /* First make sure that we don't try to start a process that is
     already running. */
    for (q = tcb->process_list; q != p && q != NULL; q = q->next);
    if (q == p) {
        return FALSE;
    }

    osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    p->next  = tcb->process_list;
    tcb->process_list = p;
    p->state = PROCESS_STATE_RUNNING;
    p->tcb   = tcb;
    PT_INIT(&p->pt);

    osel_pthread_t *caller = process_current;
    OSEL_EXIT_CRITICAL(s);

    osel_pthread_call(tcb, p, PROCESS_EVENT_INIT, data);

    OSEL_ENTER_CRITICAL(s);
    process_current = caller;
    OSEL_EXIT_CRITICAL(s);

    return TRUE;
}

static int pthread_is_running(struct process *p)
{
    return p->state != PROCESS_STATE_NONE;
}

bool_t osel_pthread_exit(osel_task_t *tcb, osel_pthread_t *p, osel_pthread_t *fromprocess)
{
    register osel_pthread_t *q = NULL;
    osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    osel_pthread_t *old_current = process_current;
    OSEL_EXIT_CRITICAL(s);

    /* Make sure the process is in the process list before we try to
     exit it. */
    for (q = tcb->process_list; q != p && q != NULL; q = q->next);

    if (q == NULL) {
        return FALSE;
    }

    if (pthread_is_running(p)) {
        p->state = PROCESS_STATE_NONE;
        /*
         * Post a synchronous event to all processes to inform them that
         * this process is about to exit. This will allow services to
         * deallocate state associated with this process.
         */
        for (q = tcb->process_list; q != p && q != NULL; q = q->next) {
            if (p != q) {
                osel_pthread_call(tcb, q, PROCESS_EVENT_EXITED, (process_data_t)p);
            }
        }

        if (p->thread != NULL && p != fromprocess) {
            /* Post the exit event to the process that is about to exit. */
            process_current = p;
            p->thread(&p->pt, PROCESS_EVENT_EXIT, NULL);
        }
    }

    OSEL_ENTER_CRITICAL(s);
    if (p == tcb->process_list) {
        tcb->process_list = tcb->process_list->next;
    } else {
        for (q = tcb->process_list; q != NULL; q = q->next) {
            if (q->next == p) {
                q->next = p->next;
                break;
            }
        }
    }

    process_current = old_current;
    OSEL_EXIT_CRITICAL(s);

    return TRUE;
}

static void osel_pthread_call(osel_task_t *tcb, osel_pthread_t *p, osel_signal_t sig, osel_param_t param)
{
    char ret;
    if ((p->state & PROCESS_STATE_RUNNING) &&
            (p->thread != NULL)) {
        osel_int_status_t s;
        OSEL_ENTER_CRITICAL(s);
        osel_pthread_t *caller = process_current;
        process_current = p;
        OSEL_EXIT_CRITICAL(s);

        p->state = PROCESS_STATE_CALLED;
        ret = p->thread(&p->pt, sig, param);
        if (    ret == PT_EXITED ||
                ret == PT_ENDED ||
                sig == PROCESS_EVENT_EXIT) {
            osel_pthread_exit(tcb, p, p);
        } else {
            p->state = PROCESS_STATE_RUNNING;
        }

        OSEL_ENTER_CRITICAL(s);
        process_current = caller;
        OSEL_EXIT_CRITICAL(s);
    }
}

void osel_pthread_init(void)
{
    process_current = NULL;
}

void osel_pthread_poll(osel_task_t *tcb, osel_event_t event)
{
    osel_pthread_t *p = NULL;

    if (event.p == NULL) {
        /* Call the processes that needs to be polled. */
        for (p = tcb->process_list; p != NULL; p = p->next) {
            osel_pthread_call(tcb, (osel_pthread_t *)p, event.sig, event.param);
        }
    } else {
        osel_pthread_call(tcb, event.p, event.sig, event.param);
    }
}
