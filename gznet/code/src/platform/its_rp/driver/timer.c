/**
 * @brief       : 
 *
 * @file        : timer.c
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
#include "sys_arch/wsnos/wsnos.h"

extern void rf_int_handler(uint16_t time);
void htimer_int_handler(void);
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer0_a1_isr(void)
{
	OSEL_ISR_ENTRY();

	switch (__even_in_range(TA0IV, 14))
	{
		case TA0IV_TA0CCR1:
			rf_int_handler(TA0CCR1);
			break;
		case TA0IV_TA0CCR2:
			break;
		case TA0IV_TA0CCR3:
			break;
		case TA0IV_TA0CCR4:
			htimer_int_handler();
			break;
		case TA0IV_TA0IFG:
			break;
		default:
			break;
	}

	OSEL_ISR_EXIT();
	LPM3_EXIT;
}


