#include "common/lib/lib.h"
#include "./driver.h"

SPI_HandleTypeDef SI4463_SPI_Handle;


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	 GPIO_InitTypeDef  GPIO_InitStruct;
	
	 RF_SPI_CLK_RCC(ENABLE);
	 RF_SPI_SCK_RCC(ENABLE);
	 RF_SPI_MOSI_RCC(ENABLE);
	 RF_SPI_MISO_RCC(ENABLE);
	 
	/* Configure SPI SCK */
    GPIO_InitStruct.Pin       = RF_SPI_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = RF_SPI_SCK_AF;
	HAL_GPIO_Init(RF_SPI_SCK_PORT, &GPIO_InitStruct);
	
	/* Configure SPI MISO and MOSI */ 
    GPIO_InitStruct.Pin       = RF_SPI_MISO_PIN;
    GPIO_InitStruct.Alternate = RF_SPI_MISO_AF;
	HAL_GPIO_Init(RF_SPI_MISO_PORT, &GPIO_InitStruct);

	/* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin       = RF_SPI_MOSI_PIN;
    GPIO_InitStruct.Alternate = RF_SPI_MOSI_AF;
	HAL_GPIO_Init(RF_SPI_MOSI_PORT, &GPIO_InitStruct);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_SPI_MspDeInit could be implenetd in the user file
   */
	GPIO_InitTypeDef  GPIO_InitStruct;
	
	HAL_GPIO_DeInit(RF_SPI_SCK_PORT, RF_SPI_SCK_PIN);
	HAL_GPIO_DeInit(RF_SPI_MISO_PORT, RF_SPI_MISO_PIN);
	HAL_GPIO_DeInit(RF_SPI_MOSI_PORT, RF_SPI_MOSI_PIN);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pin = RF_SPI_SCK_PIN | RF_SPI_MOSI_PIN;
	HAL_GPIO_Init(RF_SPI_SCK_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(RF_SPI_SCK_PORT, RF_SPI_MISO_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RF_SPI_MOSI_PORT, RF_SPI_MOSI_PIN, GPIO_PIN_RESET);
	
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pin = RF_SPI_MISO_PIN;
	HAL_GPIO_Init(RF_SPI_MISO_PORT, &GPIO_InitStruct);
	
	__HAL_RCC_SPI1_FORCE_RESET();
	__HAL_RCC_SPI1_RELEASE_RESET();
}

void radio_spi_init(void)
{
    SI4463_SPI_Handle.Instance               = RF_SPI;

    SI4463_SPI_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    SI4463_SPI_Handle.Init.Direction         = SPI_DIRECTION_2LINES;
    SI4463_SPI_Handle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SI4463_SPI_Handle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SI4463_SPI_Handle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SI4463_SPI_Handle.Init.CRCPolynomial     = 7;
    SI4463_SPI_Handle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SI4463_SPI_Handle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SI4463_SPI_Handle.Init.NSS               = SPI_NSS_SOFT;
    SI4463_SPI_Handle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SI4463_SPI_Handle.Init.Mode              = SPI_MODE_MASTER;

    if (HAL_SPI_Init(&SI4463_SPI_Handle) != HAL_OK)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    __HAL_SPI_ENABLE(&SI4463_SPI_Handle);

    if (HAL_SPI_GetState(&SI4463_SPI_Handle) != HAL_SPI_STATE_READY)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    GPIO_InitTypeDef GPIO_InitStructure;

    RF_SPI_NSS_RCC(ENABLE);
    GPIO_InitStructure.Pin   = RF_SPI_NSS_PIN;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructure.Pull  =  GPIO_NOPULL;
    HAL_GPIO_Init(RF_SPI_NSS_PORT, &GPIO_InitStructure);
    radio_set_nsel();
}

void radio_set_nsel(void)
{
    HAL_GPIO_WritePin(RF_SPI_NSS_PORT, RF_SPI_NSS_PIN, GPIO_PIN_SET);
}

void radio_clear_nsel(void)
{
    HAL_GPIO_WritePin(RF_SPI_NSS_PORT, RF_SPI_NSS_PIN, GPIO_PIN_RESET);
}

uint8_t radio_spi_read_byte(uint8_t data)
{
    uint8_t pRxData = 0;
	
	while (HAL_SPI_GetState(&SI4463_SPI_Handle) == HAL_SPI_STATE_BUSY_RX) {
		/*Do nothing*/
	}
	
	if (HAL_SPI_Receive(&SI4463_SPI_Handle, &pRxData, 1, 0xFFFf) != HAL_OK) {
		DBG_ASSERT(FALSE __DBG_LINE);
	}
	
	return pRxData;
}

void radio_spi_write_byte(uint8_t data)
{
    while (HAL_SPI_GetState(&SI4463_SPI_Handle) == HAL_SPI_STATE_BUSY_TX) {
		/*Do nothing*/
	}
	
	if (HAL_SPI_Transmit(&SI4463_SPI_Handle, &data, 1, 0xFFFf) != HAL_OK) {
		DBG_ASSERT(FALSE __DBG_LINE);
	}
}

void radio_SpiReadData(uint8_t len, uint8_t *data)
{
    while (HAL_SPI_GetState(&SI4463_SPI_Handle) == HAL_SPI_STATE_BUSY_RX) {
		/*Do nothing*/
	}
	
	if (HAL_SPI_Receive(&SI4463_SPI_Handle, data, len, 0xFFFF) != HAL_OK) {
		DBG_ASSERT(FALSE __DBG_LINE);
	}
}

void radio_SpiWriteData(uint8_t len, uint8_t *data)
{
    while (HAL_SPI_GetState(&SI4463_SPI_Handle) == HAL_SPI_STATE_BUSY_TX) {
		/*Do nothing*/
	}
	
	if (HAL_SPI_Transmit(&SI4463_SPI_Handle, data, len, 0xFFFF) != HAL_OK) {
		DBG_ASSERT(FALSE __DBG_LINE);
	}
}

static void radio_shundown_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RF_PORT_SDN_RCC(ENABLE);

    GPIO_InitStructure.Pin = RF_PORT_SDN_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(RF_PORT_SDN_PORT , &GPIO_InitStructure);
    RF_PORT_SDN_HIGH();
}

void radio_AssertShutdown(void)
{
    uint32_t tick = 0;
    RF_PORT_SDN_HIGH();
    for (tick = 128; tick > 0; tick-- )
    {
        ;
    }
}

void radio_DeassertShutdown(void)
{
    uint32_t tick = 0;
    RF_PORT_SDN_LOW();
    for (tick = 128; tick > 0; tick-- )
    {
        ;
    }
}

static void radio_4259_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RF_PORT_SW_RCC(ENABLE);

    GPIO_InitStructure.Pin = RF_PORT_SW_TX_PIN | RF_PORT_SW_RX_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RF_PORT_SW_PORT, &GPIO_InitStructure);
}

void radio_4259_tx(void)
{
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_TX_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_RX_PIN, GPIO_PIN_SET);
}

void radio_4259_rx(void)
{
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_TX_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_RX_PIN, GPIO_PIN_RESET);
}

void radio_4259_idle(void)
{
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_TX_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_PORT_SW_PORT, RF_PORT_SW_RX_PIN, GPIO_PIN_RESET);
}

static void radio_unb_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RF_PORT_UNB_RCC(ENABLE);

    GPIO_InitStructure.Pin = RF_PORT_UNB_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RF_PORT_UNB_PORT, &GPIO_InitStructure);
}

void radio_unb_on(void)
{
    HAL_GPIO_WritePin(RF_PORT_UNB_PORT, RF_PORT_UNB_PIN, GPIO_PIN_SET);
}

void radio_unb_off(void)
{
    HAL_GPIO_WritePin(RF_PORT_UNB_PORT, RF_PORT_UNB_PIN, GPIO_PIN_RESET);
}

void radio_port_init(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    radio_shundown_init();
    radio_4259_init();
    radio_unb_init();
}

void radio_nirq_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RF_PORT_IRQ_RCC(ENABLE);

    GPIO_InitStructure.Pin = RF_PORT_IRQ_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RF_PORT_IRQ_PORT, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(RF_PORT_IRQ_IRQn, 0, 0);
}

bool_t radio_get_NirqLevel(void)
{
    return RF_PORT_IRQ_IN ? 1 : 0;
}

void radio_nirq_enable(void)
{
    HAL_NVIC_EnableIRQ(RF_PORT_IRQ_IRQn);
}

void radio_nirq_disable(void)
{
    HAL_NVIC_DisableIRQ(RF_PORT_IRQ_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == RF_PORT_IRQ_PIN)
    {
        extern void rf_int_handler(uint16_t time);
        rf_int_handler(__HAL_TIM_GET_COUNTER(&rtim));
    }
}

