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
    UCB0CTL1 = UCSWRST;
	/*! SCLK常态低电平，上升沿采样，MSB在前，8位数据，主模式，3-pin，同步模式. */
	UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCB0CTL1 |= UCSSEL_2; ///  SMCLK
	UCB0BR0 = 2;
	UCB0BR1 = 0;	///  SCLK频率是SMCLK / 2
	UCB0IE &= ~(UCRXIE + UCTXIE);	  ///  禁止SPI中断
	UCB0CTL1 &= ~UCSWRST;
	P3SEL |= (BIT1 + BIT2 + BIT3);
	P3SEL &= ~BIT0;						//STE
	P3DIR |= (BIT0 + BIT1 + BIT3);
	P3DIR &= ~BIT2;
	P3OUT |= BIT0;
}

void radio_port_init(void)
{   
    //SLP
	P2SEL &= ~BIT4;
	P2DIR |= BIT4;
	//GPIO_2
	P2SEL &= ~BIT5;
	P2DIR &= ~BIT5;
	//nIRQ
	P8SEL |= BIT1;
	P8DIR &= ~BIT1;	//nIRQ captured by TA0CCR1
	TA0CCTL1 = CAP + CM_2 + CCIS_1 + SCS;
}

void radio_powerup(void)
{
    /* 拉低PWRDN(SDN)脚，使能RF */
    P2OUT &= ~BIT4;
}








