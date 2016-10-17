#include "common/lib/lib.h"
#include "driver.h"

void flash_write(uint8_t *flash_ptr,
                 const uint8_t *buffer,
                 uint16_t len)
{

}

void flash_read(const uint8_t *flash_ptr,
                uint8_t *buffer,
                uint16_t len)
{

}

/**
 * 擦除flash段或区的数据(segment or bank)
 *
 * @param *flash_ptr: 将被擦除的flash区的首地址
 * @param erase_type: 选择擦除区/段
 *
 * @return: 空
 */
void flash_erase(uint8_t *const flash_ptr,
                 uint8_t erase_type)
{
	
}