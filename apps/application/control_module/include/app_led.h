 /**
 * @brief       : 感知箱LED的处理
 *
 * @file        : app_led.h
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#ifndef _APP_LED_H_
#define _APP_LED_H_

typedef enum
{   //状态优先级从低到高
    BOX_LED_INIT = 0x0000, //初始灭灯状态 
    BOX_LED_WORK = 0x0002u,//工作状态 ,呼吸灯  
    BOX_LED_ALARM = 0x0020u,//报警灯
}box_led_state_e;


void box_led_timer_handle(void *arg);
void box_led_enter_state(box_led_state_e state);
void box_led_exit_state(box_led_state_e state);
void box_led_init(void);


#endif