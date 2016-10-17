/**
 * @brief       : \
 *
 * @file        : si4432_arch.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/11/3
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/11/3    v0.0.1      gang.cheng    first version
 */
#ifndef __SI4432_ARCH__
#define __SI4432_ARCH__

/** 清除中断标志 */
#define MCU_RADIO_INT_CLR()         (TA0CCTL1 &= ~CCIFG)

#define MCU_RADIO_INT_ENABLE()      (TA0CCTL1 |= CCIE)

#define MCU_RADIO_INT_GET()         while((P8IN & BIT1)== BIT1)

#define RF_SPI_BEGIN()                              \
    do                                              \
    {                                               \
        P3OUT &= ~BIT0;                             \
        while((P3IN & BIT2));                       \
    } while(__LINE__ == -1)             

#define RF_SPI_END() (P3OUT |=  BIT0)     

#define RF_SPI_RECEIVE_CHAR()	(UCB0RXBUF)
        
#define SPI_SEND_CHAR(x)                            \
    do                                              \
    {                                               \
		WHILE (!(UCB0IFG&UCTXIFG));					\
        UCB0TXBUF = (x);		                    \
        WHILE (!(UCB0IFG&UCTXIFG));                 \
        WHILE (!(UCB0IFG&UCRXIFG));					\
    } while(__LINE__ == -1)
        
void radio_spi_init(void);

void radio_port_init(void);

void radio_powerup(void);









#endif
