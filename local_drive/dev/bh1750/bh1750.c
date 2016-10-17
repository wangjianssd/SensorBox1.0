 /**
 * @brief       : BH1750 驱动代码
 *
 * @file        : bh1750.c
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/09/21
 *
 * Change Logs  : 
 *
 * Date        Version      Author      Notes
 * 2015/09/21  v0.0.1    cuihongpeng    first version
 */
#include <gznet.h> 
#include <bh1750.h>
#include <rfid.h> 
//#include <driver.h>
//#include <wsnos.h>

/**
 * @brief bh1750 I2C初始化
 * @return 成功或失败
 */
bool_t i2c_bh1750_init()
{
    P9SEL &= ~BIT6;
    P9DIR |= BIT6;
    P9OUT |= BIT6;
    for(uint8_t i = 0; i < 9; i++ )
    {
        P9OUT |= BIT6;
        delay_ms(1);
        P9OUT &= ~BIT6;
        delay_ms(1);
    }
    
    P9SEL |= (BIT5 + BIT6);
    UCB2CTL1 |= UCSWRST;
    UCB2CTL0 = UCMST + UCMODE_3 + UCSYNC ;          // I2C主机模式
    UCB2CTL1 |= UCSSEL_2;                           // 选择SMCLK
    UCB2BR0 = 40;
    UCB2BR1 = 0;
    UCB2CTL0 &= ~UCSLA10;                           // 7位地址模式
//    UCB2I2CSA = i2c_slave_dev;                    // NFC
    UCB2CTL1 &= ~UCSWRST;
    
    return TRUE;    
}
/**
 * @brief bh1750写一个字节
 * @return 成功或失败
 */
bool_t bh1750_write(uint8_t const value)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    
    while( UCB2CTL1 & UCTXSTP );
    
    UCB2I2CSA = DEVICE_ADDR;
    UCB2CTL1 |= UCTR;                               // 写模式
    UCB2CTL1 |= UCTXSTT;                            // 发送启动位
    
    UCB2TXBUF = value;                              // 发送字节内容
    while(!(UCB2IFG & UCTXIFG))
    {         
        if( UCB2IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        {
            UCB2CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }
    UCB2CTL1 |= UCTXSTP;
    while(UCB2CTL1 & UCTXSTP);                      // 等待发送完成
    OSEL_EXIT_CRITICAL(status);
    return TRUE;
}

/**
 * @brief bh1750读一个字节
 * @return 光照强度
 */
uint16_t bh1750_read(void)
{
    uint16_t temp = 0;
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    
    UCB2I2CSA = DEVICE_ADDR;
    UCB2CTL1 &= ~UCTR;                              // 读模式
    UCB2CTL1 |= UCTXSTT;                            // 发送启动位和读控制字节
    
    while(UCB2CTL1 & UCTXSTT)                       // 等待UCTXSTT=0
    {     
        if( UCB2IFG & UCNACKIFG )                   // 若无应答 UCNACKIFG=1
        { 
            UCB2CTL1 |= UCTXSTP;
            OSEL_EXIT_CRITICAL(status);
            return ERROR;
        }
    }
    while(!(UCB2IFG & UCRXIFG));                    // 读取字节内容
    temp = UCB2RXBUF;                               // 读取BUF寄存器在发送停止位之后
    UCB2CTL1 |= UCTXSTP;                            // 先发送停止位
    while(!(UCB2IFG & UCRXIFG));                    // 读取字节内容
    temp = (temp << 8) + UCB2RXBUF;                 // 读取BUF寄存器在发送停止位之后
    while( UCB2CTL1 & UCTXSTP );
    OSEL_EXIT_CRITICAL(status);
    return temp; 
}

/**
 * @brief bh1750初始化
 */
void bh1750_init(void)
{
    i2c_bh1750_init();
    bh1750_write(POWER_ON);
    bh1750_write(SINGLE_H_2);//CONTINU_H_2
}
