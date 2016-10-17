/**
 * @defgroup STACK STACK - SenStack WSN Protocol Stack
 *
 * @file ssn.h
 *
 * This is the main include file of STACK module for app user, one should
 * include this file if intend to use STACK functions.
 */
#ifndef __SSN_H
#define __SSN_H

#include <gznet.h>
//#include <stack.h>
//#include <data_type_def.h>

#define ssn_sbuf_t          sbuf_t
#define ssn_pbuf_t          pbuf_t

typedef enum
{
    UNRELIABLE_MODE,
    RELIABLE_MODE
} link_mode_t;

typedef void (*ssn_open_cb_t)(bool_t res);
typedef void (*ssn_close_cb_t)(bool_t res);
/**
 * [void  : you should copy data in the func]
 * @param  data [pointer to receive data, the data will free after func end]
 * @param  len  [len of receive data]
 * @return      [void]
 */
typedef void (*ssn_rx_cb_t)(uint8_t *data, uint8_t len);
typedef void (*ssn_tx_cb_t)(uint8_t *data, uint8_t len, uint8_t res, uint8_t mode);

typedef struct
{
    ssn_open_cb_t   open_cb;
    ssn_close_cb_t  close_cb;
    ssn_rx_cb_t     rx_cb;
    ssn_tx_cb_t     tx_cb;
} ssn_cb_t;


/** for user **/
/**
 * [ssn_config : config ssn API callback]
 * @param ssn_cb [the instantiation of ssn newwork interface]
 */
void ssn_config(ssn_cb_t *ssn_cb);

/**
 * [ssn_open : open the ssn network]
 * @param open_cb [the callback of open result]
 * @param sec     [timeout for open handle]
 */
void ssn_open(ssn_open_cb_t open_cb, uint32_t sec);

/**
 * [ssn_close : open the ssn network]
 * @param close_cb [the callback of close result]
 * @param sec      [timeout for close handle]
 */
void ssn_close(ssn_close_cb_t close_cb, uint32_t sec);

/**
 * [ssn_send : send data from ssn network]
 * @param  tx_cb [the callback of send result]
 * @param  data  [pointer to data to be transmitted]
 * @param  len   [len of data to be transmitted]
 * @return       [result of send:
                    0  -- ok
                    -1 -- tx_cb == NULL
                    -2 -- len over max or data pointer NULL
                 ]
 */
int8_t ssn_send(ssn_tx_cb_t tx_cb, uint8_t *data, uint8_t len, uint8_t mode);

uint8_t ssn_get_status(void);

/** for inter of ssn used **/
void ssn_open_handle(bool_t res);

void ssn_close_handle(bool_t res);

void ssn_read_handle(uint8_t *data, uint8_t len);

void ssn_send_handle(uint8_t *data, uint8_t len, bool_t res, uint8_t mode);
/**
 * @}
 */
#endif
