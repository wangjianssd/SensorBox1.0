 /**
 * @brief       : BH1750 中间层代码
 *
 * @file        : hal_bh1750.c
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
//#include <driver.h>
#include <hal_bh1750.h>

/**
 * @brief bh1750初始化
 */
bool_t hal_bh1750_init(void)
{
	bh1750_init();
	return TRUE;
}

/**
 * @brief bh1750写一个字节
 * @return 成功或失败
 */
bool_t hal_bh1750_write(uint8_t const value)
{
	if(bh1750_write(value) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * @brief bh1750读一个字节
 * @return 光照强度
 */
static uint16_t hal_bh1750_read(void)
{
	uint16_t temp = 0;
	temp = bh1750_read();
	if(temp == ERROR)
	{
		return ERROR;
	}
	return temp;
}

/**
 * @brief bh1750单次高精度读取
 * @return 光照强度
 */
float hal_bh1750_single_read(void)
{
	float lux_temp = 0;
    hal_bh1750_write(SINGLE_H_2);
//    delay_ms(180);
	lux_temp = hal_bh1750_read();
    lux_temp = (float)lux_temp/1.2;
	return lux_temp;
}

/**
 * @brief bh1750连续高精度读取
 * @return 光照强度
 */
float hal_bh1750_continu_read(void)
{
	float lux_temp = 0;
	lux_temp = hal_bh1750_read();
    lux_temp = (float)lux_temp/2.4;
	return lux_temp;
}

