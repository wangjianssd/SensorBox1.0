/**
 * @brief       : this
 * @file        : si4463_arch.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-12-03
 * change logs  :
 * Date       Version     Author        Note
 * 2015-12-03  v0.0.1  gang.cheng    first version
 */
#ifndef __SI4463_ARCH_H__
#define __SI4463_ARCH_H__


/** rf spi */
#define RF_SPI                          SPI1
#define RF_SPI_CLK_RCC(x)               __HAL_RCC_SPI1_CLK_##x()
#define RF_SPI_SCK_RCC(x)               __HAL_RCC_GPIOE_CLK_##x()
#define RF_SPI_MOSI_RCC(x)              __HAL_RCC_GPIOE_CLK_##x()
#define RF_SPI_MISO_RCC(x)              __HAL_RCC_GPIOE_CLK_##x()

#define RF_SPI_FORCE_RESET()            __HAL_RCC_SPI1_FORCE_RESET()
#define RF_SPI_RELEASE_RESET()          __HAL_RCC_SPI1_RELEASE_RESET()

#define RF_SPI_SCK_PIN                  GPIO_PIN_13
#define RF_SPI_SCK_PORT            		GPIOE
#define RF_SPI_SCK_AF                   GPIO_AF5_SPI1
#define RF_SPI_MISO_PIN                 GPIO_PIN_14
#define RF_SPI_MISO_PORT           		GPIOE
#define RF_SPI_MISO_AF                  GPIO_AF5_SPI1
#define RF_SPI_MOSI_PIN                 GPIO_PIN_15
#define RF_SPI_MOSI_PORT           		GPIOE
#define RF_SPI_MOSI_AF                  GPIO_AF5_SPI1

#define RF_SPI_NSS_PIN                  GPIO_PIN_12
#define RF_SPI_NSS_PORT            		GPIOE
#define RF_SPI_NSS_RCC(x)               __HAL_RCC_GPIOE_CLK_##x()

/** rf sdn */
#define RF_PORT_SDN_RCC(x)              __HAL_RCC_GPIOB_CLK_##x()
#define RF_PORT_SDN_PIN                 GPIO_PIN_1
#define RF_PORT_SDN_PORT                GPIOB
#define RF_PORT_SDN_HIGH()              HAL_GPIO_WritePin(RF_PORT_SDN_PORT,    \
                                                    RF_PORT_SDN_PIN,           \
                                                    GPIO_PIN_SET)
#define RF_PORT_SDN_LOW()               HAL_GPIO_WritePin(RF_PORT_SDN_PORT,    \
                                                    RF_PORT_SDN_PIN,           \
                                                    GPIO_PIN_RESET)

/** rf sw */
#define RF_PORT_SW_TX_PIN               GPIO_PIN_8
#define RF_PORT_SW_RX_PIN               GPIO_PIN_7
#define RF_PORT_SW_PORT                 GPIOE
#define RF_PORT_SW_RCC(x)               __HAL_RCC_GPIOE_CLK_##x()

/** rf unb */
#define RF_PORT_UNB_PIN                  GPIO_PIN_5
#define RF_PORT_UNB_PORT                GPIOE
#define RF_PORT_UNB_RCC(x)               __HAL_RCC_GPIOE_CLK_##x()


/** rf irq */
#define RF_PORT_IRQ_IRQn                EXTI15_10_IRQn
#define RF_PORT_IRQ_PIN                 GPIO_PIN_11
#define RF_PORT_IRQ_PORT                GPIOE
#define RF_PORT_IRQ_RCC(x)              __HAL_RCC_GPIOE_CLK_##x()
#define RF_PORT_IRQ_IN                  (RF_PORT_IRQ_PORT->IDR & RF_PORT_IRQ_PIN)

/**
 * @group radio spi module
 */

#define radio_SpiWriteByte  radio_spi_write_byte
#define radio_SpiReadByte   radio_spi_read_byte


void radio_spi_init(void);

void radio_set_nsel(void);

void radio_clear_nsel(void);

uint8_t radio_spi_read_byte(uint8_t data);

void radio_spi_write_byte(uint8_t data);

void radio_SpiReadData(uint8_t len, uint8_t *data);

void radio_SpiWriteData(uint8_t len, uint8_t *data);



void radio_AssertShutdown(void);

void radio_DeassertShutdown(void);



void radio_4259_tx(void);

void radio_4259_rx(void);

void radio_4259_idle(void);



void radio_unb_on(void);

void radio_unb_off(void);



void radio_port_init(void);



void radio_nirq_init(void);

bool_t radio_get_NirqLevel(void);

void radio_nirq_enable(void);

void radio_nirq_disable(void);

#endif
 