/**
 * @brief       : 
 *
 * @file        : m24lr64e.h
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/10/23
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/23    v0.0.1      WangJifang    some notes
 */

#ifndef __M24LR64E__
#define __M24LR64E__

#include "common/dev/nfc_defs.h"

#define RFID_DEVICE                     (rfid_device_driver)

#define ERROR                       FALSE				//错误
#define RIGHT                       TRUE				//正确

#define write		                0x00 	            //写
#define read		                0x01 	            //读

#define NFC_ADDRESS_E2_0            0x53                //NFC地址 ,正常读写用户区的地址
#define NFC_ADDRESS_E2_1            0x57                //NFC地址,配置系统区地址

#define NFC_INT_BUSY_ADDR           0x0910               //NFC中断配置

#define NFC_ID_ADDR                 0x0914

const extern rfid_driver_t rfid_device_driver;

#endif