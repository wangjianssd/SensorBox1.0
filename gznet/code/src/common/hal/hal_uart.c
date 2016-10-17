/**
 * @brief       : 
 *
 * @file        : hal_uart.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/hal/hal.h"
#include "platform/platform.h"


//int putchar(int ch)
//{
//    hal_uart_send_char(HAL_UART_1, ch);
//    return ch;
//}

void hal_uart_init(uint8_t uart_id, uint32_t baud_rate)
{
    uart_init(uart_id, baud_rate);
}

void hal_uart_send_char(uint8_t uart_id, uint8_t value)
{
    uart_send_char(uart_id, value);
}

void hal_uart_send_string(uint8_t uart_id,
                          uint8_t *const string,
                          uint16_t length)
{
    uart_send_string(uart_id, string, length);
}

bool_t hal_uart_enter_q(uint8_t hal_uart_id, uint8_t c)
{
    return uart_enter_q(hal_uart_id, c);
}

bool_t hal_uart_string_enter_q(uint8_t hal_uart_id,
                               uint8_t *const string,
                               uint16_t length)
{
    return uart_string_enter_q(hal_uart_id, string, length);
}

bool_t hal_uart_del_q(uint8_t hal_uart_id, uint8_t *c_p)
{
    return uart_del_q(hal_uart_id, c_p);
}

void hal_uart_clear_rxbuf(uint8_t id)
{
    uart_clear_rxbuf(id);
}

void hal_uart_recv_enable(uint8_t uart_id)
{
    uart_recv_enable(uart_id);
}

void hal_uart_recv_disable(uint8_t uart_id)
{
    uart_recv_disable(uart_id);
}

void hal_uart_int_cb_reg(uart_interupt_cb_t cb)
{
    uart_int_cb_reg(cb);
}
