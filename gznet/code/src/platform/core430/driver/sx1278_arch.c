/**
 * @brief       : this
 * @file        : sx1278_arch.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-14
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-14  v0.0.1  gang.cheng    first version
 */
#include "sx127x_arch.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"



void sx127x_reset(void)
{
	// P1.5
	//set reset pin to 0
	P1SEL &= ~BIT0;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	delay_ms(1);

	//set reset pin as input
	P1DIR &= ~BIT0;

	delya_ms(6);
}

void sx127x_spi_init(void)
{
	UCA1CTL1 = UCSWRST;
	/*! SCLK常态低电平，上升沿采样，MSB在前，8位数据，主模式，3-pin，同步模式. */
	UCA1CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCA1CTL1 |= UCSSEL_2; 				///  SMCLK
	UCA1BR0 = 2;
	UCA1BR1 = 0;						///  SCLK频率是SMCLK / 2
	UCA1IE &= ~(UCRXIE + UCTXIE);	  	///  禁止SPI中断
	UCA1CTL1 &= ~UCSWRST;

	P8SEL |= (BIT1 + BIT2 + BIT3);		
	P8DIR |= (BIT1 + BIT3);
	P8DIR &= ~BIT2;

	P8SEL &= ~BIT4;						//STE--CS
	P8DIR |= BIT4;
	P8OUT |= BIT4;
}


void sx127x_port_init(void)
{

}


uint8_t sx127x_spi_write_read(uint8_t val)
{
	SX127X_SPI_SEND_CHAR(val);

	return SX127X_SPI_RECEIVE_CHAR();
}


void sx127x_ant_sw_init(void)
{
	;
}

void sx127x_ant_sw_deinit(void)
{
	;
}

void sx127x_ant_sw_set(uint8_t rx_tx)
{
	if(rx_tx == 1)	//1: tx, 0: rx
	{
		;
	}
	else
	{
		;
	}
}

