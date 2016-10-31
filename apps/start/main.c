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
#include <gznet.h>
#include <app_task.h>

static uint8_t osel_heap_buf[OSEL_HEAP_SIZE];
//#define elec_lock_CtrlIO_Init_EX() {P4SEL &= ~BIT1; P4DIR |= BIT1; P4OUT &= ~BIT1;\
//                                    P6SEL &= ~BIT3; P6DIR |= BIT3; P6OUT &= ~BIT3;}
//#define elec_lock_CtrlIO_On_EX()   {P6OUT &= ~BIT3; P4OUT |= BIT1;}
//#define elec_lock_CtrlIO_Off_EX()  {P4OUT &= ~BIT1; P6OUT |= BIT3;}
//#define elec_lock_CtrlIO_Float_EX(){P4OUT &= ~BIT1; P6OUT &= ~BIT3;}
//#define EXTERN_LOCK_LOCK_DELAY_EX  (115)

int16_t main(void)
{
	/*开启了看门狗52s后复位*/
    hal_wdt_clear(16000);   //WANGJIAN
   // WDTCTL = WDTPW + WDTHOLD;
#ifdef NDEBUG
    bootloader_init();
#endif

	osel_env_init(osel_heap_buf, OSEL_HEAP_SIZE, OSEL_MAX_PRIO);

	hal_board_init();    
	stack_init();
    app_task_init();
    
	osel_run();
    
	return 0;
}
