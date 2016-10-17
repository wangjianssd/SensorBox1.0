/**
 * @brief       : this
 * @file        : uart.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-14
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-14  v0.0.1  gang.cheng    first version
 */
#ifndef __UART_H__
#define __UART_H__

#include <lib.h>

/* uart id define */
#define UART_1                      0
#define UART_2                      1
#define UART_3                      2

typedef void (*uart_interupt_cb_t)(uint8_t id, uint8_t ch);

/**
 * Initializes the serial communications peripheral and GPIO ports
 * to communicate with the peripheral device.
 *
 * @param  uart_id which uart should be operated
 * @param  baud_rate uart baud rate min.300 and max.115200
 */
void uart_init(uint8_t uart_id, uint32_t baud_rate);


/**
 * register uart id to task
 *
 * @param uart_id which uart to be registered
 * @param task_id which task to be registered
 */
void uart_send_char(uint8_t id, uint8_t value);

/**
 * Send a string by uart
 *
 * @param id  which uart should be operated
 * @param *string Pointer to a string containing the data to be sended
 * @param length Max value is 255
 */
void uart_send_string(uint8_t id, uint8_t *string, uint16_t length);

bool_t uart_enter_q(uint8_t id, uint8_t c);

bool_t uart_string_enter_q(uint8_t uart_id, uint8_t *string, uint16_t length);

bool_t uart_del_q(uint8_t uart_id, uint8_t *c_p);

void uart_clear_rxbuf(uint8_t id);

void uart_recv_enable(uint8_t uart_id);

void uart_recv_disable(uint8_t uart_id);

void uart_int_cb_reg(uart_interupt_cb_t cb);

#endif
