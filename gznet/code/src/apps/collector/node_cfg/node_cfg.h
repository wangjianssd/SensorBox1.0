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


/*以下是协议相关配置*/
/******************************************************************************/
#define NODE_TYPE_TAG                   0x00
#define NODE_TYPE_ROUTER                0x01
#define NODE_TYPE_GATEWAY             	0x02

#define NODE_TYPE                       (NODE_TYPE_GATEWAY)
#define NODE_ID                         (0x00FEull)

/******************************************************************************/
#define PBUF_TYPE_MAX_NUM               (3u)  	// PBUF缓冲类型上限
#define PBUF_NUM_MAX                    (30u) 	// 表示各类pbuf个数的上限

#define SMALL_PBUF_BUFFER_SIZE          (34u)
#define MEDIUM_PBUF_BUFFER_SIZE         (64u)
#define LARGE_PBUF_BUFFER_SIZE          (64u)

#define SMALL_PBUF_NUM                  (0u)    // 各种PBUF最大个数
#define MEDIUM_PBUF_NUM                 (0u)
#define LARGE_PBUF_NUM                  (30u)
/******************************************************************************/
#define MAX_SBUF_NUM   		            (30u)   // SBUF最大个数
/******************************************************************************/
#define PKT_LEN_MAX 		            (MEDIUM_PBUF_BUFFER_SIZE)	// 协议中最大帧长，包括物理头
#define PKT_LEN_MIN                     (6u)    // 协议中最小帧长
#define UART_LEN_MAX         	        (128u)   // RX 串口中最大帧长

#define TDMA_SEND_MODE                  (0u)    // 数据不需要退避发送
#define CSMA_SEND_MODE                  (1u)    // 数据退避发送

#define MAX_SEND_TIMES                  (1u)    // MAC最大重传次数
#define MAX_JION_NET_CNT                (3u)    // 入网最大重试次数，超过则重新入网
#define NWK_MAX_SEND_TIMES              (3u)    // nwk resend max cnt
#define APP_SEND_TIMES			        (2u)	// APP最大重传次数
#define CONTINUATION_CNT	            (4u)	// 达到连续发送失败最大次数重入网
#define TRAN_RESEND_TIMES		        (0u)	// 传输模块重传间隔基准时间，ms
#define RF_INT_DEAL_FLOW                (1u)    // RF驱动中断调用上/下溢回调
#define RECV_RF_RATE_EN                 (0u)    // 接收数据是否进行成功率的比较
#define RECV_RF_SUCESS_RATE             (40)    // 接收数据的成功率
/******************************************************************************/
#define UART_NUM                        (2u)    // 串口数量
#define CH_SN                           (7u)   	// 无线信道编号
#define POWER_SN                        (5u)   	// 无线信道功率
#define MAX_TIMERS                      (10u)   // 定时器队列定时器数量
/******************************************************************************/
#define RF_AMPLIFIER_EN                 (0u)    // 功放是否打开
#define MAX_HOPS	                    (1u)  	// 设备的最大跳数
#define RSSI_RECV_FAILED	            (-100)	// 接收到的帧的RSSI值低于这个值就丢弃
#define CCA_RSSI_THRESHOLD              (-70)   // 低于该值任务信道为空
/******************************************************************************/
#define DEBUG_INFO_PRINT_EN             (0u)	// 是否打印调试信息
#define DEBUG_DEVICE_INFO_EN		    (1u)	// 是否配置设备基础调试信息
#define DEBUG_SSN_INFO_EN		        (0u)	// 是否配置SSN网络基础信息
#define DEBUG_ROUTE_JOIN_EN             (0u)    // 是否只让中继入网
#define VIRSION_NUMBER                  (0x30)  // 版本号
#define DATA_RATE                       (100u)  // 无线帧速率
/******************************************************************************/
static volatile int OS_LINE = 0;
static volatile int HAL_LINE = 0;
#endif


