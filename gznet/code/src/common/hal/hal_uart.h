/**
 * @brief       : Configuration and interface of hal_uart.c
 *
 * @file        : hal_uart.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */


#ifndef __HAL_UART_H
#define __HAL_UART_H

#include "platform/platform.h"


#define HAL_UART_1                  UART_1
#define HAL_UART_2                  UART_2
#define HAL_UART_3                  UART_3

#define HAL_UART_WITH_FRM_PARSE     UART_WITH_FRM_PARSE
#define HAL_UART_SD_LEN_MAX         UART_SD_LEN_MAX
#define HAL_UART_LD_LEN_MAX         UART_LD_LEN_MAX

/**
 * 串口初始化
 *
 * @param  hal_uart_id: 串口id
 * @param  baud_rate: 串口波特率
 *
 * @return: 无
 */
void hal_uart_init(uint8_t hal_uart_id, uint32_t baud_rate);

/**
 * 关闭串口电平转换芯片
 *
 * @param : 无
 *
 * @return: 无
 */
void hal_uart_level_switch_disable(void);

/**
 * 关闭串口接收中断，并将管脚配置为GPIO
 *
 * @param  hal_uart_id: 串口id
 *
 * @return: 无
 */
void hal_uart_close(uint8_t hal_uart_id);

/**
 * 打开串口接收中断
 *
 * @param : 无
 *
 * @return: 无
 */
void hal_uart_recv_enable(uint8_t uart_id);

/**
 * 关闭串口接收中断
 *
 * @param : 无
 *
 * @return: 无
 */
void hal_uart_recv_disable(uint8_t uart_id);

/**
 * 通过串口输出一个字符
 *
 * @param  hal_uart_id: 串口id
 * @param  value: 待输出的字符值
 *
 * @return: 无
 */
void hal_uart_send_char(uint8_t hal_uart_id, uint8_t value);

/**
 * 通过串口输出一个字符串
 *
 * @param  hal_uart_id: 串口id
 * @param  string: 待输出的字符串的首地址
 * @param len: 待输出的字符串长度
 *
 * @return: 无
 */
void hal_uart_send_string(uint8_t hal_uart_id,
                          uint8_t *const string,
                          uint16_t length);

/**
 * 将一个字符放入串口循环队列
 *
 * @param hal_uart_id: 串口id
 * @param c: 待放入的字符
 *
 * @return: 队列满则返回FALSE
 */
bool_t hal_uart_enter_q(uint8_t hal_uart_id, uint8_t c);

/**
 * 将一个字符串放入串口循环队列
 *
 * @param hal_uart_id: 串口id
 * @param string: 待放入的字符串的首地址
 * @param length: 待放入的字符串长度
 *
 * @return: 队列满则返回FALSE
 */
bool_t hal_uart_string_enter_q(uint8_t hal_uart_id,
                               uint8_t *const string,
                               uint16_t length);

/**
 * 删除串口id对应的循环队列
 *
 * @param hal_uart_id: 串口id
 * @param c_p: 指向队列的指针
 *
 * @return: 被删除的队列的指针
 */
bool_t hal_uart_del_q(uint8_t hal_uart_id, uint8_t *c_p);


void hal_uart_clear_rxbuf(uint8_t id);

void hal_uart_int_cb_reg(uart_interupt_cb_t cb);

#endif
