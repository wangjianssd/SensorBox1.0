/**
 * @brief       : 
 *
 * @file        : energy.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#ifndef __ENERGY_H__
#define __ENERGY_H__

#include "common/lib/data_type_def.h"

#define FULL_POWER_VAR              (3100)
#define LOW_POWER_VAR               (2500)
#define POWER_DIV                   ((FULL_POWER_VAR-LOW_POWER_VAR)/100)

#define ADC12_INIT()                                                \
    do                                                              \
    {                                                               \
        ADC12CTL0 |= ADC12SHT00 + ADC12SHT01 + ADC12SHT02 +         \
              ADC12SHT03 + ADC12REF2_5V + ADC12REFON_L + ADC12ON_L; \
	    ADC12CTL1 |= ADC12SHP + ADC12DIV0 + ADC12SSEL0;             \
	    ADC12CTL2 |= ADC12PDIV + ADC12TCOFF + ADC12RES1 + ADC12SR;  \
	    ADC12MCTL0 |= ADC12SREF0 + ADC12INCH_4;                     \
        P6SEL |= BIT4;                                              \
        P6DIR &= ~BIT4;                                             \
    } while(__LINE__ == -1)

#define ADC12_WAIT_IDLE()	        while(ADC12CTL1 & ADC12BUSY)
#define ADC12_WAIT_CONVERSION_END()	while(!(ADC12IFG & BIT0))
#define ADC12_IS_CONVERSION_END()   (ADC12IFG & BIT0)
#define ADC12_ON()			        st( ADC12CTL0 |= ADC12ON_L; ADC12CTL0 |= ADC12ENC_L + ADC12SC_L; )
#define ADC12_OFF()			        st( ADC12CTL0 &= ~ADC12ENC_L; ADC12CTL0 &= ~ADC12ON_L; ADC12CTL0 = 0; )
#define ADC12_CONVERSION_RESULT()	(((uint32_t)ADC12MEM0 * 2500) / 4095)

#define ADC12_FREE()         \
    do                       \
    {                        \
        ADC12_WAIT_IDLE();   \
	    ADC12_OFF();         \
    } while(__LINE__ == -1)

void energy_init(void);

uint8_t energy_get(void);

#endif
