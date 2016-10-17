#include <gznet.h>
//#include <osel_arch.h>
//#include <debug.h>
//#include <sbuf.h>
//#include <app_prim.h>
#include <ssn.h>
//#include <hal.h>
//#include <app_handles.h>
//#include <app_cmd.h>
//#include <app_frames.h>

static ssn_cb_t ssn_func_cb;

static hal_timer_t *ssn_open_cb_timer = NULL;
static hal_timer_t *ssn_close_cb_timer = NULL;

bool_t app_get_inline_status()
{
    return FALSE;
}
void ssn_config(ssn_cb_t *ssn_cb)
{
    DBG_ASSERT(NULL != ssn_cb __DBG_LINE);
    if (NULL != ssn_cb->open_cb)
    {
        ssn_func_cb.open_cb = ssn_cb->open_cb;
    }

    if (NULL != ssn_cb->close_cb)
    {
        ssn_func_cb.close_cb = ssn_cb->close_cb;
    }

    if (NULL != ssn_cb->rx_cb)
    {
        ssn_func_cb.rx_cb = ssn_cb->rx_cb;
    }

    if (NULL != ssn_cb->tx_cb)
    {
        ssn_func_cb.tx_cb = ssn_cb->tx_cb;
    }
}

#if 0
static void ssn_open_timeout_cb(void *p)
{
//    ssn_open_cb_timer = NULL;
//    if (!app_get_inline_status())
//    {
//        if ( NULL != ssn_func_cb.open_cb)
//        {
//            ssn_func_cb.open_cb(FALSE);
//        }
//    }
//
//    osel_post(M_MAC_MONI_STOP_EVENT, NULL, OSEL_EVENT_PRIO_LOW); 
    
    // in this app, just open once.so donot take off.
    //osel_post(NWK_INLINE_EVENT, (uint32_t *)(FALSE), OSEL_EVENT_PRIO_LOW);
    
    // no used osel_post, for ssn_close_handle will be trigger
//    app_set_inline_status(FALSE); 
}
#endif
uint8_t ssn_get_status(void)
{
    return FALSE;
//    return app_get_inline_status(); 
}
#if 0
void ssn_open(ssn_open_cb_t open_cb, uint32_t sec)
{
//    if ((open_cb != NULL) && (ssn_func_cb.open_cb != open_cb))
//    {
//        ssn_func_cb.open_cb = open_cb;
//    }
//    
//    if(app_get_inline_status())
//    {
//        ssn_func_cb.open_cb(TRUE);
//        return;
//    }
//    
//    if (sec == 0)   // wait for ever
//    {
//        ;
//    }
//    else
//    {
//        if ( NULL == ssn_open_cb_timer)
//        {
//            HAL_TIMER_SET_REL(  MS_TO_TICK(sec * 1000),
//                                ssn_open_timeout_cb,
//                                NULL,
//                                ssn_open_cb_timer);
//            DBG_ASSERT(ssn_open_cb_timer != NULL __DBG_LINE);
//        }
//    }
//    
//    osel_post(M_MAC_MONI_START_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
}
#endif

#if 0
static void ssn_close_timeout_cb(void *p)
{
    ssn_close_cb_timer = NULL;
    if (app_get_inline_status())
    {
        if ( NULL != ssn_func_cb.close_cb)
        {
            ssn_func_cb.close_cb(FALSE);
        }
    }
}
#endif

void ssn_close(ssn_close_cb_t close_cb, uint32_t sec)
{
//    if ((close_cb != NULL) && (ssn_func_cb.close_cb != close_cb))
//    {
//        ssn_func_cb.close_cb = close_cb;
//    }
//    
//    if(!app_get_inline_status())
//    {
//        ssn_func_cb.close_cb(TRUE);
//        return;
//    }
//    
//    if (sec == 0)   // wait for ever
//    {
//        ;
//    }
//    else
//    {
//        if ( NULL == ssn_close_cb_timer)
//        {
//            HAL_TIMER_SET_REL(  MS_TO_TICK(sec * 1000),
//                                ssn_close_timeout_cb,
//                                NULL,
//                                ssn_close_cb_timer);
//            DBG_ASSERT(ssn_close_cb_timer != NULL __DBG_LINE);
//        }
//    }
//    
//    osel_post(APP_INLINE_EVENT, (uint32_t *)(FALSE), OSEL_EVENT_PRIO_LOW);
//    osel_post(NWK_INLINE_EVENT, (uint32_t *)(FALSE), OSEL_EVENT_PRIO_LOW);
//    osel_post(M_MAC_MONI_STOP_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
}

int8_t ssn_send(ssn_tx_cb_t tx_cb, uint8_t *data, uint8_t len, uint8_t mode)
{
    if (tx_cb == NULL)
    {
        return -1;
    }

    if (ssn_func_cb.tx_cb != tx_cb)
    {
        ssn_func_cb.tx_cb = tx_cb;
    }

    // TODO:
//    if ((len > APP_CMD_SIZE_MAX) || (data == NULL))
//    {
//        return -2;
//    }
//    app_send_data(data, len, APP_TYPE_DATA_USER, mode);

    return 0;
}

/* for user */
void ssn_open_handle(bool_t res)
{
    if ( NULL != ssn_func_cb.open_cb)
    {
        if (NULL != ssn_open_cb_timer)
        {
            hal_timer_cancel(&ssn_open_cb_timer);
        }
        ssn_func_cb.open_cb(res);
    }
}

void ssn_close_handle(bool_t res)
{
    if ( NULL != ssn_func_cb.close_cb)
    {
        if (NULL != ssn_close_cb_timer)
        {
            hal_timer_cancel(&ssn_close_cb_timer);
        }
        
        ssn_func_cb.close_cb(res);
    }
}

void ssn_read_handle(uint8_t *data, uint8_t len)
{
    if ( NULL != ssn_func_cb.rx_cb)
    {
        ssn_func_cb.rx_cb(data, len);
    }
}

void ssn_send_handle(uint8_t *data, uint8_t len, bool_t res, uint8_t mode)
{
    if ( NULL != ssn_func_cb.tx_cb)
    {
        ssn_func_cb.tx_cb(data, len, res, mode);
    }
}


