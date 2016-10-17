 /**
 * @brief       : GPS 数据发送处理流程处理
 *
 * @file        : app_gps.c
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
//#include <driver.h>
#include <hal_acc_sensor.h>
//#include <hal_uart.h>
#include <gps.h>
//#include <pbuf.h>
//#include <list.h>
#include <app_gps.h>
#include <gps.h>
#include <gprs_tx.h>
#include <app_frame.h>
#include "app_send.h"
#include <app_task.h>

//为位置信息周期上报设定定时器
//static hal_timer_t *gps_timer = NULL;
static uint32_t gps_period = 0;
uint8_t queclocator = 0;
extern osel_etimer_t gps_cycle_timer;

void box_gps_timer_end(void)
{
    osel_etimer_disarm(&gps_cycle_timer);
}

void box_gps_timer_start(void)
{
    osel_etimer_arm(&gps_cycle_timer, 
                    10, 
                    gps_period/OSEL_TICK_PER_MS);	
}

void box_gps_timer_out_event_handle(void *arg)
{
    box_gps_vcc_open();
}


/**
 * @brief gps串口解析后的得到的数据的回调处理
 *
*/
static void gps_data_cb(gps_simple_info_t gps_data)
{
    pbuf_t *pbuf;
    gps_simple_info_t *arg;
    osel_event_t event;
    
    pbuf = pbuf_alloc(sizeof(gps_simple_info_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (gps_simple_info_t *)pbuf->data_p;
    *arg = gps_data;

    event.sig = BOX_GPS_DATA_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);
}

/**
 * @brief gps经纬度格式转换
 *
*/
double gps_dm_to_d(double dm)
{
    int32_t dd;
    
    dd = (int32_t)dm / 100;
    
    return (dm - dd * 100) * ((double)1.0 / 60.0) + dd; 
}

//uint8_t open_lock_location_first_test = 0;


extern gps_simple_info_t box_location;
/**
 * @brief gps消息处理函数
 *
*/
void box_gps_data_event_handle(void *arg)
{
    pbuf_t *pbuf;
    gps_simple_info_t *cb_arg;
    box_frame_t box_frame;
    
    pbuf = (pbuf_t *)arg;
    cb_arg = (gps_simple_info_t *)pbuf->data_p;
    
    box_frame.frame_type = BOX_LOCATION_FRAME;
    box_frame.box_type_frame_u.location_info.len = 8;
    box_frame.box_type_frame_u.location_info.type = 0x01;

	if((cb_arg->latitude == 0.0)||(cb_arg->longitude == 0.0))
	{
		box_frame.box_type_frame_u.location_info.gps_info.longitude = box_location.longitude;
		box_frame.box_type_frame_u.location_info.gps_info.latitude = box_location.latitude;
	}
	else 
	{
    	box_frame.box_type_frame_u.location_info.gps_info.latitude = gps_dm_to_d(cb_arg->latitude);
    	box_frame.box_type_frame_u.location_info.gps_info.longitude = gps_dm_to_d(cb_arg->longitude);
        box_location.longitude = box_frame.box_type_frame_u.location_info.gps_info.longitude;
        box_location.latitude = box_frame.box_type_frame_u.location_info.gps_info.latitude;        
	}

    //插入链表
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);//for test not close 2016-04-05
    box_send();
    
    //关闭gps电源
    //box_gps_vcc_close();  //for test not close 2016-04-05
    pbuf_free(&pbuf __PLINE1);
    DBG_ASSERT(pbuf == NULL __DBG_LINE);
}

/**
 * @brief gps打开电源
 *
*/
void box_gps_vcc_open(void)
{
    gps_open();
}

/**
 * @brief gps关闭电源
 *
*/
void box_gps_vcc_close(void)
{
    gps_close();
}


void box_gps_sleep(void)
{
	gps_sleep();
}

void box_gps_wakeup(void)
{
	//gps_wakeup();
}

/**
 * @brief gps初始化
 *
*/
void box_gps_init(uint32_t period)
{
    gps_period = period;
    gps_init(gps_data_cb);
}