/**
 * @brief       : 
 *
 * @file        : si4432_arch.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/11/3
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/11/3    v0.0.1      gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"

void radio_spi_init(void)
{
    /* Put state machine in reset for reconfiguration */
    UCB0CTL1 |= UCSWRST;
	
	/* The inactive state is high, MSB first, 8-bit data,
     * Master mode, 3-pin SPI, Synchronous mode
     */	
    UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCB0CTL1 |= UCSSEL_2; ///  SMCLK
	UCB0BR0 = 0x02;
	UCB0BR1 = 0x00;					    // fBitClock = fBRCLK/UCBRx = SMCLK/2
	
	UCB0CTL1 &= ~UCSWRST;			    // Initialize USCI state machine
	UCB0IE &= ~(UCRXIE + UCTXIE);       // Interrupt disabled

    /* P3.1 P3.2 P3.2 as Peripheral module function, P9.0 as GPIO */
	P3SEL |= (BIT1 + BIT2 + BIT3);
	P3SEL &= ~BIT0;						//STE
	P3DIR |= (BIT0 + BIT1 + BIT3);
	P3DIR &= ~BIT2;
	P3OUT |= BIT0;
}

void radio_port_init(void)
{   
    /* P2.5 <-> RF_GDO2 */
    P2SEL &= ~BIT5;
	P2DIR &= ~BIT5;	
    
    /* P8.1 <-> Timer_A0_CCR1 <-> nIRQ */ 
    P8SEL |= BIT1;
	P8DIR &= ~BIT1;	//nIRQ captured by TA0CCR1
    /* 捕获模式，下降沿触发， CCIxB(P8口)， 捕获同步模式 */
    TA0CCTL1 = CM_2 + CCIS_1 + SCS + CAP; ///  下降沿capture，CCI3B，同步

    /* P8.4 <-> SDN */
    P2SEL &= ~BIT4;
	P2DIR |= BIT4;
}

void radio_powerup(void)
{
    /* 拉低PWRDN(SDN)脚，使能RF */
    P2OUT &= ~BIT4;
}








