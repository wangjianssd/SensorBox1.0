/**
 *
 * @brief       :  
 *
 * @file        : bsp_com.h
 * @author      : wangjian
 * Version      : v0.0.1
 * Date         : 2016-09-27
 *
 * Change Logs  :
 *
 * Date                 Version           Author          Notes
 * 2016-09-27           v0.1              wangjian        first version 
*/

#ifndef  __BSP_COM__
#define  __BSP_COM__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
/* Define --------------------------------------------------------------------*/
#define	__BSP_COM1__                                UART_2

#define	__COM1_RTX__                                DEV_UART_MODE_TX_RX 
#define	__COM1_CONFIG_STOP__                        DEV_UART_STOPBITS_1 
#define	__COM1_CONFIG_BIT__                         DEV_UART_WORDLENGTH_8B
#define	__COM1_CONFIG_PARITY__                      DEV_UART_PARITY_NONE
#define	__COM1_CONFIG_FLOW_CTL__                    UART_HWCONTROL_NONE
#define	__COM1_TX_FIFO_SIZE__                       128
#define	__COM1_RX_FIFO_SIZE__                       2048
#define	__COM1_RX_PIN_AF__                          DEV_UART_PIN_AF0
#define	__COM1_TX_PIN_AF__                          DEV_UART_PIN_AF0

/* Exported types ------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
bool BspCom1Init(uint32_t buad );
void BspCom1SendData( uint8_t *data, uint32_t len );
bool BspCom1TxFIFOIn( uint8_t *data, uint16_t len );
void BspCom1TxFIFOOut( void );
bool BspCom1RxFIFOIsEmpty( void );
void BspCom1RxFIFOClear( void );
uint8_t BspCom1RxFIFOOut( void );
void BspCom1RxDisable( void );
void BspCom1RxHander( uint8_t* data, uint16_t size );

/* Variables -----------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif

