#include "common/lib/lib.h"
#include "driver.h"

TIM_HandleTypeDef  rtim;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	__HAL_RCC_TIM9_CLK_ENABLE();
	/* Set the TIMx priority */
	HAL_NVIC_SetPriority(TIM9_IRQn, 0, 0);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM9_IRQn);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		extern void htimer_int_handler(void);
		htimer_int_handler();
	}
}

void rtimer_start(void)
{
	TIM_OC_InitTypeDef sOCConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	rtim.Instance				= TIM9;
	rtim.Init.Period            = 65535;
	rtim.Init.Prescaler         = 0;
	rtim.Init.ClockDivision     = 0;
	rtim.Init.CounterMode       = TIM_COUNTERMODE_UP;

	if (HAL_TIM_Base_Init(&rtim) != HAL_OK)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}

	/* Select TIM9 clock source */
	TIM_ClockConfigTypeDef	sClockSourceConfig;
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	HAL_TIM_ConfigClockSource(&rtim, &sClockSourceConfig);

	HAL_TIM_OC_Init(&rtim);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&rtim, &sMasterConfig);

	sOCConfig.OCMode			= TIM_OCMODE_TIMING;
	sOCConfig.OCIdleState		= TIM_OCIDLESTATE_RESET;
	sOCConfig.OCFastMode		= TIM_OCFAST_DISABLE;
	sOCConfig.OCPolarity		= TIM_OCPOLARITY_HIGH;
	sOCConfig.Pulse				= 1;
	if (HAL_TIM_OC_ConfigChannel(&rtim, &sOCConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}

#if OVERFLOW_INT_EN == 0
	__HAL_TIM_ENABLE(&rtim);
#else
	HAL_TIM_Base_Start_IT(&rtim);
#endif

	HTIMER_COMPINT_DISABLE();


	// GPIO_InitTypeDef  gpio_init_struct = {0};
	// __HAL_RCC_GPIOA_CLK_ENABLE();

	// gpio_init_struct.Pin = GPIO_PIN_11;
	// gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
	// gpio_init_struct.Pull   = GPIO_NOPULL;
	// gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
	// HAL_GPIO_Init(GPIOA, &gpio_init_struct);

	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
}










