/**
 * @brief       : this
 * @file        : radio.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-14
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-14  v0.0.1  gang.cheng    first version
 */
#ifndef __RADIO_H__
#define __RADIO_H__

#include "regs_fsk.h"
#include "regs_lora.h"


typedef enum
{
	MODEM_FSK = 0,
	MODEM_LORA,
} radio_modem_t;

typedef enum
{
	RF_IDLE = 0,
	RF_RX_RUNNING,
	RF_TX_RUNNING,
	RF_CAD,
} radio_state_t;

typedef struct
{
	int8_t power;
	uint8_t bandwidth;
	uint32_t datarate;
	bool_t low_datarate_optimize;
	uint8_t coderate;
	uint16_t preamble_len;
	bool_t fix_len;
	uint8_t payload_len;
	bool_t crc_on;

	bool_t freq_hop_on;
	uint8_t hop_period;

	bool_t iq_inverted;
	bool_t rx_continuous;
	uint32_t tx_timeout;
} radio_lora_settings_t;

typedef struct
{
	int8_t snr_value;
	uint8_t size;
	int16_t rssi_value;
} radio_lora_packethandler_t;

typdef struct
{
	uint8_t state;
	radio_modem_t modem;
	uint32_t freq;
	radio_lora_settings_t lora;
	radio_lora_packethandler_t lora_packet;
} radio_settings_t;

#define RF_MID_BAND_THRESH                          525000000

#define XTAL_FREQ									(32000000ul)
#define FREQ_STEP									(61.03515625)	//XTAL_FREQ/2^19

#define FIFO_READ_MASK							    0x7F
#define FIFO_WRITE_MASK 						    0x80

#define SSN_RADIO               sx127x_driver



#endif
