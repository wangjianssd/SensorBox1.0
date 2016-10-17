/**
 * @brief       : 
 *
 * @file        : sensor.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#define CONFIG_REG_A_ADDR       (0x00)
#define CONFIG_REG_B_ADDR       (0x01)
#define MODE_REG_ADDR           (0x02)
#define X_MSB_REG_ADDR          (0x03)
#define X_LSB_REG_ADDR          (0x04)
#define Z_MSB_REG_ADDR          (0x05)
#define Z_LSB_REG_ADDR          (0x06)
#define Y_MSB_REG_ADDR          (0x07)
#define Y_LSB_REG_ADDR          (0x08)
#define STATUS_REG_ADDR         (0x09)
#define ID_REG_A                (0x0A)
#define ID_REG_B                (0x0B)
#define ID_REG_C                (0x0C)
#define TEMPERATURE_REG_MSB     (0x31)
#define TEMPERATURE_REG_LSB     (0x32)

#define REG_A_DEFAULT_VALUE     (0x10)
#define REG_B_DEFAULT_VALUE     (0xA0)
#define REG_A_VALUE_FOR_DETECT  (0x90)
#define REG_B_VALUE_FOR_DETECT  (0xA0)
#define REG_A_VALUE_FOR_SELF_TEST   (0x91)
#define REG_B_VALUE_FOR_SELF_TEST   (0xA0)
#define MODE_REG_VALUE_FOR_SINGLE   (0x01)

/**
 * 初始化磁场传感器
 *
 * @param 无
 *
 * @return: TRUE  成功
 *          FALSE 失败
 */
bool_t sensor_init(void);

/**
 * 使用磁场传感器检测磁场值
 *
 * @param 最大的磁场值
 *
 * @return: Z轴的磁场值
 */
int16_t sensor_detect(int16_t *max_data);


void sensor_drdy_state_handler(void);
#endif
