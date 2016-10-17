/**
 * @brief       : provides an abstraction for general-purpose I/O.
 *
 * @file        : gpio.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __GPIO_H
#define __GPIO_H

#include "common/lib/data_type_def.h"

#define PIN(x)                  (*(volatile uint8_t*) ptin [(x)])
#define POUT(x)                 (*(volatile uint8_t*) ptout[(x)])
#define PDIR(x)                 (*(volatile uint8_t*) ptdir[(x)])
#define PSEL(x)                 (*(volatile uint8_t*) ptsel[(x)])
#define PIES(x)                 (*(volatile uint8_t*) pties[(x)])
#define PIE(x)                  (*(volatile uint8_t*) ptie [(x)])
#define PIFG(x)                 (*(volatile uint8_t*) ptifg[(x)])

typedef struct __pin_id_t
{
    uint8_t pin;
    uint8_t bit;
} pin_id_t;

/* gpio define */
#define P1IN_                   PAIN_
#define P1OUT_                  PAOUT_
#define P1DIR_                  PADIR_
#define P1SEL_                  PASEL_
#define P1IES_                  PAIES_
#define P1IE_                   PAIE_
#define P1IFG_                  PAIFG_
#define P2IN_                   (PAIN_ +1)
#define P2OUT_                  (PAOUT_+1)
#define P2DIR_                  (PADIR_+1)
#define P2SEL_                  (PASEL_+1)
#define P2IES_                  (PAIES_+1)
#define P2IE_                   (PAIE_ +1)
#define P2IFG_                  (PAIFG_+1)

#define P3IN_                   PBIN_
#define P3OUT_                  PBOUT_
#define P3DIR_                  PBDIR_
#define P3SEL_                  PBSEL_
#define P4IN_                   (PBIN_ +1)
#define P4OUT_                  (PBOUT_+1)
#define P4DIR_                  (PBDIR_+1)
#define P4SEL_                  (PBSEL_+1)

#define P5IN_                   PCIN_
#define P5OUT_                  PCOUT_
#define P5DIR_                  PCDIR_
#define P5SEL_                  PCSEL_
#define P6IN_                   (PCIN_ +1)
#define P6OUT_                  (PCOUT_+1)
#define P6DIR_                  (PCDIR_+1)
#define P6SEL_                  (PCSEL_+1)

#define P7IN_                   PDIN_
#define P7OUT_                  PDOUT_
#define P7DIR_                  PDDIR_
#define P7SEL_                  PDSEL_
#define P8IN_                   (PDIN_ +1)
#define P8OUT_                  (PDOUT_+1)
#define P8DIR_                  (PDDIR_+1)
#define P8SEL_                  (PDSEL_+1)

#define P9IN_                   PEIN_
#define P9OUT_                  PEOUT_
#define P9DIR_                  PEDIR_
#define P9SEL_                  PESEL_
#define P10IN_                  (PEIN_ + 1)
#define P10OUT_                 (PEOUT_+ 1)
#define P10DIR_                 (PEDIR_+ 1)
#define P10SEL_                 (PESEL_+ 1)

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


/**
 * @}
 */

