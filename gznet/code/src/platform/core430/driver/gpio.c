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
#include "driver.h"

static const uint16_t ptin [] = { P1IN_,  P2IN_,  P3IN_,  P4IN_,  P5IN_,
                                  P6IN_, P7IN_, P8IN_};
static const uint16_t ptout[] = { P1OUT_, P2OUT_, P3OUT_, P4OUT_, P5OUT_,
                                  P6OUT_, P7OUT_, P8OUT_};
static const uint16_t ptdir[] = { P1DIR_, P2DIR_, P3DIR_, P4DIR_, P5DIR_,
                                  P6DIR_, P7DIR_, P8DIR_};
static const uint16_t ptsel[] = { P1SEL_, P2SEL_, P3SEL_, P4SEL_, P5SEL_,
                                  P6SEL_, P7SEL_, P8SEL_};
static const uint16_t pties[] = { P1IES_, P2IES_, P3IES_, P4IES_  };
static const uint16_t ptie [] = { P1IE_,  P2IE_ , P3IE_ , P4IE_   };
static const uint16_t ptifg[] = { P1IFG_, P2IFG_, P3IFG_, P4IFG_  };

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
    //@TODO 修改低功耗
    P1SEL = 0;
    P1DIR = 0;
    P2SEL &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT6 + BIT7);
    P2DIR |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT6);
    P3SEL &= ~(BIT4 + BIT5);
    P3DIR |= (BIT4 + BIT5);
    P4SEL &= ~(BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P4DIR |= (BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P6SEL &= ~(BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);
    P6DIR |= (BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);
    P6OUT &= ~BIT7;
    P7SEL &= ~(BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P7DIR |= (BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P8SEL &= ~(BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P8DIR |= (BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P9SEL &= ~(BIT6 + BIT7);
    P9DIR |= (BIT6 + BIT7);

    //P9OUT |= BIT7;//借用
    
    
    return TRUE;
}

