/***************************************************************************
* @brief        : this
* @file         : si4463_hal.c
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-10-19
* @change Logs  :
* Date        Version      Author      Notes
* 2015-10-19      v0.1      gang.cheng    first version
***************************************************************************/


#include <msp430.h>
#include <osel_arch.h>
#include <driver.h>

void radio_spi_init(void)
{
    /*NSEL->P2.0 IO pin*/
    /*SDI,SDO,SCLK-->P2.1,P2.2,P2.3 Peripheral module*/
    
    P2SEL |= (BIT1 + BIT2 + BIT3);//*< 1->1b = Peripheral module function is selected;0->IO pin
    pmap_cfg(1, PM_UCB0SIMO);
    pmap_cfg(2, PM_UCB0SOMI);
    pmap_cfg(3, PM_UCB0CLK);
    
    
    P2SEL |= (BIT1 + BIT2 + BIT3);
    P2DIR |= (BIT1 + BIT3);//*< 1->output
    P2DIR &= ~BIT2;

    P5SEL &= ~BIT6;
    P5DIR |= BIT6
        ; 
    UCB0CTL1 |= UCSWRST;    //*< put eUSCI_B in reset state
    UCB0CTL1 |= UCSSEL_2;   //*< SMCLK

    /*! SCLK常态低电平，上升沿采样，MSB在前，8位数据，主模式，3-pin，同步模式. */
    UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
    UCB0BR0 = 2;            //*< SCLK频率是SMCLK / 2
    UCB0BR1 = 0;

    UCB0IE &= ~(UCRXIE + UCTXIE);   //*< stop spi interrupt
    UCB0CTL1 &= ~UCSWRST;

    /*NSEL high level*/
    P2OUT |= BIT0;
}

void radio_spi_deinit(void)
{
    pmap_cfg(1, PM_NONE);
    pmap_cfg(2, PM_NONE);
    pmap_cfg(3, PM_NONE);
    
    P2SEL &= ~(BIT1 + BIT2 + BIT3);  // IO pin
    P2DIR &= ~(BIT1 + BIT2 + BIT3);  // output low
    
    P5SEL &= ~BIT6;
    P5OUT |= BIT6;
    
    UCB0CTL1 |= UCSWRST;    //*< put eUSCI_B in reset state
    UCB0IE &= ~(UCRXIE + UCTXIE);   //*< stop spi interrupt
}

void radio_set_nsel(void)
{
    P5OUT |= BIT6;
}

void radio_clear_nsel(void)
{
    P5OUT &= ~BIT6;
}

uint8_t radio_spi_read_byte(uint8_t data)
{
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data;
    while (!(UCB0IFG & UCRXIFG));
    return UCB0RXBUF;
}

void radio_SpiWriteData(uint8_t len, uint8_t *data)
{
    uint8_t *tx_buf = data;
    
    osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    while (len > 0)
    {
        radio_spi_read_byte(*tx_buf);
        tx_buf++;
        len--;
    }
    OSEL_EXIT_CRITICAL(s);
}

void radio_SpiReadData(uint8_t len, uint8_t *data)
{
    uint8_t *rx_buf = data;
    
    osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    while(len > 0)
    {
        *rx_buf++ = radio_spi_read_byte(0xFF);
        len--;
    }
    OSEL_EXIT_CRITICAL(s);
}


static void radio_shundown_init(void)
{
    //*< TX_SDN p2.7
    P2SEL &= ~BIT7;
    P2DIR |= BIT7;   //1->output

    P2OUT |= BIT7;  //SDN High
}

void radio_AssertShutdown(void)
{
    uint16_t tick = 0;

    /*TX_SDN p2.7*/
    P2OUT |= BIT7;/*set*/
    for (tick = 128; tick > 0; tick-- )
    {
        ;
    }
}

void radio_DeassertShutdown(void)
{
    uint16_t tick = 0;
    P2OUT &= ~BIT7;/*clear*/
    for (tick = 128; tick > 0; tick-- )
    {
        ;
    }
}



static void radio_4259_init(void)
{
    /*RX->5.2;TX->5.3*/
    P5SEL &= ~(BIT2 + BIT3);
    P5DIR |= (BIT2 + BIT3);/*output*/
}

void radio_4259_tx(void)
{
    /*RX:P5.2=1,TX:P5.3=0*/
    P5OUT &= ~(BIT3);
    P5OUT |= (BIT2);
}

void radio_4259_rx(void)
{
    P5OUT &= ~(BIT2);
    P5OUT |= (BIT3);
}

void radio_4259_idle(void)
{
    P5SEL &= ~(BIT2 + BIT3);
    P5DIR |= (BIT2 + BIT3);/*output*/
    P5OUT &= ~(BIT2 | BIT3);
}

static void radio_unb_init(void)
{
    //VMDD VDD contrl p9.7
    P9SEL &= ~BIT7;
    P9DIR |= BIT7;
}

void radio_unb_on(void)
{
    //VMDD VDD contrl p9.7
    P9SEL &= ~BIT7;
    P9DIR |= BIT7;
    P9OUT |= BIT7;
}

void radio_unb_off(void)
{
    //VMDD VDD contrl p9.7
    P9SEL &= ~BIT7;
    P9DIR |= BIT7;
    P9OUT &= ~BIT7;
}

void radio_port_init(void)
{
    radio_shundown_init();  // P2OUT |= BIT7;  //SDN High    
    radio_4259_init();
    radio_unb_init();
}



void radio_nirq_init(void)
{
    //nIRQ p1.2
    P2SEL &= ~BIT0;//0->IO pin
    P2DIR &= ~BIT0;//0->input

    P2IES |= BIT0;       // P1IFG flag is set with a high-to-low transition.下降沿触发, ;
    P2IFG &= ~BIT0;     //P1IES的切换可能使P1IFG置位，需清除
    
    P2IE |= BIT0;        // Corresponding port interrupt enabled
}

bool_t radio_get_NirqLevel(void)
{
    return (P2IN & BIT0) ? 1 : 0;
}

void radio_nirq_enable(void)
{
    P2IE |= BIT0;       // Corresponding port interrupt enabled
    P2IFG &= ~BIT0;     //P1IES的切换可能使P1IFG置位，需清除
}
void radio_nirq_disable(void)
{
    P2IE &= ~BIT0;      // Corresponding port interrupt disable
    P2IFG &= ~BIT0;     //P1IES的切换可能使P1IFG置位，需清除
}

void radio_nirq_deinit(void)
{
    P2IE &= ~BIT0;      // Corresponding port interrupt disable
    P2IFG &= ~BIT0;     //P1IES的切换可能使P1IFG置位，需清除
    
    P2SEL &= ~BIT0;//0->IO pin
    P2DIR |= BIT0;//0->input
    P2OUT &= ~BIT0;
}

#pragma vector = PORT2_VECTOR
__interrupt void port2_int_handle(void)
{
    OSEL_ISR_ENTRY();
    
    if((P2IFG & BIT0) == BIT0)
    {
        P2IFG &= ~BIT0;
        extern void rf_int_handler(uint16_t time);
        rf_int_handler(TA0R);
    }
    
    OSEL_ISR_EXIT();
}
