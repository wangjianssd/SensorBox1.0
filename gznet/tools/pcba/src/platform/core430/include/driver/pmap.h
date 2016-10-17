/**
 * @brief       : 
 *
 * @file        : pmap.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/10/13
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/13    v0.0.1      gang.cheng    first version
 */

#ifndef __PMAP_H__
#define __PMAP_H__

/**
 * @biref 配置PMAP，使得管脚映射可以在运行时配置
 */
void pmap_init(void);

/**
 * @brief 任意一个PMAPx的配置，指定第几个PMAP配置特定的功能
 * @param[in] pin 第几个映射脚
 * @param[in] mode 配置的复用模式，可配置的功能如下：
 *                 #define PM_NONE       0
 *                 #define PM_CBOUT      1
 *                 #define PM_TB0CLK     1
 *                 #define PM_ADC12CLK   2
 *                 #define PM_DMAE0      2
 *                 #define PM_SVMOUT     3
 *                 #define PM_TB0OUTH    3
 *                 #define PM_TB0CCR0B   4
 *                 #define PM_TB0CCR1B   5
 *                 #define PM_TB0CCR2B   6
 *                 #define PM_TB0CCR3B   7
 *                 #define PM_TB0CCR4B   8
 *                 #define PM_TB0CCR5B   9
 *                 #define PM_TB0CCR6B   10
 *                 #define PM_UCA0RXD    11
 *                 #define PM_UCA0SOMI   11
 *                 #define PM_UCA0TXD    12
 *                 #define PM_UCA0SIMO   12
 *                 #define PM_UCA0CLK    13
 *                 #define PM_UCB0STE    13
 *                 #define PM_UCB0SOMI   14
 *                 #define PM_UCB0SCL    14
 *                 #define PM_UCB0SIMO   15
 *                 #define PM_UCB0SDA    15
 *                 #define PM_UCB0CLK    16
 *                 #define PM_UCA0STE    16
 *                 #define PM_MCLK       17
 *                 #define PM_PM_E0      18
 *                 #define PM_PM_E1      19
 *                 #define PM_ANALOG     31
 *
 * @return 配置成功还是失败，如果pin设置大于8，则失败
 */
bool_t pmap_cfg(uint8_t pin, uint8_t mode);


#endif
