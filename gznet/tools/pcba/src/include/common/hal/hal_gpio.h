/**
 * @brief       : provides an abstraction for peripheral device.
 *
 * @file        : hal_gpio.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#include <data_type_def.h>
#include <driver.h>

#define HAL_CTL_1           st( hal_pin_id.pin = 8; hal_pin_id.bit = 4; ) //P4.6
#define HAL_CTL_2           st( hal_pin_id.pin = 8; hal_pin_id.bit = 3; ) //P4.5
#define HAL_CTL_3           st( hal_pin_id.pin = 8; hal_pin_id.bit = 2; )
#define HAL_CTL_4           st( hal_pin_id.pin = 8; hal_pin_id.bit = 1; )
#define HAL_CTL_5           st( hal_pin_id.pin = 8; hal_pin_id.bit = 0; )
#define HAL_CTL_6           st( hal_pin_id.pin = 7; hal_pin_id.bit = 3; )
#define HAL_CTL_7           st( hal_pin_id.pin = 7; hal_pin_id.bit = 2; )
#define HAL_CTL_8           st( hal_pin_id.pin = 4; hal_pin_id.bit = 7; )
#define HAL_CTL_9           st( hal_pin_id.pin = 4; hal_pin_id.bit = 6; )
#define HAL_CTL_10          st( hal_pin_id.pin = 4; hal_pin_id.bit = 5; )
#define HAL_CTL_11          st( hal_pin_id.pin = 4; hal_pin_id.bit = 4; )
#define HAL_CTL_12          st( hal_pin_id.pin = 4; hal_pin_id.bit = 3; )
#define HAL_CTL_13          st( hal_pin_id.pin = 4; hal_pin_id.bit = 2; )
#define HAL_CTL_14          st( hal_pin_id.pin = 4; hal_pin_id.bit = 1; )
#define HAL_CTL_15          st( hal_pin_id.pin = 4; hal_pin_id.bit = 0; )
#define HAL_CTL_16          st( hal_pin_id.pin = 3; hal_pin_id.bit = 7; )
#define HAL_CTL_17          st( hal_pin_id.pin = 3; hal_pin_id.bit = 6; )
#define HAL_CTL_18          st( hal_pin_id.pin = 3; hal_pin_id.bit = 5; )
#define HAL_CTL_19          st( hal_pin_id.pin = 3; hal_pin_id.bit = 4; )
#define HAL_CTL_20          st( hal_pin_id.pin = 2; hal_pin_id.bit = 7; )
#define HAL_CTL_21          st( hal_pin_id.pin = 2; hal_pin_id.bit = 6; )
#define HAL_CTL_22          st( hal_pin_id.pin = 2; hal_pin_id.bit = 5; )
#define HAL_CTL_23          st( hal_pin_id.pin = 2; hal_pin_id.bit = 4; )
#define HAL_CTL_24          st( hal_pin_id.pin = 2; hal_pin_id.bit = 3; )
#define HAL_CTL_25          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_26          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_27          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_28          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_29          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_30          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_31          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_32          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_33          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_34          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_35          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )
#define HAL_CTL_36          st( hal_pin_id.pin = 1; hal_pin_id.bit = 3; )

typedef pin_id_t            hal_pin_id_t;

/**
 * pullup the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_set(hal_pin_id_t pin_id);

/**
 * pulldown the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_clr(hal_pin_id_t pin_id);

/**
 * toggle the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_toggle(hal_pin_id_t pin_id);

/**
 * switch to input direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_make_input(hal_pin_id_t pin_id);

/**
 * judge corresponding I/O pin's direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return TRUE or FALSE
 */
bool_t hal_gpio_is_input(hal_pin_id_t pin_id);

/**
 * switch to output direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_make_output(hal_pin_id_t pin_id);

/**
 * judge corresponding I/O pin's direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return TRUE or FALSE
 */
bool_t hal_gpio_is_output(hal_pin_id_t pin_id);

/**
 * Port Select
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_sel(hal_pin_id_t pin_id);

/**
 * get the value of corresponding bit of input register
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return value of the bit
 */
bool_t hal_gpio_get(hal_pin_id_t pin_id);

/**
 * enable interrupt
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_interrupt_enable(hal_pin_id_t pin_id);

/**
 * disable interrupt
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_interrupt_disable(hal_pin_id_t pin_id);

/**
 * selects the interrupt edge for the corresponding I/O pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_interrupt_edge(hal_pin_id_t pin_id, uint8_t edge);

/**
 * clear the interrupt flag for the corresponding I/O pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void hal_gpio_interrupt_clear(hal_pin_id_t pin_id);

/**
 * init I/O
 */
bool_t hal_gpio_init(void);

#endif