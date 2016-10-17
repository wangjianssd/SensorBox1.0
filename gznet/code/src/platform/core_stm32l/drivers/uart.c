#include "common/lib/lib.h"
#include "driver.h"

#define UART1_RCC_ENABLE()          \
    __HAL_RCC_GPIOD_CLK_ENABLE();   \
    __HAL_RCC_USART2_CLK_ENABLE();

#define UART2_RCC_ENABLE()          \
    __HAL_RCC_GPIOD_CLK_ENABLE();   \
    __HAL_RCC_USART3_CLK_ENABLE();

#define UART3_RCC_ENABLE()          \
    __HAL_RCC_GPIOA_CLK_ENABLE();   \
    __HAL_RCC_USART1_CLK_ENABLE();

#define UART4_RCC_ENABLE()          \
    __HAL_RCC_GPIOC_CLK_ENABLE();   \
    __HAL_RCC_UART4_CLK_ENABLE();

static uart_interupt_cb_t uart_interrupt_cb = NULL;

struct stm32_uart
{
    USART_TypeDef *uart_device;
    GPIO_TypeDef *uart_port;
    UART_HandleTypeDef uart_handle;
    IRQn_Type irq;
    uint8_t gpio_af;
    uint16_t tx_pin;
    uint16_t rx_pin;
};

struct stm32_uart uart1 =
{
    USART2,
    GPIOD,
    {0},
    USART2_IRQn,
    GPIO_AF7_USART2,
    GPIO_PIN_5,
    GPIO_PIN_6
};

struct stm32_uart uart2 =
{
    USART3,
    GPIOD,
    {0},
    USART3_IRQn,
    GPIO_AF7_USART3,
    GPIO_PIN_8,
    GPIO_PIN_9
};

struct stm32_uart uart3 =
{
    USART1,
    GPIOA,
    {0},
    USART1_IRQn,
    GPIO_AF7_USART1,
    GPIO_PIN_10,
    GPIO_PIN_11
};

struct stm32_uart uart4 =
{
    UART4,
    GPIOC,
    {0},
    UART4_IRQn,
    GPIO_AF8_UART4,
    GPIO_PIN_10,
    GPIO_PIN_11
};

static void RCC_Configuration(struct stm32_uart *uart)
{
    switch (uart->irq)
    {
    case USART1_IRQn:
        UART3_RCC_ENABLE();
        break;

    case USART2_IRQn:
        UART1_RCC_ENABLE();
        break;

    case USART3_IRQn:
        UART2_RCC_ENABLE();
        break;

    case UART4_IRQn:
        UART4_RCC_ENABLE();
        break;

    default:
        break;
    }
}

static void GPIO_Configuration(struct stm32_uart *uart)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Alternate = uart->gpio_af;
    GPIO_InitStructure.Pin       = uart->rx_pin | uart->tx_pin;
    HAL_GPIO_Init(uart->uart_port, &GPIO_InitStructure);
}

static void NVIC_Configuration(struct stm32_uart *uart)
{
    HAL_NVIC_SetPriority(uart->irq, 2, 0);
    HAL_NVIC_EnableIRQ(uart->irq);
}


static void UART_Configuration(struct stm32_uart *uart, uint32_t baud)
{
    uart->uart_handle.Instance        = uart->uart_device;
    uart->uart_handle.Init.BaudRate   = baud;
    uart->uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart->uart_handle.Init.StopBits   = UART_STOPBITS_1;
    uart->uart_handle.Init.Parity     = UART_PARITY_NONE;
    uart->uart_handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    uart->uart_handle.Init.Mode       = UART_MODE_TX_RX;
    HAL_UART_Init(&(uart->uart_handle));

    /* enable interrupt */
    __HAL_USART_ENABLE_IT(&(uart->uart_handle), USART_IT_RXNE);
    __HAL_USART_CLEAR_FLAG(&(uart->uart_handle), USART_FLAG_RXNE);
}

void uart_init(uint8_t uart_id, uint32_t baud_rate)
{
    switch (uart_id)
    {
    case UART_1:
        RCC_Configuration(&uart1);
        GPIO_Configuration(&uart1);
        NVIC_Configuration(&uart1);
        UART_Configuration(&uart1, baud_rate);
        break;

    case UART_2:
        RCC_Configuration(&uart2);
        GPIO_Configuration(&uart2);
        NVIC_Configuration(&uart2);
        UART_Configuration(&uart2, baud_rate);
        break;

    case UART_3:
        RCC_Configuration(&uart3);
        GPIO_Configuration(&uart3);
        NVIC_Configuration(&uart3);
        UART_Configuration(&uart3, baud_rate);
        break;

    case UART_4:
        RCC_Configuration(&uart4);
        GPIO_Configuration(&uart4);
        NVIC_Configuration(&uart4);
        UART_Configuration(&uart4, baud_rate);
        break;

    default:
        break;
    }
}

void uart_send_char(uint8_t id, uint8_t value)
{
    switch (id)
    {
    case UART_1:
        uart1.uart_device->DR = (value & (uint16_t)0x00FF);
        while (RESET == __HAL_UART_GET_FLAG(&(uart1.uart_handle), USART_FLAG_TXE));
        break;

    case UART_2:
        uart2.uart_device->DR = (value & (uint16_t)0x00FF);
        while (RESET == __HAL_UART_GET_FLAG(&(uart2.uart_handle), USART_FLAG_TXE));
        break;

    case UART_3:
        uart3.uart_device->DR = (value & (uint16_t)0x00FF);
        while (RESET == __HAL_UART_GET_FLAG(&(uart3.uart_handle), USART_FLAG_TXE));
        break;

    case UART_4:
        uart4.uart_device->DR = (value & (uint16_t)0x00FF);
        while (RESET == __HAL_UART_GET_FLAG(&(uart4.uart_handle), USART_FLAG_TXE));
        break;

    default:
        break;
    }
}

void uart_send_string(uint8_t id, uint8_t *string, uint16_t length)
{
    uint16_t i = 0;
    for (i = 0; i < length; i++)
    {
        uart_send_char(id, string[i]);
    }
}

bool_t uart_enter_q(uint8_t id, uint8_t c)
{
    return TRUE;
}

bool_t uart_string_enter_q(uint8_t uart_id, uint8_t *string, uint16_t length)
{
    return TRUE;
}

bool_t uart_del_q(uint8_t uart_id, uint8_t *c_p)
{
    return TRUE;
}

void uart_clear_rxbuf(uint8_t id)
{
    ;
}

void uart_recv_enable(uint8_t uart_id)
{
    switch (uart_id)
    {
    case UART_1:
        HAL_NVIC_EnableIRQ(uart1.irq);
        break;

    case UART_2:
        HAL_NVIC_EnableIRQ(uart2.irq);
        break;

    case UART_3:
        HAL_NVIC_EnableIRQ(uart3.irq);
        break;

    case UART_4:
        HAL_NVIC_EnableIRQ(uart4.irq);
        break;

    default:
        break;
    }
}

void uart_recv_disable(uint8_t uart_id)
{
    switch (uart_id)
    {
    case UART_1:
        HAL_NVIC_DisableIRQ(uart1.irq);
        break;

    case UART_2:
        HAL_NVIC_DisableIRQ(uart2.irq);
        break;

    case UART_3:
        HAL_NVIC_DisableIRQ(uart3.irq);
        break;

    case UART_4:
        HAL_NVIC_DisableIRQ(uart4.irq);
        break;

    default:
        break;
    }
}

void uart_int_cb_reg(uart_interupt_cb_t cb)
{
    if (cb != NULL)
    {
        uart_interrupt_cb = cb;
    }
}

static void uart_int_cb_handle(uint8_t id, uint8_t ch)
{
    if (uart_interrupt_cb != NULL)
    {
        uart_interrupt_cb(id, ch);
    }
}


void USART1_IRQHandler(void)
{
    OSEL_ISR_ENTRY();

    if (__HAL_UART_GET_IT_SOURCE(&(uart3.uart_handle), UART_IT_RXNE) != RESET)
    {
        uint8_t ch = (uint16_t)(uart3.uart_device->DR & (uint16_t)0x00FF);
        uart_int_cb_handle(UART_3, ch);
        /* clear interrupt */
        //__HAL_USART_CLEAR_FLAG(&(uart->uart_handle), USART_FLAG_RXNE);
    }

    OSEL_ISR_EXIT();
}

void USART2_IRQHandler(void)
{
    OSEL_ISR_ENTRY();

    if (__HAL_UART_GET_IT_SOURCE(&(uart1.uart_handle), UART_IT_RXNE) != RESET)
    {
        uint8_t ch = (uint16_t)(uart1.uart_device->DR & (uint16_t)0x00FF);
        uart_int_cb_handle(UART_1, ch);
        /* clear interrupt */
        //__HAL_USART_CLEAR_FLAG(&(uart->uart_handle), USART_FLAG_RXNE);
    }

    OSEL_ISR_EXIT();
}

void USART3_IRQHandler(void)
{
    OSEL_ISR_ENTRY();

    if (__HAL_UART_GET_IT_SOURCE(&(uart2.uart_handle), UART_IT_RXNE) != RESET)
    {
        uint8_t ch = (uint16_t)(uart2.uart_device->DR & (uint16_t)0x00FF);
        uart_int_cb_handle(UART_2, ch);
        /* clear interrupt */
        //__HAL_USART_CLEAR_FLAG(&(uart->uart_handle), USART_FLAG_RXNE);
    }

    OSEL_ISR_EXIT();
}

void UART4_IRQHandler(void)
{
    OSEL_ISR_ENTRY();

    if (__HAL_UART_GET_IT_SOURCE(&(uart4.uart_handle), UART_IT_RXNE) != RESET)
    {
        uint8_t ch = (uint16_t)(uart4.uart_device->DR & (uint16_t)0x00FF);
        uart_int_cb_handle(UART_4, ch);
        /* clear interrupt */
        //__HAL_USART_CLEAR_FLAG(&(uart->uart_handle), USART_FLAG_RXNE);
    }

    OSEL_ISR_EXIT();
}