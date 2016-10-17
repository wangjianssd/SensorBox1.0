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

//#define USE_Fingerprints
#define USE_EXTERN_LOCK
/**
 *  Definition for Elec_Lock_IO
 */

//#define elec_lock_CtrlIO_Init()     {P6SEL &= ~BIT7; P6DIR |= BIT7; P6OUT &= ~BIT7;}
//#define elec_lock_CtrlIO_On()       {P6OUT |= BIT7;}
//#define elec_lock_CtrlIO_Off()      {P6OUT &= ~BIT7;}
//#define elec_lock_CtrlIO_Float()    {P6DIR &= ~BIT7;}
#ifdef USE_EXTERN_LOCK
#define EXTERN_LOCK_FROZEN_DELAY      (1000)
#define EXTERN_LOCK_AUTO_LOCK_DELAY   (8000)

#define elec_lock_CtrlIO_Init_EX() {P4SEL &= ~BIT1; P4DIR |= BIT1; P4OUT &= ~BIT1;\
                                    P6SEL &= ~BIT3; P6DIR |= BIT3; P6OUT &= ~BIT3;}
#define elec_lock_CtrlIO_On_EX()   {P6OUT &= ~BIT3; P4OUT |= BIT1;}
#define elec_lock_CtrlIO_Off_EX()  {P4OUT &= ~BIT1; P6OUT |= BIT3;}
#define elec_lock_CtrlIO_Float_EX(){P4OUT &= ~BIT1; P6OUT &= ~BIT3;}
#endif

#define elec_lock_CtrlIO_Init()	   {P9SEL &= ~BIT0; P9DIR |= BIT0; P9OUT &= ~BIT0;}
#define elec_lock_CtrlIO_On()	   {P9OUT |= BIT0;}
#define elec_lock_CtrlIO_Off()	   {P9OUT &= ~BIT0;}
#define elec_lock_CtrlIO_Float()   {P9DIR &= ~BIT0;}


     //
#ifndef USE_Fingerprints
#define elec_lock_StateIO_Init()      {P1SEL &= ~BIT0; P1DIR &= ~BIT0; }//P1OUT |= BIT0;P1IES |= BIT0; P1IFG &= ~BIT0; P1IE |= BIT0;}
#define elec_lock_StateIO_In()        (P1IN  & BIT0)
#define elec_lock_StateIO_Int_Clear() {P1IFG &= ~BIT0;}  
#define elec_lock_StateIO_Int_Get()   (P1IFG & BIT0)
#define elec_lock_StateIO_Int_En()    {P1IFG &= ~BIT0; P1IE |= BIT0;}
#define elec_lock_StateIO_Int_Dis()   {P1IE &= ~BIT0;  P1IFG &= ~BIT0;}
#else  //3.0 3.1
////#define elec_lock_StateIO_Init()      {P3SEL &= ~(BIT0 | BIT2); P3DIR &= ~(BIT0 | BIT2);\
////                                       P3OUT |= BIT0; P3OUT &= (~BIT2); P3REN |= (BIT0 | BIT2); \
////                                       P3IES |= BIT0; P3IES &= ~BIT2; P3IFG &= ~(BIT0 | BIT2); P3IE |= (BIT0 | BIT2);}

//#define elec_lock_StateIO_Init()     {P3SEL &= ~(BIT0 | BIT2); P3DIR &= ~(BIT0 | BIT2);\
//									  P3IES |= BIT0; P3IES &= ~BIT2; P3IFG &= ~(BIT0 | BIT2); P3IE |= (BIT0 | BIT2);}
									  
//#define elec_lock_StateIO_Init()     {P3SEL &= ~(BIT3 | BIT2); P3DIR &= ~(BIT3 | BIT2);\
//									  P3IES |= BIT3; P3IES &= ~BIT2; P3IFG &= ~(BIT3 | BIT2);}// P3IE |= (BIT3 | BIT2);}

#define elec_lock_StateIO_Init()     {P3SEL &= ~(BIT3 | BIT2); P3DIR &= ~(BIT3 | BIT2);\
									  P3IES |= BIT3; P3IES |= BIT2; P3IFG &= ~(BIT3 | BIT2);}// P3IE |= (BIT3 | BIT2);}
#endif

#ifdef USE_Fingerprints
//3.2
//#define elec_lock_StateIO_In()        (P3IN  & (BIT2 | BIT0))
//#define elec_lock_StateIO1_In()        (P3IN  & BIT2)
//#define elec_lock_StateIO1_Int_Clear() {P3IFG &= ~BIT2;}  
//#define elec_lock_StateIO1_Int_Get()   (P3IFG & BIT2)
//#define elec_lock_StateIO1_Int_En()    {P3IES &= ~BIT2; P3IFG &= ~BIT2; P3IE |= BIT2;}
//#define elec_lock_StateIO1_Int_Dis()   {P3IE &= ~BIT2; P3IFG &= ~BIT2;}

//3.0
//#define elec_lock_StateIO2_In()        (P3IN  & BIT0)
//#define elec_lock_StateIO2_Int_Clear() {P3IFG &= ~BIT0;}
//#define elec_lock_StateIO2_Int_Get()   (P3IFG & BIT0)
//#define elec_lock_StateIO2_Int_En()    {P3IES |= BIT0;  P3IFG &= ~BIT0; P3IE |= BIT0;}
//#define elec_lock_StateIO2_Int_Dis()   {P3IE &= ~BIT0; P3IFG &= ~BIT0;}


//3.3
#define elec_lock_StateIO_In()         (P3IN  & (BIT2 | BIT3))
#define elec_lock_StateIO1_In()        (P3IN  & BIT3)
#define elec_lock_StateIO1_Int_Clear() {P3IFG &= ~BIT3;}  
#define elec_lock_StateIO1_Int_Get()   (P3IFG & BIT3)
#define elec_lock_StateIO1_Int_En()    {P3IES &= ~BIT3; P3IFG &= ~BIT3; P3IE |= BIT3;}
#define elec_lock_StateIO1_Int_Dis()   {P3IE &= ~BIT3; P3IFG &= ~BIT3;}
#define elec_lock_StateIO1_Int_En_IF() (P3IE & BIT3)

//3.2
#define elec_lock_StateIO2_In()        (P3IN  & BIT2)
#define elec_lock_StateIO2_Int_Clear() {P3IFG &= ~BIT2;}
#define elec_lock_StateIO2_Int_Get()   (P3IFG & BIT2)
#define elec_lock_StateIO2_Int_En()    {P3IES |= BIT2;  P3IFG &= ~BIT2; P3IE |= BIT2;}
#define elec_lock_StateIO2_Int_Dis()   {P3IE &= ~BIT2; P3IFG &= ~BIT2;}
#define elec_lock_StateIO2_Int_En_IF() (P3IE & BIT2)

#endif
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

ELEC_LOCK_Ext uint8_t elec_lock_State_updata; //一个脉冲-上传一次锁状态

ELEC_LOCK_Ext elec_lock_State_t elec_lock_State;

void elec_lock_init(void);
bool_t elec_lock_open(void);
void elec_lock_close(void);
bool_t elec_lock_state(void);
#ifdef USE_Fingerprints
void elec_lock_access();
#endif

#endif
