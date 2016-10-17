/**
 * @brief       : 
 *
 * @file        : $FILE_FNAME$
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/10/23
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/23    v0.0.1      WangJifang    some notes
 */
 
#ifndef __I2C__
#define __I2C__

#define I2C_DRIVER          (i2c_device_driver)

typedef void (*nfc_interupt_cb_t)(void);
/**
 * @brief I2C设备驱动的结构体定义
 */
typedef struct _i2c_driver_
{
    bool_t (*init)(void);                                               /**< I2C初始化接口 */
    
    bool_t (*deinit)(void);                                             /**< I2C注销接口 */

    bool_t (*send)(uint8_t mode, uint8_t *const data_buf, uint8_t const data_len, uint16_t const word_addr);          /**< I2C发送接口 */
    
    bool_t (*recv)(uint8_t mode, uint8_t *const data_buf, uint8_t const data_len, uint16_t const word_addr);          /**< I2C读接口 */

    void (*int_cb_reg)(nfc_interupt_cb_t cb);                           /**< NFC接收RF数据中断回调函数注册 */
} i2c_driver_t;

const extern i2c_driver_t i2c_device_driver;
#endif
