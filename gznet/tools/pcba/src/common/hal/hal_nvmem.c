/**
 * @brief       : 
 *
 * @file        : hal_nvmem.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <hal.h>

void hal_nvmem_write(uint8_t *flash_ptr,
                     const uint8_t *buffer,
                     uint16_t len)
{
	flash_write(flash_ptr, buffer, len);
}

void hal_nvmem_read(const uint8_t *flash_ptr,
                    uint8_t *buffer,
                    uint16_t len)
{
	flash_read(flash_ptr, buffer, len);
}

void hal_nvmem_erase(uint8_t *const flash_ptr,
                     uint8_t erase_type)
{
	flash_erase(flash_ptr, erase_type);
}
