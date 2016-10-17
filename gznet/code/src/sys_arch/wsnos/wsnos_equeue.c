/**
 * @brief       : this
 * @file        : wsnos_equeue.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-09-26
 * change logs  :
 * Date       Version     Author        Note
 * 2015-09-26  v0.0.1  gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"

void osel_equeue_init(osel_equeue_t *const this, osel_event_t const event_sto[],
                      osel_uint16_t const event_len)
{
    this->ring      = &event_sto[0];
    this->end       = event_len;
    this->head      = (equeue_ctr_t)0;
    this->tail      = (equeue_ctr_t)0;
    this->n_free    = (equeue_ctr_t)event_len;
    this->n_min     = this->n_free;
}

osel_bool_t osel_equeue_post(osel_equeue_t *const this,
                            osel_event_t const *const event)
{
    equeue_ctr_t n_free = 0;
    osel_int_status_t s = 0;
    osel_task_t *task = NULL;

    OSEL_ENTER_CRITICAL(s);
    n_free = this->n_free;
    task = list_entry_addr_find(this, osel_task_t, equeue);

    if (n_free > 0)
    {
        --n_free;                   /* one free entry just used up */
        this->n_free = n_free;      /* update the volatile */

        if (this->n_min > n_free)
        {
            this->n_min = n_free;   /* update minimum so far */
        }

        osel_rdy_grp |= osel_map_tbl[(task->prio) >> 3];
        osel_rdy_tbl[(task->prio) >> 3] |= osel_map_tbl[(task->prio) & 0x07];
    
        /* the  event insert to ring buffer */
        osel_memcpy((osel_uint8_t *)&(this->ring[this->head++]), 
                        event, sizeof(osel_event_t));
        if(this->head == this->end)
        {
            this->head = (equeue_ctr_t)0;
        }
        OSEL_EXIT_CRITICAL(s);
    }
    else
    {
        OSEL_EXIT_CRITICAL(s);
        return FALSE;
    }

    OSEL_POST_SIGNAL(this);   /* signal the event queue */

    return TRUE;
}

osel_bool_t osel_equeue_get(osel_equeue_t *const this, osel_event_t *event)
{
    equeue_ctr_t n_free = 0;
    osel_bool_t status = FALSE;
    osel_int_status_t s = 0;

    OSEL_ENTER_CRITICAL(s);

    OSEL_POST_WAIT(this);     /* wait for event to arrive directly */
    
    if(this->n_free < this->end)
    {
        /* always remove event from the front location */    
        n_free = this->n_free + 1;
        this->n_free = n_free;
        
        osel_memcpy(event, 
                    (osel_uint8_t *)&(this->ring[this->tail++]),
                    sizeof(osel_event_t));
        if(this->tail == this->end)
        {
            this->tail = (equeue_ctr_t)0;
        }
        
        status = TRUE;
    }
    else
    {
        status = FALSE; //*< squeue is empty
    }

    OSEL_EXIT_CRITICAL(s);

    return status;
}



