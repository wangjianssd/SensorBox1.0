/**
 * @brief       : 
 *
 * @file        : hal_energy.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "common/hal/hal_energy.h"
#include "platform/platform.h"

void hal_energy_init(void)
{
    energy_init();
}

uint8_t hal_energy_get(void)
{
    return energy_get();
}

uint16_t hal_voltage_get(void)
{
    return voltage_get();
}