 /**
 * @brief       : BH1750 驱动代码
 *
 * @file        : bh1750.h
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/09/21
 *
 * Change Logs  : 
 *
 * Date        Version      Author      Notes
 * 2015/09/21  v0.0.1    cuihongpeng    first version
 */
#ifndef __BH1750_H_
#define __BH1750_H_

#define DEVICE_ADDR     0x23 /**< 七位从机地址 */
#define POWER_ON        0x01 /**< 上电 */
#define POWER_DOWN      0x00 /**< 掉电 */
#define RESET           0x03 /**< 复位 */
#define CONTINU_H_1     0x10 /**< 连续高精度采样模式1    1LUX      120MS  */
#define CONTINU_H_2     0x11 /**< 连续高精度采样模式2    0.5LUX    120MS  */
#define CONTINU_L       0x13 /**< 连续低精度采样模式     4LUX      16MS   */
#define SINGLE_H_1      0x20 /**< 单次高精度采样模式1    1LUX      120MS  */
#define SINGLE_H_2      0x21 /**< 单次高精度采样模式2    0.5LUX    120MS  */
#define SINGLE_L        0x23 /**< 单次低精度采样模式     4LUX      16MS   */

/**
 * @brief bh1750写一个字节
 * @return 成功或失败
 */
bool_t bh1750_write(uint8_t const value);

/**
 * @brief bh1750读一个字节
 * @return 光照强度
 */
uint16_t bh1750_read(void);
 
/**
 * @brief bh1750初始化
 */
void bh1750_init(void);

#endif