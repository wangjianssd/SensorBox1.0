/**
 * @brief       : 
 *
 * @file        : gpio.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"

static const uint16_t ptin [] = { P1IN_,  P2IN_,  P3IN_,  P4IN_,  P5IN_,
                                  P6IN_, P7IN_, P8IN_};
static const uint16_t ptout[] = { P1OUT_, P2OUT_, P3OUT_, P4OUT_, P5OUT_,
                                  P6OUT_, P7OUT_, P8OUT_};
static const uint16_t ptdir[] = { P1DIR_, P2DIR_, P3DIR_, P4DIR_, P5DIR_,
                                  P6DIR_, P7DIR_, P8DIR_};
static const uint16_t ptsel[] = { P1SEL_, P2SEL_, P3SEL_, P4SEL_, P5SEL_,
                                  P6SEL_, P7SEL_, P8SEL_};
static const uint16_t pties[] = { P1IES_, P2IES_  };
static const uint16_t ptie [] = { P1IE_,  P2IE_   };
static const uint16_t ptifg[] = { P1IFG_, P2IFG_  };

void gpio_set(pin_id_t pin_id)
{
    POUT(pin_id.pin-1) |= (0x01 << pin_id.bit);
}

void gpio_clr(pin_id_t pin_id)
{
    POUT(pin_id.pin-1) &= ~(0x01 << pin_id.bit);
}

void gpio_toggle(pin_id_t pin_id)
{
    POUT(pin_id.pin-1) ^= (0x01 << pin_id.bit);
}

void gpio_make_input(pin_id_t pin_id)
{
    PDIR(pin_id.pin-1) &= ~(0x01 << pin_id.bit);
}

bool_t gpio_is_input(pin_id_t pin_id)
{
    return((PDIR(pin_id.pin-1) & (0x01 << pin_id.bit)) == 0);
}

void gpio_make_output(pin_id_t pin_id)
{
    PDIR(pin_id.pin-1) |= (0x01 << pin_id.bit);
}

bool_t gpio_is_output(pin_id_t pin_id)
{
    return ((PDIR(pin_id.pin-1) & (0x01 << pin_id.bit)) != 0);
}

void gpio_sel(pin_id_t pin_id)
{
    PSEL(pin_id.pin-1) |= (0x01 << pin_id.bit);
}

bool_t gpio_get(pin_id_t pin_id)
{
    return  ((PIN(pin_id.pin-1))&(0x01 << pin_id.bit));
}

void gpio_interrupt_enable(pin_id_t pin_id)
{
    PIE(pin_id.pin-1) |= (0x01 << pin_id.bit);
}

void gpio_interrupt_disable(pin_id_t pin_id)
{
    PIE(pin_id.pin-1) &= ~(0x01 << pin_id.bit);
}

void gpio_interrupt_edge(pin_id_t pin_id, uint8_t edge)
{
    if (edge == 0)
    {
        PIES(pin_id.pin-1) &= ~(0x01 << pin_id.bit);
    }
    else
    {
        PIES(pin_id.pin-1) |= (0x01 << pin_id.bit);
    }
}

void gpio_interrupt_clear(pin_id_t pin_id)
{
    PIFG(pin_id.pin-1) &= ~(0x01 << pin_id.bit);
}

bool_t gpio_init(void)
{
    return TRUE;
}

#pragma vector = PORT2_VECTOR
__interrupt void port_2_isr(void)
{
    OSEL_ISR_ENTRY();

    OSEL_ISR_EXIT();
    LPM3_EXIT;
}
