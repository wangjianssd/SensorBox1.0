/**
 * @brief       : 
 *
 * @file        : board.c
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
#include <stdlib.h>

void led_init(void)
{
	P1SEL &= ~BIT7;
	P1DIR |= BIT7;
    P1OUT |= BIT7;    
}

void board_reset(void)
{
    OSEL_INT_LOCK();
    WDTCTL = 0xFFFF;
}

void board_init(void)
{
    ;
}

void srand_arch(void)
{
    srand(TA0R);
}

void lpm_arch(void)
{
    LPM3;
}

