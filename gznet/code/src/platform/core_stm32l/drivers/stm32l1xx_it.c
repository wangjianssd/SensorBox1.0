/**
  ******************************************************************************
  * @file    Project/STM32L1xx_StdPeriph_Template/stm32l1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.3
  * @date    May-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_it.h"
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "./driver.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    ;
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
// done in context_iar.s
//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }
//}

void TIM9_IRQHandler(void)
{
    OSEL_ISR_ENTRY();
    
    HAL_TIM_IRQHandler(&rtim);
    
    OSEL_ISR_EXIT();
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}
/** 
* radio exti0
*/
void EXTI0_IRQHandler(void)
{
//	EXTI_ClearITPendingBit(EXTI_Line0);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    //OSEL_ISR_ENTRY();
    
    HAL_IncTick();
    
    //OSEL_ISR_EXIT();
}

/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx_md.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles DMA Transfer Complete interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Channel1_IRQHandler(void)
{
    //DMA_ClearFlag(DMA1_IT_TC1);
}

void RTC_Alarm_IRQHandler(void)
{
    /* Check on the AlarmA falg and on the number of interrupts per Second (60*8) */
//    if(RTC_GetITStatus(RTC_IT_ALRA) != RESET) 
//    { 
//        RTC_ClearITPendingBit(RTC_IT_ALRA); /* Clear RTC AlarmA Flags */
//    }
//    
//    EXTI_ClearITPendingBit(EXTI_Line17);    /* Clear the EXTIL line 17 */
}


/**
  * @brief  This function handles RTC Auto wake-up interrupt request.
  * @param  None
  * @retval None
  */
void RTC_WKUP_IRQHandler (void)
{
    OSEL_ISR_ENTRY();
   
    RTC_HandleTypeDef RtcHandle;
    RtcHandle.Instance = RTC;
    HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);

    OSEL_ISR_EXIT();
}


void EXTI15_10_IRQHandler(void)
{
    OSEL_ISR_ENTRY();
    
    HAL_GPIO_EXTI_IRQHandler(RF_PORT_IRQ_PIN);
  
    OSEL_ISR_EXIT();
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
