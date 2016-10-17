/**
 * @brief       : 
 *
 * @file        : main.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "common/hal/hal.h"
#include "common/dev/dev.h"
#include "sys_arch/osel_arch.h"
#include "stack/stack.h"

static uint8_t osel_heap_buf[OSEL_HEAP_SIZE];

//#define TEST_TASK_PRIO          (1)
//#define TEST_EVENT_MAX          (10u)   //*< 最多处理10个事件
//static osel_event_t test_event_store[TEST_EVENT_MAX];
//static osel_task_t *test_task_handle = NULL;
//
//#define OSEL_SYS_TICK_EVENT     (1u)
//
//PROCESS_NAME(test_process);
//PROCESS(test_process, "test_process");
//PROCESS_THREAD(test_process, ev, data)
//{
//    PROCESS_BEGIN();
//    static osel_etimer_t etimer;
//    static uint8_t txarray[] = {16, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
//    
//    while(1)
//    {
//        OSEL_ETIMER_DELAY(&etimer, OSEL_TICK_RATE_HZ);
////        hal_led_toggle(HAL_LED_BLUE);
//        
//        SSN_RADIO.set_value(RF_TXFIFO_FLUSH, 0);
//    
//        SSN_RADIO.set_value(RF_TXFIFO_CNT, sizeof(txarray));
//        SSN_RADIO.prepare(txarray, sizeof(txarray));  
//        SSN_RADIO.transmit(0);
//        txarray[1]++;
//        
//        OSEL_ETIMER_DELAY(&etimer, OSEL_TICK_RATE_HZ);
//        hal_led_toggle(HAL_LED_RED);
//    }
//    
//    PROCESS_END();
//}

#define SYSTICK_TASK_PRIO          (0)
#define SYSTICK_EVENT_MAX          (3u)   //*< 最多处理10个事件
static osel_event_t systick_event_store[SYSTICK_EVENT_MAX];
static osel_task_t *systick_task = NULL;

static hal_timer_t *osel_sys_timer = NULL;

static void systick_cb(void *arg)
{
    osel_sys_timer = NULL;
    osel_event_t event;
    osel_post(systick_task, NULL, &event);
}

static void systick_task_handle(void *param)
{
    HAL_TIMER_SET_REL(MS_TO_TICK(100),
                      systick_cb,
                      NULL,
                      osel_sys_timer);
    hal_led_toggle(HAL_LED_BLUE);
    
    static uint8_t txarray[] = {8, 1,2,3,4,5,6,7,8 };
    SSN_RADIO.set_value(RF_TXFIFO_FLUSH, 0);
    SSN_RADIO.set_value(RF_TXFIFO_CNT, sizeof(txarray));
    SSN_RADIO.prepare(txarray, sizeof(txarray));  
    SSN_RADIO.transmit(0);
    txarray[1]++;
}

void osel_etimer_task_init(void)
{
    systick_task = osel_task_create(systick_task_handle,
                                    SYSTICK_TASK_PRIO,
                                    systick_event_store,
                                    SYSTICK_EVENT_MAX);
    
    
    HAL_TIMER_SET_REL(MS_TO_TICK(300),
                      systick_cb,
                      NULL,
                      osel_sys_timer);
}


int16_t main(void)
{
	/*开启了看门狗52s后复位*/
#ifdef NDEBUG
    bootloader_init();
#endif
    
	osel_env_init(osel_heap_buf, OSEL_HEAP_SIZE, OSEL_MAX_PRIO);

	hal_board_init();

	//@TODO: 暂时hal timer移植未完成，协议栈先注释不运行 
    stack_init();
    
//    test_task_handle = osel_task_create(NULL, TEST_TASK_PRIO, test_event_store, TEST_EVENT_MAX);
//    osel_pthread_create(test_task_handle, &test_process, NULL);
    
//    SSN_RADIO.init();
//    osel_etimer_task_init();
    
	osel_run();
    
	return 0;
}
