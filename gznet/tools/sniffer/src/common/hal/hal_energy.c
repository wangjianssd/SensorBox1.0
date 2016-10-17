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

#include <hal_energy.h>
#include <driver.h>

void hal_energy_init(void)
{
    energy_init();
}

uint8_t hal_energy_get(void)
{
    return energy_get();
}