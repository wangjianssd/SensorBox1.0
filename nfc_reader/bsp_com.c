/**
 *
 * @brief       :  
 *
 * @file        : bsp_com.c
 * @author      : wangjian
 * Version      : v0.0.1
 * Date         : 2016-09-27
 *
 * Change Logs  :
 *
 * Date                 Version           Author          Notes
 * 2016-09-27           v0.1              wangjian        first version 
*/
/* Includes ------------------------------------------------------------------*/
#include "common/lib/lib.h"
#include "sys_arch/wsnos/wsnos.h"
//#include "pmap.h"
#include <gprs_tx.h>
#include "blu_tx.h"
#include "fifo.h"
#include "bsp_com.h"
/* Define --------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
//static 	void BspCom1RxHander( uint8_t* data, uint16_t size );
/* Variables -----------------------------------------------------------------*/
static uint8_t COM1TxFIFO[__COM1_TX_FIFO_SIZE__ + FIFO_INFO_SIZE] = {0};
uint8_t COM1RxFIFO[__COM1_RX_FIFO_SIZE__ + FIFO_INFO_SIZE] = {0};

//static uint8_t COM2TxFIFO[__COM2_TX_FIFO_SIZE__ + FIFO_INFO_SIZE];
//static uint8_t COM2RxFIFO[__COM2_RX_FIFO_SIZE__ + FIFO_INFO_SIZE];

//static uint8_t COM3TxFIFO[__COM3_TX_FIFO_SIZE__ + FIFO_INFO_SIZE];
//static uint8_t COM3RxFIFO[__COM3_RX_FIFO_SIZE__ + FIFO_INFO_SIZE];

void DevUartTx(uint8_t com, uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uart_send_char(com, data[i]);
    }
}

/*****************************************************************************
 * Function      : BspCom1Init
 * Description   : Init com1
 * Input         : uint32_t buad
 * Output        : None
 * Return        : bool
 * Others        : 
 * Record
 * 1.Date        : 20160927
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
bool BspCom1Init(uint32_t buad )
{
//    DevUartConfig config;
    
//    config.baud 		= buad;
//	config.length 		= DEV_UART_WORDLENGTH_8B;
//	config.mode 		= __COM1_RTX__;
//	config.parity 		= __COM1_CONFIG_PARITY__;
//	config.stop_bit     = __COM1_CONFIG_STOP__;
//	config.flow_contrl 	= __COM1_CONFIG_FLOW_CTL__;
//    config.pin.rx_af    = __COM1_RX_PIN_AF__;
//    config.pin.tx_af    = __COM1_TX_PIN_AF__;
//    config.uart_pin.rx_port = DEV_UART1_PIN_AF0_TX_PORT;
//    config.uart_pin.rx_pin  = DEV_UART1_PIN_AF0_TX_PIN;
//    config.uart_pin.tx_port = DEV_UART1_PIN_AF0_RX_PORT;
//    config.uart_pin.tx_pin  = DEV_UART1_PIN_AF0_RX_PIN;
//    config.uart_pin.rx_port = DEV_UART_PIN_AF0;

    //DevUartDeInit(__BSP_COM1__);
	
//	if (DevUartInit(__BSP_COM1__, config) != true)
//	{
//		return false;
//	}
    hal_uart_init(__BSP_COM1__, 115200); 

	FIFOInit ((FIFODataTypeDef *)COM1TxFIFO, __COM1_TX_FIFO_SIZE__);
	FIFOInit ((FIFODataTypeDef *)COM1RxFIFO, __COM1_RX_FIFO_SIZE__);
	
//	DevUartRxCbRegister(__BSP_COM1__, BspCom1RxHander);
//	DevUartIrqEnable(__BSP_COM1__ , DEV_UART_IT_RXNE);
	return true;
}

/*****************************************************************************
 * Function      : BspCom1SendData
 * Description   : COM1 send data
 * Input         : uint8_t *data
                uint32_t len
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
void BspCom1SendData( uint8_t *data, uint32_t len )
{
	DevUartTx(__BSP_COM1__, data, len);
}

/*****************************************************************************
 * Function      : BspCom1TxFIFOIn
 * Description   : put com1 send data in com1 TX FIFO 
 * Input         : uint8_t *data
                   uint16_t len
 * Output        : None
 * Return        : bool
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
bool BspCom1TxFIFOIn( uint8_t *data, uint16_t len )
{
	if (__COM1_TX_FIFO_SIZE__ - GetFIFOCount((FIFODataTypeDef *)COM1TxFIFO) < len)
	{
        return false;
	}

    for (uint32_t i = 0; i < len; i++)
    {
        FIFOIn((FIFODataTypeDef *)COM1TxFIFO, &data[i]);
    }

	return true;
}

/*****************************************************************************
 * Function      : BspCom1TxFIFOOut
 * Description   : Send COM1 TX FIFO data
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
void BspCom1TxFIFOOut( void )
{
    uint8_t byte;
    
    while (!FIFOIsEmpty((FIFODataTypeDef *)COM1TxFIFO))
    {
        FIFOOut((FIFODataTypeDef *)COM1TxFIFO, &byte);
        DevUartTx(__BSP_COM1__, &byte, 1);
    }
}

/*****************************************************************************
 * Function      : BspCom1RxFIFOIsEmpty
 * Description   : check whether COM1 RX FIFO is empty
 * Input         : void
 * Output        : None
 * Return        : bool
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
bool BspCom1RxFIFOIsEmpty( void )
{
    return FIFOIsEmpty((FIFODataTypeDef *)COM1RxFIFO);    
}

/*****************************************************************************
 * Function      : BspCom1RxFIFOOut
 * Description   : get COM1 RX data
 * Input         : void
 * Output        : None
 * Return        : uint8_t
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
uint8_t BspCom1RxFIFOOut( void )
{
    uint8_t byte;

	FIFOOut((FIFODataTypeDef *)COM1RxFIFO, &byte);

	return byte;
}

/*****************************************************************************
 * Function      : BspCom1RxHander
 * Description   : hander for COM1 rx irq
 * Input         : uint8_t* data
 				   uint16_t size
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
void BspCom1RxHander( uint8_t* data, uint16_t size )
{
	uint8_t i;
	
	for (i = 0; i < size; i++)
	{
		FIFOIn((FIFODataTypeDef *)COM1RxFIFO, &data[i]);
	}
}

/*****************************************************************************
 * Function      : BspCom1RxFIFOClear
 * Description   : clear COM1 RX FIFO
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
void BspCom1RxFIFOClear( void )
{
	FIFOInit ((FIFODataTypeDef *)COM1RxFIFO, __COM1_RX_FIFO_SIZE__);
}
/*****************************************************************************
 * Function      : BspCom1RxDisable
 * Description   : disable com1 rx
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160928
 *   Author      : wangjian
 *   Modification: Created function

*****************************************************************************/
void BspCom1RxDisable( void )
{
	//DevUartIrqDisable(__BSP_COM1__ , DEV_UART_IT_RXNE);

	DevUartRxCbUnregister(__BSP_COM1__);
}
