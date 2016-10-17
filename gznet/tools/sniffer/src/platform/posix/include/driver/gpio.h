/**
 * @brief       : this
 * @file        : gpio.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-15
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-15  v0.0.1  gang.cheng    first version
 */
#ifndef __GPIO_H__
#define __GPIO_H__

typedef struct __pin_id_t
{
    uint8_t pin;
    uint8_t bit;
} pin_id_t;



/**
 * pullup the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_set(pin_id_t pin_id);

/**
 * pulldown the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_clr(pin_id_t pin_id);

/**
 * toggle the corresponding bit of corresponding output pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_toggle(pin_id_t pin_id);

/**
 * switch to input direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_make_input(pin_id_t pin_id);

/**
 * judge corresponding I/O pin's direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return TRUE or FALSE
 */
bool_t gpio_is_input(pin_id_t pin_id);

/**
 * switch to output direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_make_output(pin_id_t pin_id);

/**
 * judge corresponding I/O pin's direction
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return TRUE or FALSE
 */
bool_t gpio_is_output(pin_id_t pin_id);

/**
 * Port Select
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_sel(pin_id_t pin_id);

/**
 * get the value of corresponding bit of input register
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 *
 * @return value of the bit
 */
bool_t gpio_get(pin_id_t pin_id);

/**
 * enable interrupt
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_interrupt_enable(pin_id_t pin_id);

/**
 * disable interrupt
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_interrupt_disable(pin_id_t pin_id);

/**
 * selects the interrupt edge for the corresponding I/O pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_interrupt_edge(pin_id_t pin_id, uint8_t edge);

/**
 * clear the interrupt flag for the corresponding I/O pin
 *
 * @param pin_id Pin id is a double-digit,10-digit is pin, single-digit is
 * pin's bit, .e.g 12, 1 is pin and 2 is pin's bit
 */
void gpio_interrupt_clear(pin_id_t pin_id);

/**
 * init I/O
 */
bool_t gpio_init(void);

/**
 * DeInit I/O
 */
void gpio_deinit(void);

#endif
