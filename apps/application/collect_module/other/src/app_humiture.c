 /**
 * @brief       : 温湿度产生处理
 *
 * @file        : app_humiture.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
//#include <data_type_def.h>
//#include <osel_arch.h>
//#include <hal_timer.h>
//#include <hal_board.h>
//#include <hal_sensor.h>
//#include <list.h>
#include <app_humiture.h>
#include <app_frame.h>
#include <app_send.h>
     
//static hal_timer_t *humiture_timer = NULL;
extern osel_etimer_t humiture_timer;
static bool_t humiture_timer_started = FALSE;
uint32_t humiture_period = BOX_HUMITURE_PERIOD;

static void humiture_start_timer(uint32_t period)
{
    if (period == 0)
    {
        return;
    }
    
    osel_etimer_arm(&humiture_timer, (period/OSEL_TICK_PER_MS), 0);   
    humiture_timer_started = TRUE;
}

void humiture_stop_timer()
{
    osel_etimer_disarm(&humiture_timer);    
    humiture_timer_started = FALSE;
}

//外部调用函数
void box_humiture_set_period(uint32_t period)
{
    humiture_period = period;
    if (humiture_period != 0)
    {
        humiture_start_timer(humiture_period);
    }
    else
    {
        humiture_stop_timer();
    }
}

//外部调用函数
void box_humiture_init(void)
{
//    humiture_timer = NULL;
    humiture_period = BOX_HUMITURE_PERIOD;
    box_humiture_set_period(humiture_period);
//    humiture_timer_started = FALSE;
}


//==============================================================================
float SHT2x_CalcRH(uint16_t u16sRH)
//==============================================================================
{
	float humidityRH; // variable for result
	u16sRH &= ~0x0003; // clear bits [1..0] (status bits)
	//-- calculate relative humidity [%RH] --
	humidityRH = -6.0 + 125.0/65536 * (float)u16sRH; // RH= -6 + 125 * SRH/2^16
	return humidityRH;
}

//==============================================================================
float SHT2x_CalcTemperatureC(uint16_t u16sT)
//==============================================================================
{
	float temperatureC; // variable for result
	u16sT &= ~0x0003; // clear bits [1..0] (status bits)
	//-- calculate temperature [��C] --
	temperatureC= -46.85 + 175.72/65536 *(float)u16sT; //T= -46.85 + 175.72 * ST/2^16
	return temperatureC;
}


extern sensor_info_t box_sensor;


void box_humiture_timer_handle(void *arg)
{
	fp32_t humidityRH; //variable for relative humidity[%RH] as float
	//char humitityOutStr[22] = {0}; //output string for humidity value
	fp32_t temperatureC; //variable for temperature[��C] as float
	//char temperatureOutStr[21] = {0}; //output string for temperature value

    uint16_t temperature;
    uint16_t humidity;
    box_frame_t box_frame;
    box_frame_t box_frame_1;
 
    if (!humiture_timer_started)
    {
        return;
    }
    
    //humiture_start_timer(humiture_period);
    hal_sensor_read(&temperature, SENSOR_DHT, DHT_SENSOR_TYPE_T);
    hal_sensor_read(&humidity, SENSOR_DHT, DHT_SENSOR_TYPE_H);

	//-- calculate humidity and temperature --
	temperatureC = SHT2x_CalcTemperatureC(temperature);
	humidityRH = SHT2x_CalcRH(humidity);

	box_sensor.temperature_value = temperatureC;
	box_sensor.humidity_value = humidityRH;
	
	box_frame.box_type_frame_u.sensor_info.temperature_value = box_sensor.temperature_value;
	box_frame.box_type_frame_u.sensor_info.humidity_value = box_sensor.humidity_value;

	box_frame.frame_type = BOX_SENSOR_FRAME;
    box_frame.box_type_frame_u.sensor_info.type = SENSOR_TMP_HUM;
	box_frame.box_type_frame_u.sensor_info.user_id = box_sensor.user_id;
	box_frame.box_type_frame_u.sensor_info.timestamp = box_sensor.timestamp;

	box_frame.box_type_frame_u.sensor_info.content[0] = ((uint8_t*)&temperatureC)[3];
    box_frame.box_type_frame_u.sensor_info.content[1] = ((uint8_t*)&temperatureC)[2];
    box_frame.box_type_frame_u.sensor_info.content[2] = ((uint8_t*)&temperatureC)[1];
    box_frame.box_type_frame_u.sensor_info.content[3] = ((uint8_t*)&temperatureC)[0];

	box_frame.box_type_frame_u.sensor_info.content[4] = ((uint8_t*)&humidityRH)[3];
    box_frame.box_type_frame_u.sensor_info.content[5] = ((uint8_t*)&humidityRH)[2];
    box_frame.box_type_frame_u.sensor_info.content[6] = ((uint8_t*)&humidityRH)[1];
    box_frame.box_type_frame_u.sensor_info.content[7] = ((uint8_t*)&humidityRH)[0];

	box_frame.box_type_frame_u.sensor_info.len = 8;
    //把温湿度信息插入链表
	box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
    box_send();    
#if 1    
    //查询是否有温度或湿度报警
    if ((uint8_t)temperatureC > BOX_TEMPERTURE_MAX)
    {
        box_frame_1.box_type_frame_u.alarm_info.type = ALARM_T_OVERRUN;
    }
    else if ((uint8_t)humidityRH > BOX_HUMITURE_MAX)
    {
        box_frame_1.box_type_frame_u.alarm_info.type = ALARM_H_OVERRUN;
    }
	
	if(((uint8_t)temperatureC > BOX_TEMPERTURE_MAX)||((uint8_t)humidityRH > BOX_HUMITURE_MAX))
	{
		box_frame_1.frame_type = BOX_ALARM_FRAME;
		
		box_blu_send_request(&box_frame_1);
    	box_send_request(&box_frame_1,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
        box_send();
        //box_blu_send_request(&box_frame_1);
	}
#endif
    //box_send();
    humiture_start_timer(humiture_period);
}
