/**
 * @brief       : 
 *
 * @file        : i2c.c
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/10/23
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/23    v0.0.1      WangJifang    some notes
 */

#include "common/lib/lib.h"
#include "common/lib/debug.h"
#include "sys_arch/osel_arch.h"
#include "platform/platform.h"
#include "platform/core430/driver/i2c.h"

//SDA 数据线定义
#define SDAOUT P8DIR |= BIT5
#define SDAIN  P8DIR &= ~BIT5
#define SDA1   P8OUT |= BIT5
#define SDA0   P8OUT &= ~BIT5
#define SDASEL0 P8SEL &= ~BIT5
#define SDASEL1 P8SEL |= BIT5

//SCL 时钟线定义
#define SCLOUT P8DIR |= BIT6
#define SCLIN  P8DIR &= ~BIT6
#define SCL1   P8OUT |= BIT6
#define SCL0   P8OUT &= ~BIT6
#define SCLSEL0 P8SEL &= ~BIT6
#define SCLSEL1 P8SEL |= BIT6

#define SECTION_SIZE        0x80            //一个扇区大小128byte
#define SECTION_AREA_SIZE   0x20            //一个扇区大小32个区域
#define AREA_SIZE           0x04            //一个区域大小4byte
#define NFC_MAX_ADDR        0x2000          //NFC最大地址

#define NFC_POWER_PROC_FLAG					1
#define NFC_POWER_PROC()                                            \
    do                                                              \
    {                                                               \
	  	P3OUT &= ~BIT7;												\
		delay_us(10);												\
		P3OUT |= BIT7;												\
    } while(__LINE__ == -1)
	  									
static uint8_t i2c_add = 0;

//*****************i2c初始化****************************************************
bool_t i2c_init(void)
{
    SCLSEL0;
    SCLOUT;
    SCL1;
    for(uint8_t i = 0; i < 9; i++ )
    {
        SCL1;
        delay_ms(1);
        SCL0;
        delay_ms(1);
    }
    
    P8SEL |= (BIT5 + BIT6);
    UCB1CTL1 |= UCSWRST;
    UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC ;          // I2C主机模式
    UCB1CTL1 |= UCSSEL_2;                           // 选择SMCLK
    UCB1BR0 = 40;
    UCB1BR1 = 0;
    UCB1CTL0 &= ~UCSLA10;                           // 7位地址模式
//    UCB1I2CSA = i2c_slave_dev;                    // NFC
    UCB1CTL1 &= ~UCSWRST;
    
    return TRUE;
}

bool_t i2c_deinit(void)
{
    return FALSE;
}

//****************i2c写nfc一个字节***************************************
static bool_t i2c_send_byte(uint16_t const word_addr, uint8_t const word_value)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);

    UCB1I2CSA = i2c_add;
    osel_int_status_t status = 0;

    OSEL_ENTER_CRITICAL(status);
    while( UCB1CTL1 & UCTXSTP );
    UCB1CTL1 |= UCTR;                               // 写模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位
    UCB1TXBUF = (uint8_t)(word_addr>>8);            // 发送字节高地址
    
    while(!(UCB1IFG & UCTXIFG))
    {  	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    
    UCB1TXBUF = (uint8_t)(word_addr);               // 发送字节低地址
    while(!(UCB1IFG & UCTXIFG))
    {		  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }   

    UCB1TXBUF = word_value;                         // 发送字节内容
    while(!(UCB1IFG & UCTXIFG));                    // 等待UCTXIFG=1

    UCB1CTL1 |= UCTXSTP;
    while(UCB1CTL1 & UCTXSTP);                      // 等待发送完成
    OSEL_EXIT_CRITICAL(status);
    return TRUE;
}

//*****************i2c读nfc一个字节****************************************
static bool_t i2c_recv_byte(uint16_t const word_addr , uint8_t *const pword_buf)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(pword_buf != NULL __DBG_LINE);
    
    UCB1I2CSA = i2c_add;
    osel_int_status_t status = 0;
    
    OSEL_ENTER_CRITICAL(status);
    UCB1CTL1 |= UCTR;                               // 写模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位和写控制字节
    
    UCB1TXBUF = (uint8_t)(word_addr>>8);            // 发送字节高地址
    while(!(UCB1IFG & UCTXIFG))
    {
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }

    UCB1TXBUF = (uint8_t)(word_addr);               // 发送字节低地址
    while(!(UCB1IFG & UCTXIFG))
    {  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }                        
    
    UCB1CTL1 &= ~UCTR;                              // 读模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位和读控制字节
    
    while(UCB1CTL1 & UCTXSTT)                       // 等待UCTXSTT=0
    {	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    UCB1CTL1 |= UCTXSTP;                            // 先发送停止位
    while(!(UCB1IFG & UCRXIFG));                    // 读取字节内容
    *pword_buf = UCB1RXBUF;                         // 读取BUF寄存器在发送停止位之后
    while( UCB1CTL1 & UCTXSTP );
    OSEL_EXIT_CRITICAL(status);
    return TRUE; 
}

//*******************i2c读数据**************************************************
bool_t i2c_recv_data(uint8_t mode, uint8_t *const data_buf, uint8_t const data_len, 
                      uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    UCB1I2CSA = mode;
    uint8_t *data_temp = data_buf;
    
    osel_int_status_t status = 0;
    
    OSEL_ENTER_CRITICAL(status);
    
    UCB1CTL1 |= UCTR;                               // 写模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位和写控制字节
    
    UCB1TXBUF = (uint8_t)(word_addr>>8);            // 发送字节高地址
    while(!(UCB1IFG & UCTXIFG))
    {	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    
    UCB1TXBUF = (uint8_t)(word_addr);               // 发送字节低地址
    while(!(UCB1IFG & UCTXIFG))
    {	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }                        
    
    UCB1CTL1 &= ~UCTR;                              // 读模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位和读控制字节
    
    while(UCB1CTL1 & UCTXSTT)                       // 等待UCTXSTT=0
    {	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    for( uint8_t i = 0 ; i < data_len - 1 ; i++ )
    {
        while(!(UCB1IFG & UCRXIFG));                // 读取字节内容
        *data_temp++ = UCB1RXBUF;                    // 发送寄存器内容
    }
    UCB1CTL1 |= UCTXSTP;
    while(!(UCB1IFG & UCRXIFG));                    // 读取最后一个字节内容
    *data_temp = UCB1RXBUF;
    while(UCB1CTL1 & UCTXSTP);                      // 等待发送完成
    OSEL_EXIT_CRITICAL(status);
    
    return TRUE;
}

//******************************************************************************
//i2c写byte。
static bool_t i2c_write_data(uint8_t mode, uint8_t *const data_buf, 
                                    uint8_t const data_len,
                                    uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    UCB1I2CSA = mode;
    uint8_t *data_temp = data_buf;
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    
//    while( UCB1CTL1 & UCTXSTP );
    UCB1CTL1 |= UCTR;                               // 写模式
    UCB1CTL1 |= UCTXSTT;                            // 发送启动位
    
    UCB1TXBUF = (uint8_t)(word_addr>>8);            // 发送字节高地址
    while(!(UCB1IFG & UCTXIFG))
    {	  
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    
    UCB1TXBUF = (uint8_t)(word_addr);               // 发送字节低地址
    while(!(UCB1IFG & UCTXIFG))
    { 
        if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
#if	NFC_POWER_PROC_FLAG
	  		NFC_POWER_PROC();
#endif			  
            UCB1CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    
    for( uint8_t i = 0 ; i < data_len ; i++ )
    {
        UCB1TXBUF = *data_temp ++;                   // 发送寄存器内容
		while(!(UCB1IFG & UCTXIFG))
    	{ 
        	if( UCB1IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        	{
#if	NFC_POWER_PROC_FLAG
	  			NFC_POWER_PROC();
#endif			  
            	UCB1CTL1 |= UCTXSTP;
            	OSEL_EXIT_CRITICAL(status);
            	return FALSE;
        	}
    	}		
    }
    
    UCB1CTL1 |= UCTXSTP;
    while(UCB1CTL1 & UCTXSTP);                      // 等待发送完成
    
    OSEL_EXIT_CRITICAL(status);
    return TRUE;                                   //返回操作成败标志  
}

const i2c_driver_t i2c_device_driver = 
{
    i2c_init,
    i2c_deinit,
    i2c_write_data,
    i2c_recv_data,
    NULL,
};