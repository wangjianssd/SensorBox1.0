 /**
 * @brief       : 心跳帧产生处理
 *
 * @file        : app_heartbeat.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
#include <string.h>
//#include <osel_arch.h>
//#include <hal_timer.h>
//#include <hal_board.h>
//#include <hal_energy.h>
//#include <list.h>
#include <app_frame.h>
#include <app_heartbeat.h>
#include <app_send.h>
////为心跳设定定时器
//static hal_timer_t *heartbeat_timer = NULL;
////为电压采集设定定时器
//static hal_timer_t *low_power_timer_1 = NULL;

extern osel_etimer_t heartbeat_timer;
extern osel_etimer_t low_power_timer_1;
static uint32_t heartbeat_period = 0;
static uint32_t power_period = 0;
static uint16_t low_power_data = 0;

static void low_power_timer_1_start()
{
    if (power_period == 0)
    {
        return;
    }
    
    osel_etimer_arm(&low_power_timer_1, (power_period/OSEL_TICK_PER_MS), 0);
}

static void heartbeat_timer_start()
{
    if (heartbeat_period == 0)
    {
        return;
    }
    
    osel_etimer_arm(&heartbeat_timer, (heartbeat_period/OSEL_TICK_PER_MS), 0);
}

void box_heartbeat_init(void)
{
//    heartbeat_timer = NULL;
//	low_power_timer_1 = NULL;
    heartbeat_period = 0;
    power_period = 0;
}

//外部调用函数
void box_power_heart_timer_start(uint32_t heart_period,uint32_t low_power_period)
{
    heartbeat_period = heart_period;//心跳周期
    power_period = low_power_period;//电压采集周期
	
    //启动心跳周期定时器
    heartbeat_timer_start();
	//定时low_power_timer_1,采集电压
	//low_power_timer_1_start();
}

void box_heartbeat_timer_stop(void)
{
    osel_etimer_disarm(&heartbeat_timer);
}

void box_low_power_1_handle(void *arg)
{
    box_frame_t box_frame;
    low_power_data = hal_voltage_get();
	
    if (low_power_data <= LOW_POWER_VAR)
    {
        //发送低电压帧
        box_frame.frame_type = BOX_ALARM_FRAME;
        box_frame.box_type_frame_u.alarm_info.type = ALARM_LOCK_LOWPOWER;
        //插入链表
		
		box_blu_send_request(&box_frame);
		
        box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
        box_send();        
    }
    
    low_power_timer_1_start();
}

void box_heartbeat_timer_handle(void *arg)
{
    box_frame_t box_frame;
	extern uint8_t box_operation_status_flag;
	
    heartbeat_timer_start(); //继续定时
	//增加low_power_timer_1定时器定时
	//low_power_timer_1_start();
	
    box_frame.frame_type = BOX_HEART_FRAME;
    box_frame.box_type_frame_u.heart_info.lock_remain_energy = low_power_data;
    //box_frame.box_type_frame_u.heart_info.ssn_remain_energy = low_power_data;
	box_frame.box_type_frame_u.heart_info.box_operation_status = box_operation_status_flag + 1;
	
//	extern bool_t is_sending;
//	is_sending = FALSE;

	//插入链表
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE); 
	box_send();

}

