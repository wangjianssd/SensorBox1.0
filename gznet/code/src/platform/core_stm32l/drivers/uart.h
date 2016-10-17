/**
 * @brief       : 
 *
 * @file        : uart.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/7/2
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/7/2    v0.0.1      gang.cheng    first version
 */
#ifndef __UART_H__
#define __UART_H__

#include "common/lib/lib.h"

typedef enum 
{
    UART_1,                         /**< 串口1 */
    UART_2,                         /**< 串口2 */
    UART_3,                         /**< 串口3 */
    UART_4,                         /**< 串口4 */
} uart_id_t;

typedef void (*uart_interupt_cb_t)(uint8_t id, uint8_t rxbuf_val);


void uart_init(uint8_t uart_id, uint32_t baud_rate);

void uart_send_char(uint8_t id, uint8_t value);

void uart_send_string(uint8_t id, uint8_t *string, uint16_t length);

bool_t uart_enter_q(uint8_t id, uint8_t c);

bool_t uart_string_enter_q(uint8_t uart_id, uint8_t *string, uint16_t length);

bool_t uart_del_q(uint8_t uart_id, uint8_t *c_p);

void uart_clear_rxbuf(uint8_t id);

void uart_recv_enable(uint8_t uart_id);

void uart_recv_disable(uint8_t uart_id);

void uart_int_cb_reg(uart_interupt_cb_t cb);



#endif