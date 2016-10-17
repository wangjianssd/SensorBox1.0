#include "common/lib/lib.h"
#include "driver.h"
#include "sys_arch/osel_arch.h"
#include <stdlib.h>

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    wsnos_ticks();
}

static void systick_init(uint16_t ms)
{
    RTC_HandleTypeDef RtcHandle;
    
    __HAL_RCC_PWR_CLK_ENABLE();                         //*< Enable the PWR clock

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState            = RCC_LSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    /* Configure the RTC data register and RTC prescaler */
    RtcHandle.Instance = RTC;
    RtcHandle.Init.AsynchPrediv = 0x7F;
    RtcHandle.Init.SynchPrediv  = 0xFF;
    RtcHandle.Init.HourFormat   = RTC_HOURFORMAT_24;
    HAL_RTC_Init(&RtcHandle);

    /* Enable the RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    /* Wait for RTC APB registers synchronisation */
    //RTC_WaitForSynchro(); //HAL_RTC_WaitForSynchro(&RtcHandle);
    /* EXTI configuration *******************************************************/
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

    /* Enable the RTC Wakeup Interrupt */
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

    /* Configure the RTC WakeUp Clock source: RTC/div16 (488.28us/tick) */
    /* Enable the RTC Wakeup Interrupt */
    /* Enable Wakeup Counter */
    uint16_t ticks = (ms * 1000)/488;
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, ticks, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}

void board_init(void)
{
//    gpio_lowpower_init(void);
    clk_init(SYSCLK_8MHZ);
    systick_init(OSEL_TICK_PER_MS);
}

void led_init(void)
{
    GPIO_InitTypeDef  gpio_init_struct = {0};
    __HAL_RCC_GPIOE_CLK_ENABLE();

    gpio_init_struct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_NOPULL;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
}


void energy_init(void)
{
    ;
}

uint8_t energy_get(void)
{
    return 100;
}


void srand_arch(void)
{
    srand(__HAL_TIM_GET_COUNTER(&rtim));
}

void lpm_arch(void)
{
    //@noto 如果用仿真器调试，关闭低功耗，否则仿真器无法正常调试
    //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void delay_us(uint32_t us)
{
    for(uint32_t i=0;i<us;i++)
    {
        for(uint32_t j=0;j<8;j++)
        {
            ;
        }
    }
}

