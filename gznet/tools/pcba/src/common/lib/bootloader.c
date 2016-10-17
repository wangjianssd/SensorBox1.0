/**
 * @brief       : 
 *
 * @file        : bootloader.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <lib.h>
#include <hal.h>

app_user_info_t app_user_info;		// 用户区代码标示

void vector_copy(void *vector_dst, void *vcetor_src, uint8_t size)
{
	hal_nvmem_write((uint8_t *)vector_dst, (uint8_t *)vcetor_src, size);
}

void bootloader_init(void)
{    
    uint8_t reset_vector[VECTOR_ELEMENT_LEN];
	hal_nvmem_read((uint8_t *)SYSTEM_INFO_ADDR, (uint8_t *)&app_user_info, sizeof(app_user_info));
    app_user_info.boot_code_done = 0xFF;   // 让boot启动的时候不直接跳转到user区
    hal_nvmem_erase((uint8_t *)SYSTEM_INFO_ADDR, FLASH_SEG_ERASE);
    hal_nvmem_write((uint8_t *)SYSTEM_INFO_ADDR, (uint8_t *)&app_user_info, sizeof(app_user_info));

    hal_nvmem_read((uint8_t *)RESET_VECTOR_ADDR, reset_vector, VECTOR_ELEMENT_LEN);
    hal_nvmem_erase((uint8_t *)SYSTEM_INTADDR, FLASH_SEG_ERASE);
    vector_copy((uint8_t *)SYSTEM_INTADDR, (uint8_t *)USER_INTADDR, INTADDR_LENGTH-VECTOR_ELEMENT_LEN);
    vector_copy((uint8_t *)RESET_VECTOR_ADDR, reset_vector, VECTOR_ELEMENT_LEN);
}

