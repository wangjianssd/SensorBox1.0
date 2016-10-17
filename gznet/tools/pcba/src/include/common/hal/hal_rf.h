/**
 * @brief       : configuration and interface of hal_rf.c
 *
 * @file        : hal_rf.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __HAL_RF_H
#define __HAL_RF_H

#include <data_type_def.h>

#if (((__TID__ >> 8) & 0x7F) != 0x2b)     /* 0x2b = 43 dec */
#define MAX_CHANNEL_NUM                 5u

#define HAL_CHANNEL_1                   CHANNEL_1
#define HAL_CHANNEL_2                   CHANNEL_2
#define HAL_CHANNEL_3                   CHANNEL_3
#define HAL_CHANNEL_4                   CHANNEL_4
#define HAL_CHANNEL_5                   CHANNEL_5
#define HAL_INVALID_CHANNEL             INVALID_CHANNEL
#endif

#define HAL_RF_INVALID_STATE	    	0x01
#define HAL_RF_IDLE_STATE				0x02
#define HAL_RF_TX_STATE					0x03
#define HAL_RF_RX_STATE					0x04
#define HAL_RF_SLEEP_STATE              0x05
#define HAL_RF_TXOK_STATE               0x06
#define HAL_RF_RXOK_STATE               0x07

#define HAL_RF_RXOK_INT				    0x00
#define HAL_RF_TXOK_INT				    0x01
#define HAL_RF_RXSFD_INT			    0x02
#define HAL_RF_TXSFD_INT			    0x03
#define HAL_RF_TXUND_INT				0x04
#define HAL_RF_RXOVR_INT				0x05

#define HAL_INT_ENABLE                  INT_ENABLE
#define HAL_INT_DISABLE                 INT_DISABLE

typedef void (*hal_rf_cb_t)(uint16_t int_time); // 参数供传递时戳使用

/**
 * RF初始化
 *
 * @param:  无
 * @return: 初始化成功与否
 */
bool_t hal_rf_init(void);

/**
 * RF重启
 *
 * @param:  无
 * @return: 无
 */
void hal_rf_reset(void);

/**
 * 配置RF中断
 *
 * @param  int_type: RF 中断类型
 * @param  flag: 中断是否使能
 * @return: 配置成功与否
 */
bool_t hal_rf_cfg_int(uint16_t int_type, bool_t flag);

/**
 * RF中断回调函数注册
 *
 * @param  int_type: RF 中断类型
 * @param  hal_rf_cb: 中断回调函数指针
 * @return: 注册成功与否
 */
bool_t hal_rf_reg_int(uint16_t int_type, hal_rf_cb_t hal_rf_cb);

/**
 * RF中断回调函数反注册
 *
 * @param  int_type: RF 中断类型
 * @return: 反注册成功与否
 */
bool_t hal_rf_unreg_int(uint8_t int_type);

/**
 * 读取RF缓冲区的内容
 *
 * @param  buffer: 存放读出数据的缓冲区的首地址
 * @param  buf_len: 需要读出数据的长度
 * @return: 读取数据成功与否
 */
bool_t hal_rf_read_fifo(uint8_t *buffer, uint8_t buf_len);

/**
 * 向RF缓冲区写入数据
 *
 * @param  buffer: 待写入的数据的缓冲区首地址
 * @param  buf_len: 待写入数据的长度
 * @return: 写入数据成功与否
 */
bool_t hal_rf_write_fifo(uint8_t *buffer, uint8_t length);

/**
 * 清空RF数据接收缓冲区
 *
 * @param: 无
 * @return: 无
 */
void hal_rf_flush_rxfifo(void);

/**
 * 清空RF数据发送缓冲区
 *
 * @param: 无
 * @return: 无
 */
void hal_rf_flush_txfifo(void);

uint8_t hal_rf_get_rxfifo_cnt(void);

bool_t hal_rf_rxfifo_overflow(void);

bool_t hal_rf_txfifo_underflow(void);

bool_t hal_rf_set_power(uint8_t power_index);

uint8_t hal_rf_get_power(void);

bool_t hal_rf_set_channel(uint8_t channel_index);

int16_t hal_rf_get_rssi(void);

/**
 * 获取在RX_SFD中读取的RSSI值，因为Si4432每次发送的数据较长时，
 * 比如48字节，在RX中断里读出来的RSSI就会失真
 *
 * @param: 无
 * @return: rssi的值
 */
int8_t hal_rf_get_rx_rssi(void);

uint8_t hal_rf_get_state(void);

bool_t hal_rf_set_state(uint8_t rf_state);

void hal_rf_write_reg(uint8_t addr, uint8_t value);

#endif

/**
 * @}
 */
