/***************************************************************************
* @brief        : this
* @file         : si4463_arch.h
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-10-19
* @change Logs  :
* Date        Version      Author      Notes
* 2015-10-19      v0.1      gang.cheng    first version
***************************************************************************/

#ifndef __SI4463_ARCH_H__
#define __SI4463_ARCH_H__



/**
 * @group radio spi module
 */

#define radio_SpiWriteByte  radio_spi_read_byte
#define radio_SpiReadByte   radio_spi_read_byte


void radio_spi_init(void);

void radio_spi_deinit(void);

void radio_set_nsel(void);

void radio_clear_nsel(void);

uint8_t radio_spi_read_byte(uint8_t data);

void radio_SpiReadData(uint8_t len, uint8_t *data);

void radio_SpiWriteData(uint8_t len, uint8_t *data);



void radio_AssertShutdown(void);

void radio_DeassertShutdown(void);



void radio_4259_tx(void);

void radio_4259_rx(void);

void radio_4259_idle(void);



void radio_unb_on(void);

void radio_unb_off(void);



void radio_port_init(void);



void radio_nirq_init(void);

bool_t radio_get_NirqLevel(void);

void radio_nirq_enable(void);

void radio_nirq_disable(void);

void radio_nirq_deinit(void);

#endif

















