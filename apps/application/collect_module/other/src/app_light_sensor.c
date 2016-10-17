 /**
 * @brief       : 光敏传感器业务处理代码
 *
 * @file        : app_light_sensor.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
#include <hal_bh1750.h>
#include <app_light_sensor.h>
//#include <hal_timer.h>
//#include <data_type_def.h>
//#include <osel_arch.h>
#include <app_frame.h>
#include <app_send.h>

static void light_sensor_t1_timer_start();
//static hal_timer_t *light_sensor_t1_timer = NULL;
//static hal_timer_t *light_sensor_t2_timer = NULL;
static uint16_t light_sensor_count = 0;
extern osel_etimer_t light_sensor_t1_timer; //*< 光敏传感器周期T1设定定时器
extern osel_etimer_t light_sensor_t2_timer; //*< 光敏传感器周期T2设定定时器
/**
* @brief 定时器T2时间内处理
*/
static void light_sensor_t2_timer_start()
{
    osel_etimer_arm(&light_sensor_t2_timer,(LIGHT_SENSOR_T2_TIMER/OSEL_TICK_PER_MS),0);
}

void box_light_sensor_t2_timer_handle(void *arg)
{
    if (hal_bh1750_continu_read() >= LIGHT_SENSOR_STATUE_THRESH_L2)
    {
        if ((++light_sensor_count) >= LIGHT_SENSOR_COUNT_THRESH_C1)
        {
#if 0            
            //发送非法开箱帧
            box_frame_t box_frame;
            box_frame.frame_type = BOX_ALARM_FRAME;
            box_frame.box_type_frame_u.alarm_info.type = ALARM_LIGHT_OVERRUN;
            //数据插入链表
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
            box_send();
            box_light_sensor_close();
#endif
        }
        else
        {
//            hal_bh1750_write(0);
//            hal_bh1750_write(1);
            osel_etimer_disarm(&light_sensor_t2_timer);
            light_sensor_t2_timer_start();           
        }
    }
    else
    {
        light_sensor_count = 0;
        light_sensor_t1_timer_start();
    }
}

/**
* @brief 定时器T1时间内处理
*/
static void light_sensor_t1_timer_start()
{
    osel_etimer_arm(&light_sensor_t1_timer,(LIGHT_SENSOR_T1_TIMER/OSEL_TICK_PER_MS),0);
}

void box_light_sensor_t1_timer_handle(void *arg)
{
    //对于光敏传感器电源的打开和连续读的模式设置根据在关闭锁之后操作
    //对于光敏传感器电源的关闭在打开锁之后操作
    float lux_temp = 0;
//    static float temp = 0;
//    lux_temp = hal_bh1750_continu_read();
    lux_temp = hal_bh1750_single_read();
    //temp = lux_temp;
    if (lux_temp >= LIGHT_SENSOR_STATUE_THRESH_L1)
    {
        light_sensor_t2_timer_start();
    }
    else
    {
//        hal_bh1750_write(0);
//        hal_bh1750_write(1);
        osel_etimer_disarm(&light_sensor_t1_timer);
        light_sensor_t1_timer_start();
    }    
}

/**
* @brief 光照传感器初始化接口函数
*/
void box_light_sensor_init()
{
    hal_bh1750_init();
}


/**
* @brief 光照传感器启动接口函数
*/
void box_light_sensor_open()
{
    hal_bh1750_write(1);
    light_sensor_t1_timer_start();
}

/**
* @brief 光照传感器关闭接口函数
*/
void box_light_sensor_close()
{
    hal_bh1750_write(0);
    osel_etimer_disarm(&light_sensor_t1_timer);
    osel_etimer_disarm(&light_sensor_t2_timer);
}



