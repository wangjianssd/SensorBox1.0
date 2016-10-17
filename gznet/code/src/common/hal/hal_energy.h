/**
 * @file hal_energy.h
 * @author gang.cheng
 */
#ifndef __HAL_ENERGY_H
#define __HAL_ENERGY_H
     
#include "common/lib/data_type_def.h"

#define HAL_CHIP_ENTER_CPU_IDLE         0u
#define HAL_CHIP_POWER_MODE_1           1u
#define HAL_CHIP_POWER_MODE_2           2u
#define HAL_CHIP_POWER_MODE_3           3u

void hal_energy_init(void);

uint8_t hal_energy_get(void);
uint16_t hal_voltage_get(void);

#endif

