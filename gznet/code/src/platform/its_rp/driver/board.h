/**
 * @brief       : provides an abstraction for peripheral device.
 *
 * @file        : board.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __BOARD_H
#define __BOARD_H

#include <node_cfg.h>

#define BLUE              (1u)
#define RED               (2u)
#define GREEN             (3u)


#define LED_OPEN(color)   ((color) == BLUE) ? (P1OUT &= ~BIT5):(((color) == RED) ? (P1OUT &= ~BIT6):(P1OUT &= ~BIT7))
#define LED_CLOSE(color)  ((color) == BLUE) ? (P1OUT |= BIT5):(((color) == RED) ? (P1OUT |= BIT6):(P1OUT |= BIT7))
#define LED_TOGGLE(color) ((color) == BLUE) ? (P1OUT ^= BIT5):(((color) == RED) ? (P1OUT ^= BIT6):(P1OUT ^= BIT7))


void led_init(void);

/**
 * reset system
 */
void board_reset(void);

/**
 * Initializes mcu clock, peripheral device and enable globle interrupt
 */
void board_init(void);

void srand_arch(void);

void lpm_arch(void);

#endif

/**
 * @}
 */

