 /**
 * @brief       : 感知箱LED的处理
 *
 * @file        : app_led.c
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
//#include <hal_board.h>
//#include <hal_timer.h>
#include <app_led.h>

#define BOX_LED_DEFAULT_TIMER_PERIOD    500
#define BOX_LED_OPEN_TIMER_PERIOD    200

//此处调试时用8000ms，，版本固件使用100s
#define BOX_LED_CLOSE_TIMER_PERIOD   100000

//static hal_timer_t *led_timer = NULL;
extern osel_etimer_t led_timer;
static uint32_t led01_timer_period = BOX_LED_DEFAULT_TIMER_PERIOD;
static uint32_t led02_timer_period = BOX_LED_DEFAULT_TIMER_PERIOD;
static uint32_t led01_open_period = BOX_LED_OPEN_TIMER_PERIOD;
static uint32_t led01_close_period = BOX_LED_CLOSE_TIMER_PERIOD;
static uint16_t led_state = BOX_LED_INIT;

#define LED_DEV_ERR_COUNT_MAX_NUM       120//500ms闪一次，闪120次就是一分钟

static void stop_led_timer(void)
{
    osel_etimer_disarm(&led_timer);
}

static void start_led_timer(uint8_t led01,uint8_t led02)
{
    uint8_t led_temp = 0;  
    stop_led_timer();

    led_temp = (led01 << 1) | (led02);
    if (led_temp == 0x2)
    {
        osel_etimer_arm(&led_timer,(led01_timer_period/OSEL_TICK_PER_MS),0);
    }
    else if (led_temp == 0x1)
    {
        osel_etimer_arm(&led_timer,(led02_timer_period/OSEL_TICK_PER_MS),0);
    }
    else if (led_temp == 0x3)
    {
        osel_etimer_arm(&led_timer,(BOX_LED_DEFAULT_TIMER_PERIOD/OSEL_TICK_PER_MS),0);
    }
}

static void box_led_set_state(box_led_state_e state)
{    
    stop_led_timer();

    led01_open_period = BOX_LED_DEFAULT_TIMER_PERIOD;
    led01_close_period = BOX_LED_DEFAULT_TIMER_PERIOD;    
    switch (state)
    {
    //初始状态(优先级最低 0x00)    
    case BOX_LED_INIT:
        hal_led_close(HAL_LED_BLUE);
        hal_led_close(HAL_LED_RED);
        break;         
    //工作状态(优先级 0x01)    
    case BOX_LED_WORK:
        hal_led_open(HAL_LED_BLUE);
        hal_led_close(HAL_LED_RED);
        //绿灯亮200ms，灭100s(即0.01hz闪烁)：
        //调试时设置亮200ms，灭8s
        led01_open_period = BOX_LED_OPEN_TIMER_PERIOD;
        led01_close_period = BOX_LED_CLOSE_TIMER_PERIOD;
        led01_timer_period = led01_open_period;
        start_led_timer(1,0);
        break;   
        
     //报警状态(优先级最高 0x20)
    case BOX_LED_ALARM:
        hal_led_open(HAL_LED_BLUE);
        hal_led_open(HAL_LED_RED);
        led02_timer_period = BOX_LED_DEFAULT_TIMER_PERIOD;
        start_led_timer(0,1);
        break;
        
    default :
        break;
    }
}

void box_led_enter_state(box_led_state_e state)
{
    box_led_state_e current_state = BOX_LED_INIT;
    
    if (led_state & state)
    {
        return;
    }
    
    led_state |= state;
    
    for (uint16_t i = 0x8000; i != 0; i >>= 1)
    {
        if (led_state & i)
        {
            current_state = (box_led_state_e)i;
            break;
        }
    }
    
    box_led_set_state(current_state);
}

void box_led_exit_state(box_led_state_e state)
{
    box_led_state_e current_state = BOX_LED_INIT;
    
    if (!(led_state & state))
    {
        return;
    }
    
    led_state &= ~state;
    
    for (uint16_t i = 0x8000; i != 0; i >>= 1)
    {
        if (led_state & i)
        {
            current_state = (box_led_state_e)i;
            break;
        }
    }
    
    box_led_set_state(current_state);
}

void box_led_timer_handle(void *arg)
{
    uint8_t led = (uint32_t)arg;
    
    switch (led)
    {
    case 0x1://红灯闪烁
        hal_led_toggle(HAL_LED_RED);
        start_led_timer(0,1);        
        break;
        
    case 0x2://绿灯闪烁
        if (led01_timer_period == led01_open_period)
        {
            led01_timer_period = led01_close_period;
        }
        else
        {
            led01_timer_period = led01_open_period;
        }
        hal_led_toggle(HAL_LED_BLUE);
        start_led_timer(1,0);

        break;
        
    case 0x3://交替闪烁
        hal_led_toggle(HAL_LED_RED);
        hal_led_toggle(HAL_LED_BLUE);
        start_led_timer(1,1);        
        break;
        
    default :
        break;
    }
}

void box_led_init(void)
{
//    led_timer = NULL;
    led01_timer_period = BOX_LED_DEFAULT_TIMER_PERIOD;
    led02_timer_period = BOX_LED_DEFAULT_TIMER_PERIOD;
    led_state = BOX_LED_INIT;
}






