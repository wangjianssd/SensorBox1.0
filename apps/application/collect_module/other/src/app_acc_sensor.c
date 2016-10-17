 /**
 * @brief       : 加速度传感器数据采集
 *
 * @file        : app_acc_sensor.c
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
#include <hal_acc_sensor.h>
//#include <hal_timer.h>
//#include <list.h>
//#include <hal_board.h>
//#include <ssn.h>
//#include <mac_moniter.h>
#include <acc_sensor_algo.h>
#include <app_frame.h>
#include <app_send.h>
#include <app_gps.h>
#include "sensor_accelerated.h"
#include <elec_lock.h>
//#include <gps.h>

#define     ACC_FILTER_TEST     0
#if ACC_FILTER_TEST  
#include <math.h>
#endif

//static hal_timer_t *acc_data_timer = NULL;
extern osel_etimer_t acc_cycle_etimer;
extern osel_etimer_t gps_cycle_timer;
extern void gps_open(void);
extern uint8_t box_operation_status_flag;


extern osel_etimer_t buzzer_cycle_timer;

void buzzer_cycle_timer_end(void)
{
	buzzer_off();
    osel_etimer_disarm(&buzzer_cycle_timer);
}

void buzzer_cycle_timer_start(uint32_t args)
{
	buzzer_on();   //WANGJIAN
    osel_etimer_arm(&buzzer_cycle_timer,  
                    args/OSEL_TICK_PER_MS,0);	
}

void buzzer_cycle_timer_out_event_handle(void)
{
    buzzer_off();
    osel_etimer_disarm(&buzzer_cycle_timer);
    
#ifdef USE_Fingerprints
    elec_lock_access();
#endif
}




/**
* @breif 周期性读取加速度值定时器超时后的回调处理
* @param[in] 无
* @return 无
*/
//static void acc_data_timer_cb(void *arg)
//{
//    acc_data_timer = NULL;
//    osel_post(BOX_ACC_DATA_TIMER_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
//}

/**
* @breif 加速度算法处理后的回调结果
* @param[in] acc_alg_event_t *acc_alg_event 触发算法的事件
* @return 无
*/
static void acc_alg_event_callback(const acc_alg_event_t *acc_alg_event)
{
    box_frame_t box_frame;
    DBG_ASSERT(acc_alg_event != NULL __DBG_LINE);
               
    switch (acc_alg_event->type)
    {
        case ACC_ALG_OVERRUN_EVENT_TYPE:
            //if (box_operation_status_flag)
            {
                //发送加速度值超限
                box_frame.frame_type = BOX_ALARM_FRAME;
                box_frame.box_type_frame_u.alarm_info.type = ALARM_ACC_OVERRUN;
                //数据插入链表
                buzzer_cycle_timer_start(300);
                box_blu_send_request(&box_frame);				
                box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);   
                box_send();                    
            }  
            
            break;
            
        case ACC_ALG_ACTIVITY_EVENT_TYPE:            
            switch (acc_alg_event->activity_args.activity_status_type)
            {
                case ACC_ALG_STATIC_TO_ACT_STATUS:
                    //启动gps周期显示定时器
                    if (box_operation_status_flag)
                    {
                        //box_gps_timer_start();
                        box_gps_vcc_open();
                    }
                    
                    break;

                case ACC_ALG_ACT_TO_STATIC_STATUS:
                    if (box_operation_status_flag)
                    {
                        box_gps_timer_end();
                        box_gps_vcc_close();                        
                    }

                    break;
                    
//                default :
//                    DBG_ASSERT(FALSE __DBG_LINE);
            }
            break;
            
        default :               
//            DBG_ASSERT(FALSE __DBG_LINE);
            break;
    }
}

/**
* @breif 启动周期性读取加速度值的定时器
* @param[in] time 周期
* @return 无
*/
//static void start_acc_data_timer(uint32_t time)
//{
//    if (NULL != acc_data_timer)
//    {
//        hal_timer_cancel(&acc_data_timer);
//        DBG_ASSERT(acc_data_timer == NULL __DBG_LINE);
//    }
//    HAL_TIMER_SET_REL(MS_TO_TICK(time),
//                      acc_data_timer_cb,
//                      NULL,
//                      acc_data_timer);
//    DBG_ASSERT(acc_data_timer != NULL __DBG_LINE);
//}

/**
* @breif 周期性采集加速度传感器数据，并调用算法接口处理
* @param[in] 无
* @return 无
*/
void box_acc_data_timer_event_handle(void *arg)
{
    acc_data_t acc_data[32];
    uint8_t count;
    acc_alg_data_t acc_alg_data;
#if ACC_FILTER_TEST
    fp32_t alpha = 0.1;
    fp32_t rx,ccx,ry,ccy,rz,ccz;
    static fp32_t eex = 0;
    static fp32_t ex = 0;
    static fp32_t aax = 0;
    static fp32_t data_ix = 0;  
  
    static fp32_t eey = 0;
    static fp32_t ey = 0;
    static fp32_t aay = 0;
    static fp32_t data_iy = 0; 

    static fp32_t eez = 0;
    static fp32_t ez = 0;
    static fp32_t aaz = 0;
    static fp32_t data_iz = 0; 
    
    rx = alpha;
    ry = alpha;
    rz = alpha;
#endif    
//    hal_wdt_clear(16000);
//    hal_hardware_wdt_clear();    
//    start_acc_data_timer(800);   
    count = hal_acc_get_detect_data(acc_data, 32);
	
#if GPS_DEBUG_INFO == 1
	extern uint8_t gps_cmd_cnt;
	gps_cmd_cnt += count;
#endif
    
    for (uint8_t i = 0; i < count; i++)
    {
#if ACC_FILTER_TEST        
        //x轴迭代
        ex = acc_data[i].x - data_ix;
        eex = rx * ex + (1-rx) * eex;
        aax = rx * fabs(ex) + (1 - rx) * aax;        
        if (fabs(aax) < 0.001)
        {
            ccx = 0.5;
        }
        else
        {
            ccx = fabs(eex/aax);
        }  
        
        data_ix = ccx * acc_data[i].x + (1 - ccx) * data_ix;
        
        //y轴迭代
        ey = acc_data[i].y - data_iy;
        eey = ry * ey + (1-ry) * eey;
        aay = ry * fabs(ey) + (1 - ry) * aay;        
        if (fabs(aay) < 0.001)
        {
            ccy = 0.5;
        }
        else
        {
            ccy = fabs(eey/aay);
        } 
        
        data_iy = ccy * acc_data[i].y + (1 - ccy) * data_iy;      
        
        //z轴迭代
        ez = acc_data[i].z - data_iz;
        eez = rz * ez + (1-rz) * eez;
        aaz = rz * fabs(ez) + (1 - rz) * aaz;        
        if (fabs(aaz) < 0.001)
        {
            ccz = 0.5;
        }
        else
        {
            ccz = fabs(eez/aaz);
        }  
        
        data_iz = ccz * acc_data[i].z + (1 - ccz) * data_iz; 
        
//        printf("%f         %f          %f\r\n",data_ix,data_iy,data_iz); 
        acc_alg_data.x = (int16_t)(data_ix * 3.9);
        acc_alg_data.y = (int16_t)(data_iy * 3.9);
        acc_alg_data.z = (int16_t)(data_iz * 3.9);        
#else 
//        printf("%d         %d          %d\r\n",acc_data[i].x,acc_data[i].y,acc_data[i].z);
        acc_alg_data.x = (int16_t)(acc_data[i].x * 3.9);
        acc_alg_data.y = (int16_t)(acc_data[i].y * 3.9);
        acc_alg_data.z = (int16_t)(acc_data[i].z * 3.9);
#endif
        acc_alg_new_data(&acc_alg_data);
    }
}


/**
* @breif 启动算法处理接口：算法初始化和算法配置函数,以及启动周期读取加速度数据定时器
* @param[in] 无
* @return 无
*/
void box_enable_acc_alg(void)
{   //算法初始化 
    acc_alg_init();
    // 配置算法模块
    acc_alg_config_t acc_alg_cfg;    
    acc_alg_cfg.data_rate = 100;
    acc_alg_cfg.acc_alg_cb = acc_alg_event_callback;
    acc_alg_config(&acc_alg_cfg);    
    // 启动读取加速度数据的定时器
//    start_acc_data_timer(800);
    osel_etimer_arm(&acc_cycle_etimer, 10, 80);   //*< 加速度传感器周期性采集触发 10ms =1tick
}


void box_disable_acc_alg(void)
{
    osel_etimer_disarm(&acc_cycle_etimer);    
}
