 /**
 * @brief       : RFID 驱动代码
 *
 * @file        : hal_rfid.c
 * @author      : wangjifang
 * @version     : v0.0.1
 * @date        : 2014/02/25
 *
 * Change Logs  : 修改注释及代码格式
 *
 * Date        Version      Author      Notes
 * 2014/02/25  v0.0.1    wangjifang    first version
 * 2015/09/15  v0.0.2    cuihongpeng   Second version
 */
#include <gznet.h> 
#include <rfid.h>
//#include <driver.h>
//#include <debug.h>
//#include <wsnos.h>

#define SDAOUT P8DIR |= BIT5
#define SDAIN  P8DIR &= ~BIT5
#define SDA1   P8OUT |= BIT5
#define SDA0   P8OUT &= ~BIT5
#define SDASEL0 P8SEL &= ~BIT5
#define SDASEL1 P8SEL |= BIT5
		
#define SCLOUT P8DIR |= BIT6
#define SCLIN  P8DIR &= ~BIT6
#define SCL1   P8OUT |= BIT6
#define SCL0   P8OUT &= ~BIT6
#define SCLSEL0 P8SEL &= ~BIT6
#define SCLSEL1 P8SEL |= BIT6

#define 	SECTION_SIZE        (0x80)				/**< 个扇区大小128byte */
#define 	SECTION_AREA_SIZE   (0x20)				/**< 一个扇区大小32个区域 */
#define 	AREA_SIZE           (0x04)				/**< 一个区域大小4byte */
#define 	NFC_MAX_ADDR        (0x2000)			/**< NFC最大地址 */

#define 	NFC_POWER_PROC_FLAG		(1U)
#define 	NFC_POWER_PROC()                                        \
    do                                                              \
    {                                                               \
	  	P9OUT &= ~BIT7;												\
		delay_us(10);												\
		P9OUT |= BIT7;												\
    } while(__LINE__ == -1)

static uint8_t i2c_add = 0;							/**< rfid 地址 */

/**
 * @brief i2c初始化
 */
void m24lr64e_i2c_init(void)
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
    UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC ;	// I2C主机模式
    UCB1CTL1 |= UCSSEL_2;				  	// 选择SMCLK
    UCB1BR0 = 40;
    UCB1BR1 = 0;
    UCB1CTL0 &= ~UCSLA10;					// 7位地址模式
    UCB1CTL1 &= ~UCSWRST;
}

/**
 * @brief i2c写nfc一个字节
 * @return 成功或失败
 */
static uint8_t i2c_send_byte(uint16_t const word_addr, uint8_t const word_value)
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
            return ERROR;
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
            return ERROR;
        }
    }   
    
    UCB1TXBUF = word_value;                         // 发送字节内容
    while(!(UCB1IFG & UCTXIFG));                    // 等待UCTXIFG=1
    
    UCB1CTL1 |= UCTXSTP;
    while(UCB1CTL1 & UCTXSTP);                      // 等待发送完成
    OSEL_EXIT_CRITICAL(status);
    return RIGHT;
}

/**
 * @brief i2c读nfc一个字节
 * @return TRUE or FALSE
 */
static uint8_t i2c_recv_byte(uint16_t const word_addr , uint8_t *const pword_buf)
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
            return ERROR;
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
            return ERROR;
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
            return ERROR;
        }
    }
    UCB1CTL1 |= UCTXSTP;                            // 先发送停止位
    while(!(UCB1IFG & UCRXIFG));                    // 读取字节内容
    *pword_buf = UCB1RXBUF;                         // 读取BUF寄存器在发送停止位之后
    while( UCB1CTL1 & UCTXSTP );
    OSEL_EXIT_CRITICAL(status);
    return RIGHT; 
}

/**
 * @brief 读nfc数据
 * @param[in] 空
 * @return TRUE or FALSE
 */
bool_t read_rfid_data(uint8_t *const data_buf, uint8_t const data_len, 
                      uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    UCB1I2CSA = i2c_add;
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
            return ERROR;
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
            return ERROR;
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
            return ERROR;
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
    
    return RIGHT;
}

/**
 * @brief 写入4个字节
 * @param[in] 空
 * @return TRUE or FALSE
 */
static uint8_t write_nfc_4byte_data(uint8_t *const data_buf, 
                                    uint8_t const data_len,
                                    uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    UCB1I2CSA = i2c_add;
    uint8_t *data_temp = data_buf;
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
            return ERROR;
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
            return ERROR;
        }
    }
    
    for( uint8_t i = 0 ; i < data_len ; i++ )
    {
        UCB1TXBUF = *data_temp ++;                   // 发送寄存器内容
		while(!(UCB1IFG & UCTXIFG))
    	{ 
        	if( UCB1IFG & UCNACKIFG )                // 若无应答 UCNACKIFG=1
        	{
#if	NFC_POWER_PROC_FLAG
	  			NFC_POWER_PROC();
#endif			  
            	UCB1CTL1 |= UCTXSTP;
            	OSEL_EXIT_CRITICAL(status);
            	return ERROR;
        	}
    	}		
    }
    
    UCB1CTL1 |= UCTXSTP;
    while(UCB1CTL1 & UCTXSTP);                      // 等待发送完成
    
    OSEL_EXIT_CRITICAL(status);
    return RIGHT;                                   //返回操作成败标志  
}

/**
 * @brief 写入一个扇区
 * @param[in] 空
 * @return TRUE or FALSE
 */
bool_t write_rfid_area_data(uint8_t *const data_buf,
                            uint8_t const data_len,
                            uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    uint8_t *data_temp = data_buf;
    uint16_t word_addr_temp = word_addr;
    //该起始地址所在区域还剩多少字节可写
    uint8_t remain_data_pro = AREA_SIZE - (word_addr_temp % AREA_SIZE);
    uint8_t remain_data_end = 0;
    if(data_len >= remain_data_pro)
    {
        //最后一个区域
        remain_data_end = (data_len-remain_data_pro)%AREA_SIZE;
    }
    if(remain_data_pro != 0)
    {
        if(data_len <= remain_data_pro)
        {
            remain_data_pro = data_len;
        }
        if(RIGHT == write_nfc_4byte_data(data_temp, remain_data_pro, word_addr_temp))
        {
            data_temp += remain_data_pro;
            word_addr_temp += remain_data_pro;
            delay_ms(5);
        }
    }
    //写完开始的后还剩几个完整区域
    uint8_t data_area_numb = (data_len-remain_data_pro)/AREA_SIZE;
    for(uint8_t i=0; i<data_area_numb; i++)
    {
        if(RIGHT == write_nfc_4byte_data(data_temp, AREA_SIZE, word_addr_temp))
        {
            data_temp += AREA_SIZE;
            word_addr_temp += AREA_SIZE;
            delay_ms(5);
        }
        else
        {
            return ERROR;
        }
    }
	
    if(remain_data_end != 0)                             //写最后的区域
    {
        write_nfc_4byte_data(data_temp, remain_data_end, word_addr_temp);
        delay_ms(5);
    }
	
    return RIGHT;
}

/**
 * @brief 写入数据(多块写入)
 * @param[in] 空
 * @return TRUE or FALSE
 */
bool_t write_rfid_data(uint8_t *const data_buf,
                       uint8_t const data_len,
                       uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    uint8_t *data_temp = data_buf;
    uint16_t word_addr_temp = word_addr;
    uint8_t remain_data_area_pro = SECTION_SIZE - (word_addr_temp % SECTION_SIZE);  
    uint8_t remain_data_area_end = 0;
    if(data_len >= remain_data_area_pro)
    {
        remain_data_area_end = (data_len-remain_data_area_pro)%SECTION_SIZE;
    }
    if(remain_data_area_pro != 0)
    {
        if(data_len <= remain_data_area_pro)
        {
            remain_data_area_pro = data_len;
        }
        if(RIGHT == write_rfid_area_data(data_temp, remain_data_area_pro,
                                        word_addr_temp))
        {
            data_temp  += remain_data_area_pro;
            word_addr_temp += remain_data_area_pro;
        }
    }
    //写完开始的后还剩几个整扇区
    uint8_t data_section_numb = (data_len-remain_data_area_pro)/SECTION_SIZE;
    for(uint8_t i=0;i<data_section_numb; i++)
    {
        if(RIGHT == write_rfid_area_data(data_temp, SECTION_SIZE, word_addr_temp))
        {
            data_temp += SECTION_SIZE;
            word_addr_temp += SECTION_SIZE;
        }
        else
        {
            return ERROR;
        }
    }
	
    if(remain_data_area_end != 0)
    {
        if(ERROR == write_rfid_area_data(data_temp, remain_data_area_end, word_addr_temp))
        {
            return ERROR;
        }   
    }
	
    return RIGHT;
}

void m24lr64e_vcc_open(void)
{
    P3SEL &= ~BIT7;
	P3DIR |= BIT7;
	P3OUT |= BIT7;
}

void m24lr64e_vcc_close(void)
{
    P3SEL &= ~BIT7;
	P3DIR |= BIT7;
	P3OUT &= ~BIT7;
}

/**
 * @brief m24lr64e硬件端口初始化
 * @param[in] 空
 * @return TRUE or FALSE
 */
void m24lr64e_port_init(void)
{
 	TB0CCTL0 &= ~CCIE; 
  	P4SEL |= BIT0;   //config P8.0 as input capture io
	P4DIR &= ~BIT0;  //config P8.0 as input
    TB0CCTL0 = CM_1 + CCIS_0 + SCS + CAP;  //CM_1上升沿捕获，CCIS_1选择CCIxB,
                                            //SCS同步捕获，CAP捕获模式  
}

/**
 * @brief m24lr64e中断配置
 * @param[in] 空
 * @return TRUE or FALSE
 */
void m24lr64e_int_cfg(void)
{
  	//设置中断P4.0
  	TB0CCTL0 &= ~CCIFG;//初始为中断未挂起
	TB0CCTL0 |= CCIE;// 开启中断使能
}

/**
 * @brief m24lr64e寄存器配置
 * @param[in] 空
 * @return TRUE or FALSE
 */
void m24lr64e_reg_cfg(void)
{
  	uint8_t oldreg = 0x00;
	uint8_t newreg = 0x00;
	
  	i2c_recv_byte(0x910,&oldreg);
	
	newreg = oldreg | 0x08;//RF WIP/BUSY 设置为1，即WRITE模式
	
	i2c_send_byte(0x910,newreg);
}

/**
 * @brief m24lr64e初始化
 * @param[in] 空
 * @return TRUE or FALSE
 */
void m24lr64e_init(void)
{
	m24lr64e_vcc_open();
    m24lr64e_i2c_init();//I2C模块的初始化:包括使用前I2C的释放操作和寄存器的配置

    m24lr64e_port_init();//配置硬件接口
	
	i2c_add = RFID_ADDRESS_E2_1;
	m24lr64e_reg_cfg();//rfid寄存器配置	
	i2c_add = RFID_ADDRESS_E2_0;

    m24lr64e_int_cfg();//中断配置
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void timer0_b0_isr(void)
{
    OSEL_ISR_ENTRY();
    extern void m24lr64e_int_proc(void);
	m24lr64e_int_proc();
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}
