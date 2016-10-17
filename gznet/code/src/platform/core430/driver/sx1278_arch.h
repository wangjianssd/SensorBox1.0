/**
 * @brief       : this
 * @file        : sx1278_arch.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-01-14
 * change logs  :
 * Date       Version     Author        Note
 * 2015-01-14  v0.0.1  gang.cheng    first version
 */
#ifndef __SX127X_ARCH_H__
#define __SX127X_ARCH_H__


#define SX127X_SPI_BEGIN()		(P8OUT &= ~BIT4)
#define SX127X_SPI_END()		(P8OUT |= BIT4)

#define SX127X_SPI_SEND_CHAR(x)						\
    do                                              \
    {                                               \
        while (!(UCA1IFG&UCTXIFG));                 \
        UCA1TXBUF = (x);		                    \
        while (!(UCA1IFG&UCTXIFG));                 \
        while (!(UCA1IFG&UCRXIFG));                 \
    } while(__LINE__ == -1)

#define SX127X_SPI_RECEIVE_CHAR()	(UCA1RXBUF)


void sx127x_reset(void);

void sx127x_spi_init(void);

void sx127x_port_init(void);

uint8_t sx127x_spi_write_read(uint8_t val);


void sx127x_ant_sw_init(void);

void sx127x_ant_sw_deinit(void);

void sx127x_ant_sw_set(uint8_t rx_tx);

#endif



