/***************************************************************************
* @brief        : this
* @file         : radio.h
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-10-19
* @change Logs  :
* Date        Version      Author      Notes
* 2015-10-19      v0.1      gang.cheng    first version
***************************************************************************/
#ifndef __RADIO_DEFS_H__
#define __RADIO_DEFS_H__


#define INT_ENABLE                      (1u)
#define INT_DISABLE                     (0u)

typedef enum {
    DATA_RATE_10KBPS,
    DATA_RATE_100KBPS,
    DATA_RATE_250KBPS,
} data_rate_bps_t;


/**
 * RF中断类型宏定义 
 */
typedef enum
{
    TX_SFD_INT,
    RX_SFD_INT,
    RX_OK_INT,
    TX_OK_INT,
    TX_UND_INT,
    RX_OVR_INT,
    RF_INT_MAX_NUM,
} rf_int_t;

/**
 * rf的状态枚举变量,用于描述驱动层状态,非芯片状态值
 */
typedef enum 
{
    RF_INVALID_STATE,
    RF_IDLE_STATE,
    RF_RX_STATE,
    RF_TX_STATE,
    RF_SLEEP_STATE,
    RF_TXOK_STATE,
    RF_RXOK_STATE,
} rf_state_t;

typedef enum
{
    RF_STATE,
    RF_POWER,
    RF_CHANNEL,
    RF_DATARATE,

    RF_RXRSSI,
    RF_CCA,

    RF_LADDR0,
    RF_LADDR1,

    RF_DADDR0,
    RF_DADDR1,
    RF_DADDR2,
    RF_DADDR3,

    RF_RXFIFO_CNT,
    RF_RXFIFO_FLUSH,

    RF_TXFIFO_CNT,
    RF_TXFIFO_FLUSH,

    RF_RXFIFO_OVERFLOW,
    RF_TXFIFO_UNDERFLOW,
} rf_cmd_t;

typedef enum
{
    RF_RESULT_OK,
    RF_RESULT_NOT_SUPPORTED,
    RF_RESULT_INVALID_VALUE,
    RF_RESULT_ERROR,
} rf_result_t;

typedef void (*rf_int_reg_t)(uint16_t time);

/**
 * 无线设备驱动的结构体定义
 */
struct radio_driver
{
    bool_t (*init)(void);                               
    int8_t (*prepare)(const uint8_t *payload, uint8_t len);   //*< 把数据写入FIFO缓冲区，并未发送
    int8_t (*transmit)(uint8_t transmit_len);              //*< 发送之前已经准备好的数据

    int8_t (*send)(const uint8_t *payload, uint8_t len);      //*< prepare & transmit a packet
    int8_t (*recv)(uint8_t *const buf, uint8_t len);          //*< 从FIFO接收缓冲区里面读取指定长度的数据

    rf_result_t (*get_value)(rf_cmd_t cmd, void *value);
    rf_result_t (*set_value)(rf_cmd_t cmd, uint8_t value);

    bool_t (*int_cfg)(rf_int_t state, rf_int_reg_t int_cb, uint8_t type);
};


#endif

