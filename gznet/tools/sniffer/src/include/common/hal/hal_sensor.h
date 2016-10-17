/**
 * @brief       : 
 *
 * @file        : hal_sensor.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __HAL_SENSOR_H__
#define __HAL_SENSOR_H__

bool_t hal_sensor_read(int16_t *sensor_data, int16_t *max_data);

bool_t hal_sensor_init(void);

typedef struct
{
    int16_t x_axis;
    int16_t y_axis;
    int16_t z_axis;
} sensor_accelerated_data_t;

#endif
