/**
 * @brief       : 
 *
 * @file        : energy.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#include <energy.h>
#include <msp430.h>

void energy_init(void)
{
    
}

uint16_t energy_get(void)
{    
    uint16_t voltage_mv;

    ADC12_INIT();
	ADC12_WAIT_IDLE();
	ADC12_ON();
	ADC12_WAIT_CONVERSION_END();
	voltage_mv = ADC12_CONVERSION_RESULT()*3;
	ADC12_FREE();
    
    return voltage_mv;
}

