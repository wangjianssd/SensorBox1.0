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

#include <data_type_def.h>
#include <hal.h>
#include <driver.h>

bool_t hal_sensor_read(int16_t *sensor_data_p,int16_t *max_data)
{
//	if(hal_rf_get_state() == HAL_RF_TX_STATE)	// RF处于发送态会导致磁场数据异常
//	{
//        *sensor_data_p = 0;
//		return FALSE;
//	}

	*sensor_data_p = sensor_detect(max_data);
    return TRUE;
}


bool_t hal_sensor_init(void)
{
    return sensor_init();
}

