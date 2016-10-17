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

#include <driver.h>
#include <debug.h>

void led_init(void)
{
#if (NODE_TYPE == NODE_TYPE_DETECTOR)
	P1SEL &= ~BIT7;
	P1DIR |= BIT7;
    P1OUT |= BIT7;             // BLUE

#elif (NODE_TYPE == NODE_TYPE_ROUTER) || (NODE_TYPE == NODE_TYPE_COLLECTOR)
	P1SEL &= ~(BIT5 + BIT6 + BIT7);
	P1DIR |= BIT5 + BIT6 + BIT7;
	P1OUT |= BIT5 + BIT6 + BIT7;

#elif (NODE_TYPE == NODE_TYPE_HOST)
    P1SEL &= ~(BIT4 + BIT6);
	P1DIR |= BIT4 + BIT6;
	P1OUT |= BIT4 + BIT6;
    P1OUT &= ~BIT6;
#elif (NODE_TYPE == NODE_TYPE_SMALL_HOST)
    P1SEL &= ~(BIT2 + BIT6 + BIT7); //!< led_com-P1.2, led_error-p1.6, led_dbg-p1.7
    P1DIR |= (BIT2 + BIT6 + BIT7);
    P1OUT |= (BIT2 + BIT6 + BIT7);
#endif

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

