/**
 * @brief       : this
 * @file        : flash.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-15
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-15  v0.0.1  gang.cheng    first version
 */
#ifndef __FLASH_H__
#define __FLASH_H__
#include "sys_arch/osel_arch.h"

#define INFO_A_ADDR     0x1980U     //不能用
#define INFO_B_ADDR     0x1900U     //设备信息
#define INFO_C_ADDR     0x1880U     //DEBUG信息、背景值信息
#define INFO_D_ADDR     0x1800U     //标志位

#define FLASH_SEG_ERASE 0
#define FLASH_BK_ERASE  1


/**
 * 向flash中写数据，应先禁止全局中断
 *
 * @param *buffer: 存放待写数据的源地址指针(缓冲区)
 * @param *flash_ptr: 将被写入的目的地址指针(flash 区)
 * @param len: 将被写入的数据的长度,最长为128字节
 *
 * @return: 空
 */
void flash_write(uint8_t *flash_ptr,
                 const uint8_t *buffer,
                 uint16_t len);

/**
 * 从flash区读取数据
 *
 * @param *flash_ptr: 将被读取的源地址指针(flash 区)
 * @param *buffer: 存放读出数据的目的地址指针(缓冲区)
 * @param len: 将被读取的数据的长度,最长为128字节
 *
 * @return: 空
 */
void flash_read(const uint8_t *flash_ptr,
                uint8_t *buffer,
                uint16_t len);

/**
 * 擦除flash段或区的数据(segment or bank)
 *
 * @param *flash_ptr: 将被擦除的flash区的首地址
 * @param erase_type: 选择擦除区/段
 *
 * @return: 空
 */
void flash_erase(uint8_t *const flash_ptr,
                 uint8_t erase_type);

#endif
