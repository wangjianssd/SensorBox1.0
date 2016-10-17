/**
 * @brief       : .h 主要提供外部接口函数以及结构体、枚举定义 
 *
 * @file        : gprs_tx.h
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */

#ifndef __GPRS_TX_H
#define __GPRS_TX_H

#include "stdio.h"
#include "common/lib/lib.h"
#include "platform/platform.h"
#include "sys_arch/osel_arch.h"
#include "common/dev/sim928a/gprs_str.h"


#define GPRS_DATA_MAXLEN    128u

#define POWER_IOA_ON()          (P1OUT |= BIT3)
#define POWER_IOA_OFF()         (P1OUT &= ~BIT3)
#define POWER_IOB_ON()          (pow_iob_on())
#define POWER_IOB_OFF()         (pow_iob_off())

#define GPRS_TIMEOUT_TIME_PER   (1500ul)    //*< gprs超时的最小粒度

/**
 * @brief 定义GPRS的AT指令
 */
typedef enum 
{
    GPRS_CMD_NULL,
    GPRS_CMD_AT,
    GPRS_CMD_ATE0,
    GPRS_CMD_CGATT,
    GPRS_CMD_CIPSTART,
    GPRS_CMD_SEND_LEN,
    GPRS_CMD_STATUS,
    GPRS_CMD_CIPSHUT,
    GPRS_CMD_CIPCLOSE,
    GPRS_CMD_CPIN,
    GPRS_SEND_DATA,
}gprs_cmd_type_e;

/**
 * @brief 定义GPRS的各个状态枚举变量
 */
typedef enum
{
    INIT_SIG,
    TIMEOUT_SIG,
    SEND_AT,
    SEND_ATE0,
    SEND_CGATT_TIMEOUT,
    SEND_CGATT,
    SEND_CONNECT_TIMEOUT,
    SEND_CIPSTART,
    SEND_CIPSEND_LEN_TIMEOUT,    
    SEND_CIPSEND_LEN,
    SEND_DATA,
} sig_event_e;

/**
* @brief 定义GPRS上电或下电枚举变量
 */
typedef enum
{
    GPRS_POWER_ON,
    GPRS_POWER_OFF,
}power_operate_e;

/**
* @brief 发送模式定义
 */
typedef enum
{
    INVALID_MODE,
	TCP_SINGLE,
    TCP_CONTINE,
	UDP_SINGLE,
    UDP_CONTINE,    
}gprs_send_mode_e;

/**
* @brief gprs接收数据类型定义
 */
typedef struct{
    uint8_t *gprs_data;
    uint16_t sn;
    uint16_t len;
}gprs_receive_t;

/**
 *
 * @brief gprs发送反馈信息
 * @param param TRUE或者FALSE，表示发送成功或失败
 *        sn 帧的序列号
 * @return 无
 * 
 */
typedef void (*gprs_send_cb_t)(uint16_t param, uint16_t sn);
/**
 *
 * @brief gprs接收反馈信息
 * @param param 接收的数据
 * @return 无
 * 
 */
typedef void (*gprs_recv_cb_t)(gprs_receive_t param);

/**
 * @brief gprs ip和port配置
 */
typedef struct
{
    uint32_t  ip_addr;
    uint16_t   port;
    gprs_send_cb_t gprs_send_cb;
    gprs_recv_cb_t gprs_recv_cb;    
}gprs_init_cfg_t;

/**
 * @brief 接收app发来的数据信息
 */
typedef struct
{
    uint8_t     gprs_data_len;
    gprs_send_mode_e gprs_data_mode;
    uint16_t    gprs_sn;    
    uint8_t     gprs_data[GPRS_DATA_MAXLEN];
}gprs_data_type_t;

/**
 * @brief 状态
 */
typedef struct _fsm_s_
{
    void (*current_state) (struct _fsm_s_ *me, sig_event_e sig);
} fsm_t;

/**
 *@brief 定义指向状态切换的函数指针
 */
typedef void (*state_t) (fsm_t *me, sig_event_e sig);

#define TRAN(state)                                                            \
            hal_int_state_t s;                                                \
            HAL_ENTER_CRITICAL(s);                                             \
            (fsm_gprs.current_state = (state_t)(state));                       \
            HAL_EXIT_CRITICAL(s)                                              

#define FSM_DISPATCH(me, sig)           (fsm_gprs.current_state((me), (sig)))
                
/**
 * gprs的操作结果函数回调指针类型
 */
//typedef void (*gprs_power_operate_result_t)(int8_t res);

 
void pow_iob_on(void);
 
void pow_iob_off(void);

void gprs_flush_recv_buf_from_app(void);          

/**
 *
 * @brief IO口初始化（gsm_power、powerkey、reset）
 *        uart初始化、发送缓存和接收缓存的初始化、任务建立与消息绑定
 * @param gprs_cfg ip和port的配置
 * @return 无
 * 
 */
void gprs_tran_init(const gprs_init_cfg_t *gprs_cfg);

/**
 *
 * @brief gprs数据的发送处理函数
 * @param *data_p要发送的数据地址
 *        len     要发送的数据长度
 *        gprs_send_cb  发送回调
 *        connect_mode  连接模式，包括TCP和UDP模式
 *        sn 帧的序列号
 * @return 无
 * 
 */
void gprs_tran_send(void *data_p, 
                      uint8_t len,  
                      gprs_send_mode_e connect_mode,
                      uint16_t sn);

/**
 *
 * @brief 判断传输模块当前是否可以发送
 * @param 无
 * @return 可以发送则返回TRUE， 反之为FALSE
 * 
 */
bool_t gprs_tran_can_send(void);
void gprs_power_off_state(fsm_t *me, sig_event_e sig);
void gprs_power_on_state(fsm_t *me, sig_event_e sig);
void gprs_power_operate_proc(power_operate_e flag);
void gprs_prepare_connect_state(fsm_t *me, sig_event_e sig);
void gprs_connect_state(fsm_t *me,sig_event_e sig);
void gprs_send_state(fsm_t *me,sig_event_e sig);
void stop_gprs_no_data_timeout(void);
void gprs_pow_close(void);
void gprs_pow_open(void);


#endif
