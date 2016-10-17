/**
 * @brief       : 
 *
 * @file        : nfc_defs.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/10/23
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/23    v0.0.1      gang.cheng    first version
 */
#ifndef __NFC_DEFS_H__
#define __NFC_DEFS_H__


#define NFC_DEVICE                     (rfid_device_driver)

typedef void (*rfid_interupt_cb_t)(void);
                     
typedef enum
{
    NFC_ID,
} nfc_cmd_t;


/**
 * RFID设备驱动的结构体定义
 */
typedef struct _rfid_driver_
{
    bool_t (*init)(void);                                             /**< RFID初始化接口 */
    
    bool_t (*deinit)(void);                                           /**< RFID注销接口 */

    uint8_t (*send)(uint8_t mode,uint8_t *const data_buf, uint8_t const data_len, uint16_t const word_addr);        /**< RFID发送接口 */
    
    uint8_t (*recv)(uint8_t mode,uint8_t *const data_buf, uint8_t const data_len, uint16_t const word_addr);        /**< RFID读接口 */
    
    bool_t (*get_value)(uint8_t type, void *value);                     /**< RFID控制接口 */

    void (*int_cb_reg)(rfid_interupt_cb_t cb);                                  /**< RFID中断回调注册*/
} rfid_driver_t;


#endif

