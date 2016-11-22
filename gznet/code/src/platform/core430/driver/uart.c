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
#include "pmap.h"
#include <gprs_tx.h>
#include "blu_tx.h"
#include "gps.h"
#include "fifo.h"
#include "bsp_com.h"
#include "blu_tx.h"

extern uint8_t COM1RxFIFO[__COM1_RX_FIFO_SIZE__ + FIFO_INFO_SIZE];

static uart_interupt_cb_t uart_interrupt_cb = NULL;

void uart_init(uint8_t uart_id, uint32_t baud_rate)
{
    fp32_t baud = 0;
    if(baud_rate <= 9600)
    {
        baud = (fp32_t)ACLK / baud_rate;
    }
    else
    {
        baud = (fp32_t)SMCLK / baud_rate;
    }
    uint8_t br0 = (uint16_t)baud & 0xFF;
    uint8_t br1 = ((uint16_t)baud >> 8) & 0xFF; // 37 us
    uint8_t fract = (uint8_t)((baud - (uint16_t)baud) * 8 + 0.5); // 104us

    switch (uart_id)
    {
    case UART_1:
        /* Initialize uart 1 IO */
        P2SEL |= BIT4 + BIT5;

        pmap_cfg(4, PM_UCA0TXD);
        pmap_cfg(5, PM_UCA0RXD);

        UCA0CTL1 = UCSWRST;
        UCA0CTL0 = UCMODE_0;
        if(baud_rate <= 9600)
        {
            UCA0CTL1 |= UCSSEL__ACLK;			/* ACLK,32.768K */
        }
        else
        {
            UCA0CTL1 |= UCSSEL_2;
        }
        UCA0BR0 = br0;
        UCA0BR1 = br1;
        UCA0MCTL = UCBRF_0 | (fract << 1);
        UCA0CTL1 &= ~UCSWRST;
        UCA0IE |= UCRXIE;
        break;

    case UART_2:
        /* Initialize uart 1 IO */
        P9SEL |= BIT2 + BIT3;
         P9DIR |= BIT2;
         P9DIR &= ~BIT3;
        
        UCA2CTL1 = UCSWRST;
        UCA2CTL0 = UCMODE_0;
		if(baud_rate <= 9600)
        {
            UCA2CTL1 |= UCSSEL__ACLK;			/* ACLK,32.768K */
        }
        else
        {
            UCA2CTL1 |= UCSSEL_2;
        }
        UCA2BR0 = br0;
        UCA2BR1 = br1;
        UCA2MCTL = UCBRF_0 | (fract << 1);
        UCA2CTL1 &= ~UCSWRST;
        UCA2IE |= UCRXIE;
        break;

    case UART_3:
        P8SEL |= BIT2 + BIT3;
        P8DIR |= BIT2;
        P8DIR &= ~BIT3;

        UCA1CTL1 = UCSWRST;
        UCA1CTL0 = UCMODE_0;

        if(baud_rate <= 9600)
        {
            UCA1CTL1 |= UCSSEL__ACLK;			/* ACLK,32.768K */
        }
        else
        {
            UCA1CTL1 |= UCSSEL_2;
        }
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
        UCA2TXBUF = value;
        while (UCA2STAT & UCBUSY);
    }
    else if (id == UART_3)
    {
        UCA1TXBUF = value;
        while (UCA1STAT & UCBUSY);
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
        UCA2IE |= UCRXIE;
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
        UCA2IE &= ~UCRXIE;
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

/******************************************************************************/
extern bool_t blu_if_connected;

#pragma vector = USCI_A0_VECTOR
__interrupt void uart0_rx_isr(void)
{
    OSEL_ISR_ENTRY();
    //uart_int_cb_handle(UART_1, UCA0RXBUF);
    uint8_t ch = UCA0RXBUF;
    extern sqqueue_ctrl_t blu_auto_recv_sqq;
    blu_auto_recv_sqq.enter(&blu_auto_recv_sqq,(void *)&ch);    

	extern blu_cmd_type_e blu_cmd_type;
	blu_cmd_type = BLU_RECV_DATA;
	blu_if_connected = TRUE;
	
    clk_ccrx_restart(5, BLU_UART_EVENT);
		
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void uart1_rx_isr(void)
{
    OSEL_ISR_ENTRY();
//    uart_int_cb_handle(UART_3, UCA1RXBUF);
    uint8_t ch = UCA1RXBUF;
    extern sqqueue_ctrl_t auto_recv_sqq;
    auto_recv_sqq.enter(&auto_recv_sqq,(void *)&ch);    
    
    clk_ccrx_restart(1, GPRS_UART_EVENT);
    
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}

extern void BspCom1RxHander( uint8_t* data, uint16_t size );

#pragma vector = USCI_A2_VECTOR
#if USE_RFID_READER > 0
__interrupt void uart2_rx_isr(void)
{
    uint8_t data;
    OSEL_ISR_ENTRY();
    data = UCA2RXBUF;
    FIFOIn((FIFODataTypeDef *)COM1RxFIFO, &data);

    OSEL_ISR_EXIT();
    LPM3_EXIT;
}
#else
__interrupt void uart2_rx_isr(void)
{
    OSEL_ISR_ENTRY();
    uart_int_cb_handle(UART_2, UCA2RXBUF);
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}

#endif


