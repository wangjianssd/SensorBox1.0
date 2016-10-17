#ifndef __NODE_CONFIG_H
#define __NODE_CONFIG_H

/******************************************************************************/
#define NODE_TYPE_TAG                   0x00
#define NODE_TYPE_ROUTER                0x01
#define NODE_TYPE_GATEWAY               0x02
#define NODE_TYPE_SERIAL_SERVER			0x03
#define NODE_TYPE                       (NODE_TYPE_TAG)
/******************************************************************************/
#define NODE_ID                         (0x0000000000000018ull)
#define NODE_LINCESE                    "P-ITCS13049ITCSM107A0E"
#define NWK_ADDR						(0xfffe)
/******************************************************************************/
#define PBUF_TYPE_MAX_NUM               (3u)    // PBUF缓冲类型上限
#define PBUF_NUM_MAX                    (30u)   // 表示各类pbuf个数的上限

#define SMALL_PBUF_BUFFER_SIZE          (20u)
#define MEDIUM_PBUF_BUFFER_SIZE         (62u)
#define LARGE_PBUF_BUFFER_SIZE          (80u)

#define SMALL_PBUF_NUM                  (0u)    // 各种PBUF最大个数
#define MEDIUM_PBUF_NUM                 (10u)
#define LARGE_PBUF_NUM                  (10u)
/******************************************************************************/
#define MAX_SBUF_NUM                    (16u)   // SBUF最大个数
/******************************************************************************/
#define PKT_LEN_MAX                     (64u)   // 协议中最大帧长
#define PKT_LEN_MIN                     (6u)    // 协议中最小帧长
#define UART_LEN_MAX                    (128u)  // 串口协议中最大帧长
#define TDMA_SEND_MODE                  (0u)    // 数据时隙发送
#define CSMA_SEND_MODE                  (1u)    // 数据退避发送

#define DATA_RATE                       (100u)  // 无线速率
#define MAX_JION_NET_CNT                (3u)    // 入网最大重试次数，超过则重新入网
#define MAC_MAX_SEND_TIMES              (1u)    // 同一数据达到最大重传次数则丢弃
#define NWK_MAX_SEND_TIMES              (2u)    // nwk resend max cnt
#define APP_MAX_SEND_TIMES              (1u)    // app resend max cnt
#define CONTINUATION_CNT                (12u)   // 达到连续发送失败最大次数重入网
#define TRAN_RESEND_TIMES               (0u)    // 传输模块重传间隔基准时间，ms
#define RF_INT_DEAL_FLOW                (1u)    // RF驱动中断处理上/下溢回调
/******************************************************************************/
#define UART_NUM                        (2u)    // 串口数量
#define CH_SN                           (11u)    // 无线信道编号
#define POWER_SN                        (5u)    // 无线功率编号
#define MAX_TIMERS                      (15u)   // 定时器队列定时器数量
/******************************************************************************/
#define MAX_HOPS                        (5u)    // 设备的最大跳数
#define RSSI_QUERY_FAILED               (-100)  // 问询应答可靠通信的下限
#define RSSI_QUERY_THRESHOLD            (-80)   // 问询应答可靠通信的100%阈值
#define CCA_RSSI_THRESHOLD              (-70)   // 低于该阈值表示信道空闲，用于CCA
/******************************************************************************/
#define SIMA_SENSOR_DATA_EN             (0u)    // 是否使用模拟数据
#define DEBUG_INFO_PRINT_EN             (0u)    // 是否打印调试信息
#define DEBUG_DEVICE_INFO_EN            (1u)    // 是否配置设备基础调试信息
#define DEBUG_COORD_HOP                 (0u)    // 强制接收上级的跳数,小于这个数值不统计
#define SYSTEM_PLAN_NUM			        (1u)	// 时隙配置方案序号
/******************************************************************************/
#define DEV_COUNT                       (16u)   // neighbors num
#define PRINT_SYNC_PARA_EN              (0)     // 是否打印同步参数
#define PLATFORM						(0u)


#endif



