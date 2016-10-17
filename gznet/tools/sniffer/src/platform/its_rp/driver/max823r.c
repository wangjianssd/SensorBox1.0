#include <max823r.h>
#include <msp430.h>

void max823r_init(void)
{
    P1SEL &= ~BIT0;
    P1DIR |= (BIT0);
}

void max823r_clear(void)
{
    P1OUT ^= BIT0;
}
