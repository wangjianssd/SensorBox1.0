/*! @file radio.h
 * @brief This file contains the public functions controlling the radio chip.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef RADIO_H_
#define RADIO_H_

#include "osel_arch.h"
#include <inc/compiler_defs.h>

/*****************************************************************************
 *  Global Typedefs & Enums
 *****************************************************************************/
typedef struct
{
    const uint8_t *Radio_ConfigurationArray;

    uint8_t Radio_ChannelNumber;
    uint8_t Radio_PacketLength;
    uint8_t Radio_State_After_Power_Up;

    uint16_t Radio_Delay_Cnt_After_Reset;
} tRadioConfiguration;

/*****************************************************************************
 *  Global Variable Declarations
 *****************************************************************************/
extern const SEGMENT_VARIABLE_SEGMENT_POINTER(pRadioConfiguration, tRadioConfiguration, SEG_CODE, SEG_CODE);

/*! Si446x configuration array */
extern const SEGMENT_VARIABLE(Radio_Configuration_Data_Array[], uint8_t, SEG_CODE);

/*****************************************************************************
 *  Global Function Declarations
 *****************************************************************************/
void  vRadio_Init(void);
void  vRadio_StartTx(uint8_t, uint8_t *);
void  vRadio_StartRX(uint8_t);


#define SSN_RADIO           si446x_driver
extern const struct radio_driver si446x_driver;





#endif /* RADIO_H_ */




