/**
 * @brief       : 
 *
 * @file        : bootloader.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "common/lib/data_type_def.h"

#define VECTOR_ELEMENT_LEN              (0x02)
#define INTADDR_LENGTH                  (0x80)
#define RESET_VECTOR_ADDR               (0x00FFFE)
#define SYSTEM_INTADDR                  (0x00FF80)
#define SYSTEM_INT_END_ADDR             (0x00FFFF)
#define SYSTEM_INT_SEG                  (0x00FE00)

#define SYSTEM_INFO_ADDR                (0x001800)

#define RESET_VECTOR_BOOT_ADDR          (0x00FDFE)
#define BOOTLOADER_INTADDR              (0x00FD80)
#define BOOTLOADER_INT_SEG              (0x00FC00)
#define BOOTLOADER_END_ADDR             (0x00FBFF)
#define BOOTLOADER_START_ADDR           (0x007C00)

#define RESET_VECTOR_APP_ADDR           (0x007BFE)
#define USER_INTADDR                    (0x007B80)
#define USER_INT_SEG                    (0x007A00)
#define USER_END_ADDR_1                 (0x0079FF)
#define USER_START_ADDR_1               (0x005C00)
#define USER_END_ADDR_2                 (0x01FFFF)
#define USER_START_ADDR_2                 (0x010000)

#define FLASH_SEGMENT_SIZE              (512u)



typedef struct
{
	bool_t user_code_ready;				// 用户代码已经准备好
	bool_t boot_code_done;				// boot代码直接启动user代码的标志
} app_user_info_t;

extern app_user_info_t app_user_info;

void bootloader_init(void);
void vector_copy(void *vector_dst, void *vcetor_src, uint8_t size);
#endif

