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

//bool_t hal_sensor_read(int16_t *sensor_data, int16_t *max_data);
//bool_t hal_sensor_init(void);

#define     DETECTOR_TYPE_DHT                   (0x01u)
#define     DETECTOR_TYPE_ACCELERATED           (0x02u)

typedef enum {
    SENSOR_DHT = 0x00,
    SENSOR_ACCE,
    SENSOR_RTU_READ,
    SENSOR_RTU_CHECK,
    SENSOR_RTU_SEND,
    SENSOR_SMOKE_FILTER,
    SENSOR_SMOKE_POWER_ON,
    SENSOR_SMOKE_POWER_OFF,
	SENSOR_DOOR
} snesor_type_t;

#define     DHT_SENSOR_TYPE_T                   (0x00)  // temperature
#define     DHT_SENSOR_TYPE_H                   (0x01)  // humidity

typedef struct
{
    int16_t x_axis;
    int16_t y_axis;
    int16_t z_axis;
} sensor_accelerated_data_t;

bool_t hal_sensor_read(void *sensor_data_p, uint8_t sensor_type, uint8_t type);
void hal_sensor_init(void);
#endif
