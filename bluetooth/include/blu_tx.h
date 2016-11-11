/**
 * @brief       : .h 主要提供外部接口函数以及结构体、枚举定义 
 *
 * @file        : gprs_tx.h
 * @author      : xhy
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1     xhy    文件初始版本
 */

#ifndef __BLU_H
#define __BLU_H

#include <gznet.h> 
#include <stdio.h>
#include <gprs_str.h>

#define blu_Init()     {P3SEL &= ~(BIT1);\
							P3DIR &= ~(BIT1);\
						 	P3IES |= BIT1;\
						 	P3IFG &= ~(BIT1);\
						 	P3IE &= (BIT1);}

#define blu_Int_In()    (P3IN  & BIT1)
#define blu_Int_Clear() {P3IFG &= ~BIT1;}
#define blu_Int_Get()   (P3IFG & BIT1)
#define blu_Int_En()    {P3IES |= BIT1;  P3IFG &= ~BIT1; P3IE |= BIT1;}
#define blu_Int_Dis()   {P3IE &= ~BIT1; P3IFG &= ~BIT1;}
#define blu_Int_En_IF() (P3IE & BIT1)



#define	BOX_BLU_CMD_REPLY_HEAD   	0x80
#define BOX_BLU_CMD_RDBOXID		 	0x01
#define BOX_BLU_CMD_OPLOCK		 	0x02 	
#define BOX_BLU_CMD_RUNSENSOR    	0x03
#define BOX_BLU_CMD_LOSERIGHT    	0x04
#define BOX_BLU_CMD_ALARM        	0x05
#define BOX_BLU_CMD_SENSORREPORT	0x06
#define BOX_BLU_CMD_BOXSTATE	 	0x07
#define BOX_BLU_CMD_TAGINFO 	 	0x08

#define BOX_BLU_CMD_COM_HEAD1       0xD5
#define BOX_BLU_CMD_COM_HEAD2       0xC8
#define BOX_BLU_CMD_COM_VER         0x10



#define BLU_DATA_MAXLEN			 128u

#define POWER_IO_ON()			 //(P1OUT |= BIT2)
#define POWER_IO_OFF()           //(P1OUT &= ~BIT2)

#define BLU_TIMEOUT_TIME_PER   	 (1000)    //*< gprs超时的最小粒度

#define BLU_TASK_PRIO       	 (6u)


typedef enum
{
    BLU_SEND_EVENT = ((BLU_TASK_PRIO<<8) | 0x01),
    BLU_STATE_TRANS_EVENT,
    BLU_UART_EVENT,
    BLU_TIMEOUT_EVENT,
    //BLU_NO_DATA_EVENT,
    BOX_BLU_DATA_SENT_EVENT,
    BLU_NO_ACK_EVENT,
    BLU_NO_LOCK_EVENT,
} blu_task_sig_enum_t;

PROCESS_NAME(blu_event_process);

/**
 * @brief 定义BLU的AT指令
 */
typedef enum 
{
    BLU_CMD_NULL,
    BLU_CMD_AT,    
    BLU_CMD_AT_RESET,
    BLU_CMD_AT_VERSION,
    BLU_CMD_AT_LADDR,
    BLU_CMD_AT_NAME,
    BLU_CMD_AT_PIN,
    BLU_CMD_AT_BAUD,
    BLU_CMD_AT_STOP,
    BLU_CMD_AT_PARI,
	BLU_CMD_AT_DEFAULT,
	BLU_CMD_AT_PWRM,
	BLU_CMD_AT_SLEEP,
	BLU_CMD_AT_ROLE,
	BLU_CMD_AT_INQ,
	BLU_CMD_AT_SHOW,
	BLU_CMD_AT_CONN,
	BLU_CMD_AT_POWE,
	BLU_CMD_AT_HELP,
    BLU_SEND_DATA,
    BLU_RECV_DATA,
}blu_cmd_type_e;


typedef enum
{
    BLU_INIT_SIG,
    BLU_TIMEOUT_SIG,
    BLU_SEND_CMD_AT_SIG,
    BLU_SEND_CMD_AT_SLEEP_SIG,
    BLU_SEND_CMD_AT_ROLE_SIG,
    BLU_SEND_CMD_AT_INQ_SIG,
    BLU_SEND_CMD_AT_SHOW_SIG,
    BLU_SEND_CMD_AT_CONN_SIG,
    BLU_DATA_SEND_SIG,
    BLU_DATA_RECEIVE_SIG,
} blu_sig_event_e;


/**
* @brief 定义BLU上电或下电枚举变量
 */
typedef enum
{
    BLU_POWER_ON,
    BLU_POWER_OFF,
}blu_power_operate_e;

/**
* @brief 发送模式定义
 */
typedef enum
{
    BLU_INVALID_MODE,
	BLU_SLEEP_MODE,
	BLU_MASTER_MODE,
	BLU_SLAVE_MODE,
	BLU_STANDBY_MODE,
	BLU_CONNECT_MODE,
}blu_send_mode_e;

/**
* @brief gprs接收数据类型定义
 */
typedef struct{
    uint8_t *blu_data;
    uint16_t sn;
    uint16_t len;
}blu_receive_t;

/**
 *
 * @brief gprs发送反馈信息
 * @param param TRUE或者FALSE，表示发送成功或失败
 *        sn 帧的序列号
 * @return 无
 * 
 */
typedef void (*blu_send_cb_t)(uint16_t param, uint16_t sn);
/**
 *
 * @brief gprs接收反馈信息
 * @param param 接收的数据
 * @return 无
 * 
 */
typedef void (*blu_recv_cb_t)(blu_receive_t param);

/**
 * @brief blu ip和port配置
 */
typedef struct
{
    blu_send_cb_t blu_send_cb;
    blu_recv_cb_t blu_recv_cb;    
}blu_init_cfg_t;

/**
 * @brief 接收app发来的数据信息
 */
typedef struct
{
    uint8_t     blu_data_len;
    //blu_send_mode_e blu_data_mode;
    uint16_t    blu_sn;    
    uint8_t     blu_data[BLU_DATA_MAXLEN];
}blu_data_type_t;

/**
 * @brief 状态
 */
typedef struct blu_fsm_s_
{
    void (*current_state) (struct blu_fsm_s_ *me, blu_sig_event_e sig);
} blu_fsm_t;

/**
 *@brief 定义指向状态切换的函数指针
 */
typedef void (*blu_state_t) (blu_fsm_t *me, blu_sig_event_e sig);

#define BLU_TRAN(state)                                                            \
            hal_int_state_t s;                                                \
            HAL_ENTER_CRITICAL(s);                                             \
            (blu_fsm.current_state = (blu_state_t)(state));                       \
            HAL_EXIT_CRITICAL(s)                                              

#define BLU_FSM_DISPATCH(me, sig)           (blu_fsm.current_state((me), (sig)))
                
/**
 * gprs的操作结果函数回调指针类型
 */
//typedef void (*gprs_power_operate_result_t)(int8_t res);

     
void blu_flush_recv_buf_from_app(void);          

/**
 *
 * @brief IO口初始化（gsm_power、powerkey、reset）
 *        uart初始化、发送缓存和接收缓存的初始化、任务建立与消息绑定
 * @param gprs_cfg ip和port的配置
 * @return 无
 * 
 */
void blu_init(const blu_init_cfg_t *blu_cfg);

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
void blu_tran_send(void *data_p, 
                      uint8_t len,  
                      uint16_t sn);

/**
 *
 * @brief 判断传输模块当前是否可以发送
 * @param 无
 * @return 可以发送则返回TRUE， 反之为FALSE
 * 
 */

void stop_blu_no_lock_timeout(void);

//void blu_no_data_timeout_set(uint32_t ticks);

void blu_no_ack_timeout_set(uint32_t ticks);
void stop_blu_no_ack_timeout(void);

#endif
