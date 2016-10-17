 /**
 * @brief       : 感知箱主APP任务处理
 *
 * @file        : app_task.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
#include <app_task.h>
//#include <data_type_def.h>
//#include <osel_arch.h>
//#include <debug.h>
//#include <hal_uart.h>
//#include <serial.h>
//#include <hal_board.h>
//#include <hal_nvmem.h>
//#include <hal_timer.h>
#include <app_acc_sensor.h>
#include <app_gps.h>
#include <app_heartbeat.h>
#include <app_humiture.h>
#include <app_light_sensor.h>
#include <app_rfid.h>
#include <app_led.h>
#include <app_send.h>
#include <app_frame.h>
//#include <wsnos.h>
#include <hal_elec_lock.h>
#include <gps.h>
extern device_info_t device_info;
extern bool_t if_get_password;

static osel_task_t *app_task = NULL;
static osel_event_t app_task_event_store[20];
osel_etimer_t acc_cycle_etimer;   //*< 该定时器用于加速度传感器周期性采集
osel_etimer_t gps_cycle_timer;     //*< 该定时器用于GPS周期性上报
osel_etimer_t heartbeat_timer;      //*< 心跳设定定时器
osel_etimer_t low_power_timer_1;    //*< 电压采集设定定时器
osel_etimer_t humiture_timer;    //*< 温湿度集设定定时器
osel_etimer_t light_sensor_t1_timer; //*< 光敏传感器周期T1设定定时器
osel_etimer_t light_sensor_t2_timer; //*< 光敏传感器周期T2设定定时器
osel_etimer_t led_timer;            //*< LED
osel_etimer_t wait_ack_timer;       //*< 下行接收等待定时器
osel_etimer_t nfc_wait_isr_timer;   //*< nfc中断延迟定时器
osel_etimer_t acc_gps_cycle_etimer; //*< 进入运动后gps周期上报定时器
void lock_no_password_event_handle(void);
osel_etimer_t buzzer_cycle_timer;
osel_etimer_t gprs_test_stop_timer;

extern void blu_no_data_timer_cb(void);

extern osel_etimer_t nfc_no_lock_timer;
extern void nfc_no_lock_timer_cb(void);
extern gps_simple_info_t box_location;

#ifdef USE_Fingerprints
osel_etimer_t fingerprints_isr_timer;
#endif

#ifdef USE_EXTERN_LOCK   
osel_etimer_t extern_lock_lock_timer;
osel_etimer_t extern_lock_frozen_timer;

#endif

PROCESS(app_task_thread_process, "app task thread process");

PROCESS_THREAD(app_task_thread_process,ev,data)
{
    PROCESS_BEGIN();    
    while(1)
    {
       if (BOX_ACC_DATA_TIMER_EVENT == ev)//加速度传感器周期采集
       {
           box_acc_data_timer_event_handle(data);
       }
       else if (BOX_GPS_TIMER_EVENT == ev)//gps周期性上报数据
       {
           box_gps_timer_out_event_handle(data);
       }
       else if (BOX_GPS_DATA_EVENT == ev)//gps获取到数据后的回调处理
       {
           box_gps_data_event_handle(data);
       }
       else if (BOX_POWER_TIMER_1_EVENT == ev)
       {
           box_low_power_1_handle(data);
       }
       else if (BOX_HEARTBEAT_TIMER_EVENT == ev)
       {
           box_heartbeat_timer_handle(data);
       }        
       else if (BOX_HUMITURE_TIMER_EVENT == ev)
       {
           box_humiture_timer_handle(data);
       }  
       else if (BOX_LIGHT_T1_TIMER_EVENT == ev)
       {
           box_light_sensor_t1_timer_handle(data);
       }       
       else if (BOX_LIGHT_T2_TIMER_EVENT == ev)
       {
           box_light_sensor_t2_timer_handle(data);
       }       
       else if (BOX_NFC_INT_EVENT == ev)
       {
           box_nfc_int_handle(data);
       }       
       else if (BOX_LED_TIMER_EVENT == ev)
       {
           box_led_timer_handle(data);
       }         
       else if (BOX_DATA_SENT_EVENT == ev)
       {
           box_data_sent_event_handle(data);
       }       
       else if (BOX_WAIT_ACK_TIMER_EVENT == ev)
       {
           box_wait_ack_timer_event_handle(data);
       }       
       else if (BOX_DATA_RECEIVED_EVENT == ev)
       {
           box_data_received_event_handle(data);
       } 
       else if (BOX_SSN_CONTROL_EVENT == ev)
       {
           box_ssn_ctrl_event_handle(data);
       }
       else if (BOX_GPRS_STOP_EVENT == ev)
       {
           box_gprs_stop_timer_handle(data);
       }
#if 0	   
	   else if (BOX_BLU_DATA_SENT_EVENT == ev)
	   {
		   box_blu_data_sent_event_handle(data);
	   }  
#endif	   
#if 0	   
       else if (BLU_NO_DATA_EVENT == ev)
	   {
		   blu_no_data_timer_cb();
	   }
#endif	   
#if 1

	   else if (BUZZER_TIMER_EVENT == ev)
	   {
			buzzer_cycle_timer_out_event_handle();
	    }
#endif	   
#ifdef USE_Fingerprints       
       else if (BOX_FRIGNER_INT_EVENT == ev)
       {
           box_fingerprints_int_handle(data);
       }
#endif  
#ifdef USE_EXTERN_LOCK          
      else if (EXTERN_LOCK_LOCK_EVENT == ev)
      {
          extern_lock_lock_handle(data);
      }
      else if (EXTERN_LOCK_FROZEN_EVENT == ev)
      {
          extern_lock_frozen_handle(data);
          extern_lock_status_send();
      }
#endif  

       else if (NFC_NO_LOCK_EVENT == ev)
       {
          nfc_no_lock_timeout_timer_cb();
       }
       else if (LOCK_NO_PASSWORD_EVENT == ev)
       {
           lock_no_password_event_handle();
       }
	 
       PROCESS_YIELD();     //*< 释放线程控制权，进行任务切换 
    }
    PROCESS_END();
}

//当锁未授权的时候打开的处理
void lock_no_password_event_handle(void)
{
    box_frame_t box_frame;
    box_frame.frame_type = BOX_ALARM_FRAME;
    box_frame.box_type_frame_u.alarm_info.type = ALARM_NO_PASSWORD;
    //数据插入链表
    buzzer_cycle_timer_start(1000);
    box_blu_send_request(&box_frame);				
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);   
    box_send();     
}
/**
* @brief 任务绑定与注册
*
*/
void app_task_init(void)
{
    uint32_t gps_period_temp = 0;
    if_get_password = FALSE;
   
    app_task = osel_task_create(NULL,
                                APP_TASK_PRIO,
                                app_task_event_store,
                                10);
    if (NULL == app_task)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
    
    osel_pthread_create(app_task,&app_task_thread_process,NULL);     
    osel_etimer_ctor(&acc_cycle_etimer,&app_task_thread_process,BOX_ACC_DATA_TIMER_EVENT,NULL);//*<加速度传感器周期采集定时器
    osel_etimer_ctor(&gps_cycle_timer,&app_task_thread_process,BOX_GPS_TIMER_EVENT,NULL);//*<GPS周期上报定时器
    osel_etimer_ctor(&heartbeat_timer,&app_task_thread_process,BOX_HEARTBEAT_TIMER_EVENT,NULL);//*<心跳周期定时器
    osel_etimer_ctor(&low_power_timer_1,&app_task_thread_process,BOX_POWER_TIMER_1_EVENT,NULL);//*<低电压周期采集定时器
    osel_etimer_ctor(&humiture_timer,&app_task_thread_process,BOX_HUMITURE_TIMER_EVENT,NULL);//*<温湿度周期采集定时器
#ifndef USE_Fingerprints
    osel_etimer_ctor(&light_sensor_t1_timer,&app_task_thread_process,BOX_LIGHT_T1_TIMER_EVENT,NULL);//*<光敏传感器周期T1采集定时器
    osel_etimer_ctor(&light_sensor_t2_timer,&app_task_thread_process,BOX_LIGHT_T2_TIMER_EVENT,NULL);//*<光敏传感器周期T2采集定时器
#endif    
    osel_etimer_ctor(&led_timer,&app_task_thread_process,BOX_LED_TIMER_EVENT,NULL);//*<LED定时器
    osel_etimer_ctor(&wait_ack_timer,&app_task_thread_process,BOX_WAIT_ACK_TIMER_EVENT,NULL);//*<下行接收等待定时器
    osel_etimer_ctor(&nfc_wait_isr_timer,&app_task_thread_process,BOX_NFC_INT_EVENT,NULL);//*<NFC中断延时定时器    
    
    box_nfc_init();
#ifdef USE_Fingerprints
    osel_etimer_ctor(&fingerprints_isr_timer,&app_task_thread_process,BOX_FRIGNER_INT_EVENT,NULL);//*<ָ�ƿ����ж϶�ʱ��
#endif        

#ifdef USE_EXTERN_LOCK   
    osel_etimer_ctor(&extern_lock_lock_timer,&app_task_thread_process,EXTERN_LOCK_LOCK_EVENT,NULL);//*<������ʱ��
osel_etimer_ctor(&extern_lock_frozen_timer,&app_task_thread_process,EXTERN_LOCK_FROZEN_EVENT,NULL);//*<������ʱ��
#endif   

#if 1
	osel_etimer_ctor(&buzzer_cycle_timer, &app_task_thread_process, BUZZER_TIMER_EVENT, NULL); 
#endif
    device_info = hal_board_info_get();
    
    osel_etimer_ctor(&nfc_no_lock_timer, &app_task_thread_process, NFC_NO_LOCK_EVENT, NULL);     
 
    box_send_init();//内部包括gprs的初始化
    
    gps_period_temp = 30000;//60s
    box_gps_init(gps_period_temp);

#if 1
	box_frame_t box_frame;     
	box_frame.frame_type = BOX_REG_DEVICE_FRAME;	
	box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
    
    box_frame.frame_type = BOX_LOCATION_FRAME;
    box_frame.box_type_frame_u.location_info.len = 8;
    box_frame.box_type_frame_u.location_info.type = 0x01;
//    box_frame.box_type_frame_u.location_info.gps_info.longitude = 120.3801434053;//无锡公司
//    box_frame.box_type_frame_u.location_info.gps_info.latitude = 31.4958789655;     
    box_frame.box_type_frame_u.location_info.gps_info.longitude = box_location.longitude;//手机给的
    box_frame.box_type_frame_u.location_info.gps_info.latitude = box_location.latitude;    
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);    
        
	box_send();
#endif	

#if 1
    buzzer_cycle_timer_start(100); 
#endif    

#if 1
    box_heartbeat_init();      
	box_power_heart_timer_start(600000,300000);
#endif
    //开启锁的中断使能
#ifdef USE_Fingerprints
    elec_lock_access();
#endif
    //box_enable_acc_alg();//test
#if 0
    //造数据发送(为测试用)
    //1设备启动帧:上电已发送
	box_frame_t box_frame;     
	box_frame.frame_type = BOX_REG_DEVICE_FRAME;	
	box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,TRUE);    
    
    //2位置信息
    box_frame.frame_type = BOX_LOCATION_FRAME;
    box_frame.box_type_frame_u.location_info.len = 8;
    box_frame.box_type_frame_u.location_info.type = 0x01;
    box_frame.box_type_frame_u.location_info.gps_info.longitude = 120.3801434053;//无锡公司
    box_frame.box_type_frame_u.location_info.gps_info.latitude = 31.4958789655;    
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,TRUE);
  
    //3心跳
    box_frame.frame_type = BOX_HEART_FRAME;
    box_frame.box_type_frame_u.heart_info.lock_remain_energy = 3000;
    box_frame.box_type_frame_u.heart_info.ssn_remain_energy = 3000;
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);       
    //传感器信息
    //4归还箱子使用权
    box_frame.frame_type = BOX_SENSOR_FRAME;
    box_frame.box_type_frame_u.sensor_info.len = 0x00;
    box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOSE_RIGHTS;
    box_frame.box_type_frame_u.sensor_info.user_id =0;
    box_frame.box_type_frame_u.sensor_info.timestamp = 0;    
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE); 
    //5开锁信息
	box_frame.frame_type = BOX_SENSOR_FRAME;
	box_frame.box_type_frame_u.sensor_info.type = SENSOR_UNLOCK;
	box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);        

    //报警信息
    //6电子锁低电压 
    box_frame.frame_type = BOX_ALARM_FRAME;
    box_frame.box_type_frame_u.alarm_info.type = ALARM_LOCK_LOWPOWER;
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
    //7温度超限
    box_frame.frame_type = BOX_ALARM_FRAME;
    box_frame.box_type_frame_u.alarm_info.type = ALARM_T_OVERRUN;
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
    //8湿度超限
    box_frame.frame_type = BOX_ALARM_FRAME;
    box_frame.box_type_frame_u.alarm_info.type = ALARM_H_OVERRUN;
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);    
    //9加速度超限
    box_frame.frame_type = BOX_ALARM_FRAME;
    box_frame.box_type_frame_u.alarm_info.type = ALARM_ACC_OVERRUN;
    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE); 
    
    box_send();
    
#endif    
}
