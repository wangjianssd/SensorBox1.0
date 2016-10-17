 /**
 * @brief       : GPS nmea解析与处理
 *
 * @file        : gps.h
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Change Logs  : 修改注释及代码格式
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 * 2015/09/29  v0.0.2       xukai       修改部分内容
 */
#ifndef __GPS_H
#define __GPS_H

/**
 * @brief GPS简单信息，仅包括经度、纬度和速度
 */ 
typedef struct
{
    double longitude;               /**< 经度结果 */
    double latitude;                /**< 纬度结果 */
    double speed;                   /**< 速度结果 */
} gps_simple_info_t;

typedef struct _location_t
{
	uint16_t deg;
	uint8_t min_dec;
	uint16_t min_fract;
} location_t;

typedef void (*gps_cb_t)(gps_simple_info_t gps_data);

#define GPS_UART            	UART_2

#define GPS_DEBUG           	0
#define GPS_PROTECT_DEBUG     	0
#define GPS_DATA_DEBUG     		0
#define GPS_DEBUG_INFO			0

#define GPS_RECV_LEN_MAX    	128         /**< GPRMC nmea语句接收最大长度 */
#define GPS_CMD_LEN_MAX     	100
#define GPS_CR              	0x0D
#define GPS_LF              	0x0A
#define GPS_CRC_PROMPT      	'*'
#define GPS_COMMA      			','
#define GPS_RMC_BLANK_NUM    	12
#define GPS_RMC_LEN_MIN    		7           /**< GPRMC nmea语句最小长度 */
#define GPS_LAT_LEN				9
#define GPS_LON_LEN				10
#define GPS_DATA_TYPE_LAT		1
#define GPS_DATA_TYPE_LON		2

#define GPS_TASK_PRIO       (3u)

typedef enum
{
    GPS_UART_EVENT = ((GPS_TASK_PRIO<<8) | 0x01),
    GPS_RECEIVE_EVENT,
    GPS_OPEN_EVENT,
	GPS_WAIT_FOR_OPEN_EVENT,
    GPS_PROTECT_EVENT,
} gps_task_sig_enum_t;

PROCESS_NAME(gps_task_thread_process);

void gps_init(gps_cb_t gps_cb);

void gps_open(void);

void gps_close(void);
void gps_sleep(void);

#endif
