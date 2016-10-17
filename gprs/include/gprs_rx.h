/**
 * @brief       : 接口定义
 *
 * @file        : gprs_rx.h
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */

#ifndef __GPRS_TX_PARSE_H
#define __GPRS_TX_PARSE_H

#define GPRS_RECV_CMD_SIZE_MAX        128u

void gprs_send_ok_handler(void);

void gprs_response_event_parse(void);
void gprs_cmd_cip_recv_handle(void *arg);
void gprs_uart_event_handler(void);

#endif
