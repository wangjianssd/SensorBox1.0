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



/**
* @brief 锁初始化
*
*/
void elec_lock_init(void)
{
    elec_lock_CtrlIO_Init();
    elec_lock_StateIO_Init();

    elec_lock_State = lock_Close;
}

/**
* @brief 开锁操作 
*
*/
bool_t elec_lock_open(void)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    elec_lock_CtrlIO_On();
    delay_ms(200);  //delay for ~500ms
    elec_lock_CtrlIO_Off();
    delay_ms(100);
    if(elec_lock_StateIO_In() == 0)
    {
        elec_lock_State = lock_Open;
        OSEL_EXIT_CRITICAL(status);
        return TRUE;
    }
    OSEL_EXIT_CRITICAL(status);
    return FAILE;    
}

/**
* @brief 闭锁操作 
*
*/
void elec_lock_close(void)
{
    ;
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

#if 1
#pragma vector = PORT1_VECTOR
__interrupt void port1_int_handle(void)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status); 
    elec_lock_StateIO_Int_Clear();
//    if(elec_lock_StateIO_Int_Get() != 0)
//    {
//        elec_lock_StateIO_Int_Clear();
//
//        delay_ms(100); //wait key done
//      
//        if(elec_lock_StateIO_In() == 0)
//        {
//            elec_lock_State = lock_Open;
//        }
//        else
//        {
//            elec_lock_State = lock_Close;
//        }
//    }
    OSEL_EXIT_CRITICAL(status);
}
#endif

