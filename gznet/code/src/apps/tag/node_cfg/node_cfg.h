/**
 * @brief       : 工程中各模块的参数配置
 *
 * @file        : node_cfg.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/8
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/8    v0.0.1      gang.cheng    some notes
 */

#ifndef __NODE_CONFIG_H
#define __NODE_CONFIG_H

#include "platform/platform_def.h"

/******************************************************************************/
#define NODE_TYPE_TAG                   0x00
#define NODE_TYPE_ROUTER                0x01
#define NODE_TYPE_GATEWAY				0x02

#define NODE_TYPE                       (NODE_TYPE_TAG)

#define NODE_PF                         PF_CORE_M3
/******************************************************************************/
#define NODE_ID                         (0x0007u)
/******************************************************************************/
#define PBUF_TYPE_MAX_NUM               (3u)        // PBUF缓冲类型上限
#define PBUF_NUM_MAX                    (20u)       // 表示各类pbuf个数的上限

#define SMALL_PBUF_BUFFER_SIZE          (20u)
#define MEDIUM_PBUF_BUFFER_SIZE         (62u)
#define LARGE_PBUF_BUFFER_SIZE          (64u)

#define SMALL_PBUF_NUM                  (0u)    // 各种PBUF最大个数
#define MEDIUM_PBUF_NUM                 (0u)
#define LARGE_PBUF_NUM                  (10u)
/******************************************************************************/
#define MAX_SBUF_NUM   			        (10u)   // SBUF最大个数
/******************************************************************************/
#define PKT_LEN_MAX 			        (64u)	// 协议中最大帧长
#define PKT_LEN_MIN         			(6u)    // 协议中最小帧长
#define UART_LEN_MAX         			(128u)   // 串口协议中最大帧长
#define TDMA_SEND_MODE                  (0u)    // 数据时隙发送
#define CSMA_SEND_MODE                  (1u)    // 数据退避发送

#define MAX_JION_NET_CNT                (3u)    // 入网最大重试次数，超过则重新入网
#define MAC_SEND_TIMES					(3u)	// 同一数据达到最大重传次数则丢弃
#define NWK_MAX_SEND_TIMES              (3u)    // nwk resend max cnt
#define APP_MAX_SEND_TIMES              (1u)    // app resend max cnt
#define CONTINUATION_CNT				(12u)	// 达到连续发送失败最大次数重入网
#define TRAN_RESEND_TIMES				(0u)	// 传输模块重传间隔基准时间，ms
#define RF_INT_DEAL_FLOW                (1u)    // RF驱动中断处理上/下溢回调
/******************************************************************************/
#define UART_NUM                        (1u)    // 串口数量
#define CH_SN                           (7u)    // 无线信道编号
#define POWER_SN                        (5u)   	// 无线功率编号
#define MAX_TIMERS                      (20u)   // 定时器队列定时器数量
/******************************************************************************/
#define SENSOR_DETECT_DISTANCE			(1u)    // 1:0-30cm 2:31-50cm 3:51-100cm
#define SENSOR_DETECT_INTERVAL_1		(1000u)	// 传感器入网前检测周期,单位毫秒
#define SENSOR_DETECT_INTERVAL_2        (20u)   // 传感器入网后检测周期,单位毫秒
#define SENSOR_DETECT_INTERVAL_3        (100u)  // 传感器入网后低功耗检测周期
#define APP_DATA_TRAN_PERIOD			(60u)    // 定期发送心跳数据的周期,单位秒
#define SENSOR_DATA_EXCPTION_THRE    	(30)	// 传感器数据毛刺判断阈值，须为有符号数
#define CAR_DETECT_OVR                  (45u)
#define CAR_DETECT_UND                  (15u)
/******************************************************************************/
#define RF_AMPLIFIER_EN                 (1u)    // 功放是否打开
#define MAX_HOPS	                    (5u)  	// 设备的最大跳数
#define RSSI_RECV_FAILED                (-70)  // 接收到的帧的RSSI值低于这个值就丢弃
#define CCA_RSSI_THRESHOLD              (-70)   // 低于该阈值表示信道空闲，用于CCA

/*以下是调试相关的配置*/
/******************************************************************************/
#define DEBUG_INFO_PRINT_EN             (1u)	// 是否打印调试信息
#define APP_DEBUG_INFO                  (0u)    // 调试信息
#define SENSOR_DATA_PERIOD              (20ul)  // x*100ms发送一次数据
#define DEBUG_DEVICE_INFO_EN			(1u)	// 是否配置设备基础调试信息
#define DEBUG_COORD_HOP                 (0u)    // 强制接收上级的跳数,小于这个数值不统计
#define DEBUG_SEND_QUERY_ONLY           (0u)    // 只发送查询帧
/******************************** 算法*****************************************/
/*	0x00: 真实流量 0x01:真实测速 0x02:电子警察  0x03: 模拟流量 0x04: 模拟测速 */
#define ALGORITHM_MODE_TYPE				(0x00u)

#define BOOT_TRIGGER_SENSOR_VAL_MAX		(1600u)		// 触发进入boot的阈值
#define BOOT_TRIGGER_SENSOR_CNT_MAX_1   (3u)       // 连续触发的次数，每次1s
#define BOOT_TRIGGER_SENSOR_CNT_MAX_2	(150u)		// 连续触发的次数，每次20ms
#define VIRSION_NUMBER                  (0x05)      // 版本号
#define DATA_RATE                       (100u)      // 无线速率
// ssn mac config

/******************************************************************************/
static volatile int OS_LINE = 0;
static volatile int HAL_LINE = 0;
#endif



