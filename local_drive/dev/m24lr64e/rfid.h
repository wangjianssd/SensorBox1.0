 /**
 * @brief       : RFID 驱动代码
 *
 * @file        : hal_rfid.h
 * @author      : wangjifang
 * @version     : v0.0.1
 * @date        : 2014/02/25
 *
 * Change Logs  : 修改注释及代码格式
 *
 * Date        Version      Author      Notes
 * 2014/02/25  v0.0.1    wangjifang    first version
 * 2015/09/15  v0.0.2    cuihongpeng   Second version
 */
#ifndef __RFID_H_
#define __RFID_H_

#pragma once
#include <gznet.h>
//#include <debug.h>

#define ERROR               FALSE      /**< 错误 */
#define RIGHT               TRUE       /**< 正确 */
#define incept_addread      (0x0004)   /**< 读操作内部地址首址 */
#define incept_addwrite     (0x0004)   /**< 写操作内部地址首址 */
#define Quantity            (4U)       /**< 操作数量 */
#define write		        (0x00)     /**< 写 */
#define read		        (0x01)     /**< 读 */

#define RFID_ADDRESS_E2_0    (0x53)     /**< NFC地址 ,正常读写用户区的地址 */
#define RFID_ADDRESS_E2_1    (0x57)     /**< NFC地址,配置系统区地址 */


/*************************************************************/
/*****************接口函数************************************/
/*************************************************************/

/**
 * i2c初始化
 *
 * @param i2c_slave_dev 从设备的I2C地址
 *
 * @return void
 */
void m24lr64e_i2c_init();

/**
 * i2c向NFC写一个byte
 *
 * @param word_addr  在NFC的哪个地址写字节
 * @param word_value 要写的字节
 *
 * @return void
 */
bool_t i2c_send_byte(uint16_t const word_addr,
                      uint8_t const word_value);

/**
 * i2c向NFC读一个byte
 *
 * @param pword_buf 读出来的字节存放的地址
 * @param word_addr 在NFC的哪个地址读字节
 *
 * @return void
 */
bool_t i2c_recv_byte(uint16_t const word_addr ,
                      uint8_t *const pword_buf);

/**
 * i2c向NFC连续读数据
 *
 * @param data_buf  读出来的数据存放的地址
 * @param data_len  读出来的数据长度
 * @param word_addr 从NFC的哪个地址读数据
 *
 * @return void
 */
bool_t read_rfid_data(uint8_t *const data_buf, 
                      uint8_t const data_len, 
                      uint16_t const word_addr);

/**
 * i2c向NFC连续写同个一个扇区的数据
 *
 * @param data_buf  读出来的数据存放的地址
 * @param data_len  读出来的数据长度
 * @param word_addr 从NFC的哪个地址读数据
 *
 * @return void
 */
bool_t write_rfid_area_data(uint8_t *const data_buf, 
                            uint8_t const data_len, 
                            uint16_t const word_addr);

/**
 * i2c向NFC连续写数据，可同可不同个扇区
 *
 * @param data_buf  读出来的数据存放的地址
 * @param data_len  读出来的数据长度
 * @param word_addr 从NFC的哪个地址读数据
 *
 * @return void
 */
bool_t write_rfid_data(uint8_t *const data_buf,
                       uint8_t const data_len,
                       uint16_t const word_addr);
/**
 * RFID 初始化
 *
 * @return void
 */ 
 void m24lr64e_init(void);
 /**
 * RFID 中断处理函数
 *
 * @return void
 */ 
 
 #endif