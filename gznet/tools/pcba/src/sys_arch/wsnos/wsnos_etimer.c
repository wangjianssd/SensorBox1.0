/**
 * @brief       : this
 * @file        : wsnos_ticks.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-05-15
 * change logs  :
 * Date       Version     Author        Note
 * 2015-05-15  v0.0.1  gang.cheng    first version
 * 2015-09-30  v0.0.2  gang.cheng    add pthread support
 */

#include "lib.h"
#include "wsnos.h"

//DBG_THIS_MODULE("wsnos_ticks")


osel_etimer_t time_evt_head;

static uint64_t osel_ticks = 0;

void osel_etimer_ctor(osel_etimer_t *const me, 
                      osel_pthread_t *p,
                      osel_signal_t sig,
                      osel_param_t param)
{
    me->next        = NULL;
    me->ctr         = 0;
    me->interval    = 0;
    
    me->task        = NULL;
    me->p           = p;

    me->super.sig   = sig;
    me->super.param = param;
    me->super.ref_ctr = 0;
}

etimer_ctr osel_etimer_ctr_get(osel_etimer_t *const me)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    etimer_ctr ctr = me->ctr;

    OSEL_EXIT_CRITICAL(status);

    return ctr;
}

void osel_etimer_arm(osel_etimer_t *const me,
                     etimer_ctr const ticks,
                     etimer_ctr const interval)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    me->ctr      = ticks;
    me->interval = interval;
    
//    me->p = PROCESS_CURRENT();
//    DBG_ASSERT(me->p != NULL __DBG_LINE);
    
    if ((me->super.ref_ctr & 0x80) == 0)
    {
        me->super.ref_ctr |= 0x80;

        me->next = (osel_etimer_t *)time_evt_head.act;
        time_evt_head.act = me;
    }

    OSEL_EXIT_CRITICAL(status);
}

/**
 * @brief disarm the time event so it can be safely reused.
 *
 * @param[in] me pointer
 *
 * @return 'true' if the time event wastruly disarmed, that is it was running.
 * the return of 'false' means that the time event was not truly disarmed because
 * it was not running.
 */
osel_bool_t osel_etimer_disarm(osel_etimer_t *const me)
{
    bool_t was_armed = FALSE;

    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    /** the time event is running */
    if (me->ctr != 0)
    {
        was_armed = TRUE;
        me->ctr = 0;
        
        osel_etimer_t *prev = &time_evt_head;       
        for(;;)
        {
            osel_etimer_t *t = prev->next;
            if (t == NULL)
            {
                if (time_evt_head.act != NULL) 
                {
                    prev->next = time_evt_head.act;
                    time_evt_head.act = NULL;
                    t = prev->next;
                }
                else 
                {
                    break;
                }
            }

            if (t == me) 
            {
                prev->next = t->next;
                t->super.ref_ctr &= 0x7F;   /** mark as unlinked */
                break;
            }
            else
            {
                prev = t;
            }
        }
    }
    else
    {
        /** the time event was already not running */
        was_armed = FALSE;
    }

    OSEL_EXIT_CRITICAL(status);

    return was_armed;
}

osel_bool_t osel_etimer_rearm(osel_etimer_t *const me, etimer_ctr const ticks)
{
    bool_t is_armed;

    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    /** the time evt is not running */
    if (me->ctr == 0)
    {
        is_armed = FALSE;

        if ((me->super.ref_ctr * 0x80) == 0)
        {
            me->super.ref_ctr |= 0x80;
            me->next = (osel_etimer_t *)time_evt_head.act;
            time_evt_head.act = me;
        }
    }
    /** the time event is armed */
    else
    {
        is_armed = TRUE;
    }

    me->ctr = ticks;
    me->interval = ticks;

    OSEL_EXIT_CRITICAL(status);
    return is_armed;
}


/*
 * @brief
 * this function must be called periodically from a time-tick ISR or from a task
 * so that QF can manage the timeout events assigned to the given system clock
 * tick rate.
 *
 * @param[in] tick_rate system clock tick rate servied in this call.
 *
 */
void wsnos_ticks(void)
{
    osel_etimer_t *prev = &time_evt_head;

    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    osel_ticks++;

    for (;;)
    {
        osel_etimer_t *t = prev->next;

        if (t == NULL)
        {
            if (time_evt_head.act != NULL)
            {
                prev->next = time_evt_head.act;
                time_evt_head.act = NULL;
                t = prev->next;
            }
            else
            {
                break;
            }
        }

        if (t->ctr == 0)
        {
            prev->next = t->next;
            t->super.ref_ctr &= 0x7F;   /** mark as unlinked */
            OSEL_EXIT_CRITICAL(status);
        }
        else
        {
            --t->ctr;

            if (t->ctr == 0)
            {
//                osel_task_tcb *tcb = (osel_task_tcb *)t->act;

                /** periodic time evt */
                if (t->interval != 0)
                {
                    t->ctr = t->interval;
                    prev = t;
                }
                else     /** one shot time event automatically disarm */
                {
                    prev->next = t->next;
                    t->super.ref_ctr &= 0x7F;
                }

                OSEL_EXIT_CRITICAL(status);
                if(t->super.sig != 0)
                {
                    osel_post(t->task, t->p, &(t->super));
                }
            }
            else
            {
                prev = t;
                OSEL_EXIT_CRITICAL(status);
            }
        }

        OSEL_ENTER_CRITICAL(status);
    }

    OSEL_EXIT_CRITICAL(status);
}

uint64_t osel_ticks_get(void)
{
    return osel_ticks;
}

