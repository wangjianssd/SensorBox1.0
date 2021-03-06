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
#include "pmap.h"
#include <stdlib.h>

void led_init(void)
{
    LDOKEYPID = 0x9628; //*< unlock
    PUCTL |= PUOPE; //*< set PU.0 PU.1 to output
    
    LED_CLOSE(BLUE);
    LED_CLOSE(GREEN);
    LED_CLOSE(RED);

	LED_OPEN(BLUE);
    LED_OPEN(GREEN);
    LED_OPEN(RED);
	
//	P1SEL &= ~BIT7;
//	P1DIR |= BIT7;
//    P1OUT |= BIT7;     
}

void board_reset(void)
{
    OSEL_INT_LOCK();
    WDTCTL = 0xFFFF;
}

void board_init(void)
{
//    pmap_init();
}


void srand_arch(void)
{
    srand(TA0R);
}

void lpm_arch(void)
{
    //LPM3;
}
