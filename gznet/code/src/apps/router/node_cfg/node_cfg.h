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
#define NODE_TYPE_GATEWAY               0x02

#define NODE_TYPE                       (NODE_TYPE_ROUTER)
#define NODE_ID                         (0x00a1ull)
#define NODE_LINCESE                    "P-ITCS13049ITCSM107A0E"

/******************************************************************************/
#define PBUF_TYPE_MAX_NUM               (3u)    // PBUF缓冲类型上限
#define PBUF_NUM_MAX                    (20u)   // 表示各类pbuf个数的上限

#define SMALL_PBUF_BUFFER_SIZE          (20u)
#define MEDIUM_PBUF_BUFFER_SIZE         (62u)
#define LARGE_PBUF_BUFFER_SIZE          (64u)

#define SMALL_PBUF_NUM                  (0u)    // 各种PBUF最大个数
#define MEDIUM_PBUF_NUM                 (0u)
#define LARGE_PBUF_NUM                  (20u)
/******************************************************************************/
#define MAX_SBUF_NUM                    (20u)   // SBUF最大个数
/******************************************************************************/
#define PKT_LEN_MAX                     (60u)   // 协议中最大帧长，包括物理头
#define PKT_LEN_MIN                     (6u)    // 协议中最小帧长
#define UART_LEN_MAX                    (128u)   // RX 串口中最大帧长

#define TDMA_SEND_MODE                  (0u)    // 数据时隙发送
#define CSMA_SEND_MODE                  (1u)    // 数据退避发送

#define DATA_RATE                       (100u)  // 无线帧速率
#define MAX_INTRA_SEND_TIMES            (5u)    // MAC最大簇内重传次数
#define MAX_INTER_SEND_TIMES            (2u)    // MAC最大簇间重传次数
#define NWK_MAX_SEND_TIMES              (1u)    // nwk resend max cnt
#define APP_MAX_SEND_TIMES              (1u)    // APP最大重传次数
#define MAX_JION_NET_CNT                (3u)    // 入网最大重试次数，超过则重新入网
#define CONTINUATION_CNT                (4u)    // 达到连续发送失败最大次数重入网
#define TRAN_RESEND_TIMES               (0u)    // 传输模块重传间隔基准时间，ms
#define RF_INT_DEAL_FLOW                (1u)    // RF驱动中断调用上/下溢回调
#define RECV_RF_RATE_EN                 (0u)    // 接收数据是否进行成功率的比较
#define RECV_RF_SUCESS_RATE             (70)    // 接收数据的成功率
/******************************************************************************/
#define SLOT_RANDOM_MAX                 (155)   // 时隙随机选择的最大数
/******************************************************************************/
#define UART_NUM                        (1u)    // 串口数量
#define CH_SN                           (6u)    // 无线信道编号
#define POWER_SN                        (5u)    // 无线信道功率
#define MAX_TIMERS                      (15u)    // 定时器队列定时器数量
/******************************************************************************/
#define MAX_TOPU_SIZE                   (48u)   // 拓扑数组的最大维度
#define MAX_TOPU_SURVIVAL_TIME          (7u)    // 最大存活时间
/******************************************************************************/
#define MAX_HOPS                        (5u)    // 设备的最大跳数
#define RSSI_RECV_FAILED                (-100)  // 接收到的帧的RSSI值低于这个值就丢弃
#define RSSI_QUERY_THRESHOLD            (-65)   // 问询应答可靠通信的100%阈值
#define CCA_RSSI_THRESHOLD              (-70)   // 低于该值任务信道为空
/******************************************************************************/
#define DEBUG_INFO_PRINT_EN             (0u)    // 是否打印调试信息
#define APP_DEBUG_INFO                  (0u)    // 调试信息
#define DEBUG_DEVICE_INFO_EN            (1u)    // 是否配置设备基础调试信息
#define DEBUG_COORD_HOP                 (0u)    // 强制接收上级的跳数,小于这个数值不统计
#define DEBUG_COORD_ID                  (0u)    // 是否强制上级ID
#define DEBUG_COORD_FATHER_ID           (0x00a1)// 强制上级ID  
/******************************************************************************/
#define PRINT_SYNC_PARA_EN              (0)     // 是否打印同步参数
#define APP_DATA_TRAN_PERIOD			(60u)   // 定期发送心跳数据的周期,单位秒

#define VIRSION_NUMBER                  (0x30)  //版本号

#endif



