/**
 * @brief       : this
 * @file        : board.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-12-03
 * change logs  :
 * Date       Version     Author        Note
 * 2015-12-03  v0.0.1  gang.cheng    first version
 */
#ifndef __BOARD_H__
#define __BOARD_H__

#define BLUE              (1u)
#define RED               (2u)
#define GREEN             (3u)

#define LED_OPEN(color)   ((color==BLUE)?(HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET)):(HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)))
#define LED_CLOSE(color)  ((color==BLUE)?(HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET)):(HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)))
#define LED_TOGGLE(color) ((color==BLUE)?(HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2)):(HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3)))

#define SYSCLK_8MHZ			(2u)

void led_init(void);

void board_init(void);

void board_reset(void);

void energy_init(void);

uint8_t energy_get(void);

void srand_arch(void);

void lpm_arch(void);

void delay_us(uint32_t us);


#endif
