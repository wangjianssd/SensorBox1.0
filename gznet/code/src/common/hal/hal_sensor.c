/**
 * @brief       : 
 *
 * @file        : hal_sensor.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "hal.h"
#include "platform/platform.h"
#include <sensor_humiture.h>
#include <sensor_accelerated.h>

//bool_t hal_sensor_read(int16_t *sensor_data_p,int16_t *max_data)
//{
////	if(hal_rf_get_state() == HAL_RF_TX_STATE)	// RF处于发送态会导致磁场数据异常
////	{
////        *sensor_data_p = 0;
////		return FALSE;
////	}
//
//	*sensor_data_p = sensor_detect(max_data);
//    return TRUE;
//}
//
//
//bool_t hal_sensor_init(void)
//{
//    return sensor_init();
//}

bool_t hal_sensor_read(void *sensor_data_p, uint8_t sensor_type, uint8_t type)
{
    if (sensor_type == SENSOR_DHT)
    {
        if (type == DHT_SENSOR_TYPE_H)
        {
            return humiture_sensor_read_h((uint16_t *)sensor_data_p);
        }
        else if (type == DHT_SENSOR_TYPE_T)
        {
            return humiture_sensor_read_t((uint16_t *)sensor_data_p);
        }
        else
        {
            return FALSE;
        }
    }
    else if (sensor_type == SENSOR_ACCE)
    {
        return adxl345_read_buff(0x32, (uint8_t *)sensor_data_p, 6);
    }
    else
    {
        return FALSE;
    }
}


void hal_sensor_init(void)
{
    humiture_sensor_init();
    
#if (NODE_TYPE == NODE_TYPE_TAG)
//    smoke_sensor_init();
//    rtu_init();
#endif
}