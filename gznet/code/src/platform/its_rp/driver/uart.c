/**
 * @brief       : 
 *
 * @file        : uart.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "sys_arch/wsnos/wsnos.h"

static uart_interupt_cb_t uart_interrupt_cb = NULL;

void uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	fp32_t baud = (fp32_t)ACLK/baud_rate;
//#if ((NODE_TYPE == NODE_TYPE_TAG) || (NODE_TYPE == NODE_TYPE_ROUTER))
//	fp32_t baud = (fp32_t)ACLK/baud_rate;
//#else
//    fp32_t baud = (fp32_t)SMCLK/baud_rate;
//#endif
	uint8_t br0 = (uint16_t)baud&0xFF;
	uint8_t br1 = ((uint16_t)baud>>8)&0xFF;		// 37 us
	uint8_t fract = (uint8_t)((baud-(uint16_t)baud) * 8 + 0.5);	// 104us
    
	switch (uart_id)
	{
	case UART_1:
		/* Initialize uart 1 IO */
		P3SEL |= BIT4 + BIT5;
		P3DIR |= BIT4;
		P3DIR &= ~BIT5;
        
		UCA0CTL1 = UCSWRST;
		UCA0CTL0 = UCMODE_0;
		UCA0CTL1 |= UCSSEL__ACLK;
//#if ((NODE_TYPE == NODE_TYPE_TAG) || (NODE_TYPE == NODE_TYPE_ROUTER))
//		UCA0CTL1 |= UCSSEL__ACLK;			/* ACLK,32.768K */
//#else
//        UCA0CTL1 |= UCSSEL_2;				/* SMCLK,8M */
//#endif
		UCA0BR0 = br0;
		UCA0BR1 = br1;
		UCA0MCTL = UCBRF_0 | (fract<<1);
		UCA0CTL1 &= ~UCSWRST;
		UCA0IE |= UCRXIE;
		break;
        
	case UART_2:
        /* max3221 init */
        
        P10SEL |= BIT4 + BIT5;
		P10DIR |= BIT4;
		P10DIR &= ~BIT5;
        
		UCA3CTL1 = UCSWRST;
		UCA3CTL0 = UCMODE_0;
		UCA3CTL1 |= UCSSEL_2;				 /* SMCLK,8M */
		UCA3BR0 = br0;
		UCA3BR1 = br1;
		UCA3MCTL = UCBRF_0 | (fract<<1);
		UCA3CTL1 &= ~UCSWRST;
		UCA3IE |= UCRXIE;
		break;
        
    case UART_3:
        /* Initialize uart 1 IO */
        P5SEL |= BIT6 + BIT7;
        P5DIR |= BIT6;      // TXD_TO_485
        P5DIR &= ~BIT7;     // RXD_FROM_485
        
        P10SEL &= ~(BIT6 + BIT7);
        P10DIR |= BIT6 + BIT7;
        P10OUT &= ~(BIT6+BIT7);
        
        UCA1CTL1 = UCSWRST;
        UCA1CTL0 = UCMODE_0;
        UCA1CTL1 |= UCSSEL_2;                /* SMCLK,8M */
        UCA1BR0 = br0;
        UCA1BR1 = br1;
        UCA1MCTL = UCBRF_0 | (fract << 1);
        UCA1CTL1 &= ~UCSWRST;
        UCA1IE |= UCRXIE;
        break;
	default:
		break;
	}
}


void uart_send_char(uint8_t id, uint8_t value)
{
    if (id == UART_1)
	{
        UCA0TXBUF = value;
		while (UCA0STAT & UCBUSY);
	}
	else if (id == UART_2)
	{
		UCA3TXBUF = value;
		while (UCA3STAT & UCBUSY);
	}
    else if(id == UART_3)
    {
        P10OUT |= (BIT6+BIT7);
        UCA1TXBUF = value;
		while (UCA1STAT & UCBUSY);
        P10OUT &= ~(BIT6+BIT7);
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


void uart_send_string(uint8_t id, uint8_t *string, uint16_t length)
{
    uint16_t i = 0;
    for (i = 0; i < length; i++)
    {
        uart_send_char(id, string[i]);
    }
}

void uart_clear_rxbuf(uint8_t id)
{
    ;
}

void uart_recv_enable(uint8_t uart_id)
{
    if (uart_id == UART_1)
    {
        UCA0IE |= UCRXIE;
    }
    else if (uart_id == UART_2)
    {
        UCA1IE |= UCRXIE;
    }
    else if (uart_id == UART_3)
    {
        UCA1IE |= UCRXIE;
    }
}

void uart_recv_disable(uint8_t uart_id)
{
    if (uart_id == UART_1)
    {
        UCA0IE &= ~UCRXIE;
    }
    else if (uart_id == UART_2)
    {
        UCA1IE &= ~UCRXIE;
    }
    else if (uart_id == UART_3)
    {
        UCA1IE &= ~UCRXIE;
    }
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


#pragma vector = USCI_A3_VECTOR
__interrupt void uart3_rx_isr(void)
{
	OSEL_ISR_ENTRY();
	uart_int_cb_handle(UART_2, UCA3RXBUF);
	OSEL_ISR_EXIT();
	LPM3_EXIT;
}


// 485
#pragma vector = USCI_A1_VECTOR
__interrupt void uart1_rx_isr(void)
{
    OSEL_ISR_ENTRY();
    uart_int_cb_handle(UART_3, UCA1RXBUF);
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}

