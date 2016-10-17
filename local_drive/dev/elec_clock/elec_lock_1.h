 /**
 * @brief       : 感知箱锁的控制
 *
 * @file        : elec_lock.h
 * @author      : xxx
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */

#ifndef _ELEC_LOCK_H_
#define _ELEC_LOCK_H_

#include <gznet.h> 
//#include <data_type_def.h>
//#include "driver.h"
/**
 *  Definition for Elec_Lock_IO
 */

#define elec_lock_CtrlIO_Init()     {P6SEL &= ~BIT7; P6DIR |= BIT7; P6OUT &= ~BIT7;}
#define elec_lock_CtrlIO_On()       {P6OUT |= BIT7;}
#define elec_lock_CtrlIO_Off()      {P6OUT &= ~BIT7;}
//
#define elec_lock_StateIO_Init()      {P1SEL &= ~BIT0; P1DIR &= ~BIT0; P1OUT |= BIT0;P1IES |= BIT0; P1IFG &= ~BIT0; P1IE |= BIT0;}
#define elec_lock_StateIO_In()        (P1IN  & BIT0)
#define elec_lock_StateIO_Int_Clear() {P1IFG &= ~BIT0;}
#define elec_lock_StateIO_Int_Get()   (P1IFG & BIT0)
#define elec_lock_StateIO_Int_En()    {P1IFG &= ~BIT0; P1IE |= BIT0;}
#define elec_lock_StateIO_Int_Dis()   {P1IE &= ~BIT0;  P1IFG &= ~BIT0;}


/**
 *  Definition for Elec_Lock_State
 */ 
typedef enum
{
  lock_Open = 0,            /*!< elec lock open       */
  lock_Close                /*!< elec lock close       */
}elec_lock_State_t; 


#ifndef ELEC_LOCK_GLO
#define ELEC_LOCK_Ext  extern
#else
#define ELEC_LOCK_Ext
#endif

ELEC_LOCK_Ext elec_lock_State_t elec_lock_State;


void elec_lock_init(void);
bool_t elec_lock_open(void);
void elec_lock_close(void);
bool_t elec_lock_state(void);


#endif
