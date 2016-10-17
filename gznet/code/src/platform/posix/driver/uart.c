/**
 * @brief       : this
 * @file        : uart.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-10
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-10  v0.0.1  gang.cheng    first version
 */	
#include "sys_arch/osel_arch.h"
#include "driver.h"

static uart_interupt_cb_t uart_interrupt_cb = NULL;

void uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	;
}

void uart_send_char(uint8_t id, uint8_t value)
{
	printf("%d", value);
}

void uart_send_string(uint8_t id, uint8_t *string, uint16_t length)
{
	printf("uart %d:", id);
	for(uint16_t i=0;i<length;i++)
	{
		uart_send_char(id, string[i]);
	}
}

bool_t uart_enter_q(uint8_t id, uint8_t c)
{
    return TRUE;
}

bool_t uart_string_enter_q(uint8_t id, uint8_t *string, uint16_t length)
{
    bool_t ret = FALSE;

    return ret;
}

bool_t uart_del_q(uint8_t id, uint8_t *c_p)
{
    return FALSE;
}

void uart_clear_rxbuf(uint8_t id)
{
    ;
}


void uart_recv_enable(uint8_t uart_id)
{
    ;
}

void uart_recv_disable(uint8_t uart_id)
{
    ;
}


void uart_int_cb_reg(uart_interupt_cb_t cb)
{
    if (cb != NULL)
    {
        uart_interrupt_cb = cb;
    }
}

static void uart_int_cb_handle(uint8_t id, uint8_t ch)
{
    if (uart_interrupt_cb != NULL)
    {
        uart_interrupt_cb(id, ch);
    }
}
