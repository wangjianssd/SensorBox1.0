 /**
 * @brief       : 心跳的头文件
 *
 * @file        : app_heartbeat.h
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#ifndef _APP_HEARTBEAT_H_
#define _APP_HEARTBEAT_H_

#include <gznet.h>
//#include <data_type_def.h>

void box_heartbeat_init(void);
void box_power_heart_timer_start(uint32_t heart_period,uint32_t low_power_period);
void box_heartbeat_timer_stop(void);
void box_low_power_1_handle(void *arg);
void box_low_power_2_handle(void *arg);
void box_heartbeat_timer_handle(void *arg);
#endif