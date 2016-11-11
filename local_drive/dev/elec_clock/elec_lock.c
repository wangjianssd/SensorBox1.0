/**
 * @brief       : 感知箱锁的驱动
 *
 * @file        : elec_lock.c
 * @author      : xxx
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#define ELEC_LOCK_GLO

#include <elec_lock.h>
#include <msp430.h>
#include "blu_tx.h"

#ifdef USE_Fingerprints
#include <app_frame.h>
#include <app_send.h>
#include <app_task.h>
#include "app_rfid.h"
#endif
extern bool_t if_get_password;
#ifdef USE_EXTERN_LOCK   
extern osel_etimer_t extern_lock_lock_timer;
extern osel_etimer_t extern_lock_frozen_timer;
#endif
/**
* @brief 锁初始化
*
*/
void elec_lock_init(void)
{
//    osel_int_status_t status = 0; //2016-04-14
//    OSEL_ENTER_CRITICAL(status); //2016-04-14

    elec_lock_CtrlIO_Init();

#ifdef USE_EXTERN_LOCK
    elec_lock_CtrlIO_Init_EX();
#endif

    elec_lock_StateIO_Init();
#ifndef USE_Fingerprints      
    elec_lock_StateIO_Int_Dis();
#endif    
    elec_lock_CtrlIO_Off();
#ifdef USE_Fingerprints
	//elec_lock_StateIO1_Int_En();
    elec_lock_StateIO1_Int_Dis();
	elec_lock_StateIO2_Int_Dis();
#endif 

    elec_lock_State = lock_Close;
    elec_lock_State_updata = 0;

//   OSEL_EXIT_CRITICAL(1);  //2016-04-14
}

/**
* @brief 开锁操作 
*
*/
#ifndef USE_Fingerprints
#ifdef USE_EXTERN_LOCK  
void extern_lock_timeout_set(uint16_t ticks)
{
    osel_etimer_disarm(&extern_lock_lock_timer);
    osel_etimer_arm(&extern_lock_lock_timer, (ticks/OSEL_TICK_PER_MS), 0); 
}

void stop_extern_lock_timeout(void)
{
    osel_etimer_disarm(&extern_lock_lock_timer);
}

void extern_lock_frozen_timeout_set(uint16_t ticks)
{
    osel_etimer_disarm(&extern_lock_frozen_timer);
    osel_etimer_arm(&extern_lock_frozen_timer, (ticks/OSEL_TICK_PER_MS), 0); 
}

void stop_extern_lock_frozen_timeout(void)
{
    osel_etimer_disarm(&extern_lock_frozen_timer);
}

#endif  

bool_t elec_lock_open(void)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

#ifdef USE_EXTERN_LOCK    

    elec_lock_CtrlIO_On_EX();

    elec_lock_State = lock_Open;

    extern_lock_frozen_timeout_set(EXTERN_LOCK_FROZEN_DELAY);
#else
    elec_lock_CtrlIO_On();
    delay_ms(300);//delay_ms(250);  //delay for ~500ms
    elec_lock_CtrlIO_Off();

    delay_ms(100);
#endif    

    if(elec_lock_StateIO_In() == 0)
    {
        elec_lock_State = lock_Open;
        OSEL_EXIT_CRITICAL(status);
        return TRUE;
    }
    OSEL_EXIT_CRITICAL(status);
    return FAILE;    
}
#else
bool_t elec_lock_open(void)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    elec_lock_StateIO1_Int_En();
    elec_lock_StateIO2_Int_En();
    //delay_ms(20);
    elec_lock_CtrlIO_On(); 
    OSEL_EXIT_CRITICAL(status);
    
    return TRUE;
}

void elec_lock_access(void)
{
	elec_lock_StateIO1_Int_Clear();
	elec_lock_StateIO1_Int_En();

}


#endif
/**
* @brief 闭锁操作 
*
*/
void elec_lock_close(void)
{
#ifdef USE_Fingerprints
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    elec_lock_CtrlIO_Off();
    OSEL_EXIT_CRITICAL(status);
#elif defined (USE_EXTERN_LOCK)

    elec_lock_CtrlIO_Off_EX();
    elec_lock_State = lock_Close;
    
    extern_lock_frozen_timeout_set(EXTERN_LOCK_FROZEN_DELAY);
#else

#endif

}


/**
* @brief elec lock state
*
*/
bool_t elec_lock_state(void)
{
    if(elec_lock_StateIO_In() != 0)
    {
        elec_lock_State = lock_Close;
        return FALSE;
    }
    else
    {
        elec_lock_State = lock_Open;
        return TRUE;
    }
}

#if 0
#pragma vector = PORT1_VECTOR
__interrupt void port1_int_handle(void)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status); 
    elec_lock_StateIO_Int_Clear();
    OSEL_EXIT_CRITICAL(status);
}
#endif

#ifdef USE_Fingerprints
extern void box_send_request(box_frame_t *frame, 
                         uint8_t         priority, 
                         uint8_t         allowed_send_times,
                         bool_t          need_ack);

#pragma vector = PORT3_VECTOR
__interrupt void port3_int_handle(void)
{  
//    osel_int_status_t status = 0;
//    OSEL_ENTER_CRITICAL(status);     
    if((elec_lock_StateIO1_Int_Get() != 0)&&(elec_lock_StateIO1_Int_En_IF() != 0) )
    {
        if (if_get_password)//如果有开锁权限
        {
            elec_lock_StateIO1_Int_Clear();        
            elec_lock_StateIO1_Int_Dis();
            elec_lock_StateIO2_Int_Clear();        
            elec_lock_CtrlIO_On();                      
            elec_lock_StateIO2_Int_Clear();
            elec_lock_StateIO2_Int_En();            
        }
        else
        {
            elec_lock_StateIO1_Int_Clear();
            elec_lock_StateIO1_Int_Dis();
            osel_event_t event;
            event.sig = LOCK_NO_PASSWORD_EVENT;
            event.param = NULL;
            osel_post(NULL, &app_task_thread_process, &event);               
        }        
    }
    else if((elec_lock_StateIO2_Int_Get() != 0)&&(elec_lock_StateIO2_Int_En_IF() != 0) )//&&((P3IE & BIT0) != 0) )
    {
    	elec_lock_StateIO2_Int_Dis();
        elec_lock_StateIO2_Int_Clear();
        elec_lock_StateIO1_Int_Clear();     
        if(elec_lock_StateIO2_In() == 0)
        {
            delay_ms(1);
            if(elec_lock_StateIO2_In() == 0)
            {
                elec_lock_CtrlIO_Off();
                stop_blu_no_lock_timeout();
                stop_nfc_no_lock_timeout();
                elec_lock_State_updata = 1;
                elec_lock_State = lock_Open;
                
                fingerprints_int_proc();
                elec_lock_StateIO1_Int_En();
                elec_lock_init();
                //开锁成功后，授权撤销
                if_get_password = FALSE;
                elec_lock_access();
            }
            else
            {
                elec_lock_StateIO2_Int_En();
            }
        }
        else
        {
            elec_lock_StateIO2_Int_En();
        }		
    }

//    OSEL_EXIT_CRITICAL(status);
}
#endif



