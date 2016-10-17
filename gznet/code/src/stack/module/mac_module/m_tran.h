/**
 * @brief       : configuration and interface of m_tran.c
 *
 * @file        : m_tran.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */


#ifndef __M_TRAN_H
#define __M_TRAN_H

#include "stack/common/pbuf.h"
#include "stack/common/sbuf.h"
#include "hal_timer.h"

#define M_TRAN_DGB_EN           (0u)

/* 需注册的回调函数类型 */
typedef bool_t (*tran_frm_parse_cb_t)(pbuf_t *const pbuf);
typedef void (*tran_tx_finish_cb_t)(sbuf_t *const sbuf, bool_t result);
typedef pbuf_t *(*tran_frm_get_cb_t)(void);
typedef void (*tran_send_ack_cb_t)(uint8_t seq);

typedef struct _tran_cfg_t
{
    tran_frm_parse_cb_t     frm_head_parse;
    tran_frm_parse_cb_t     frm_payload_parse;
    tran_tx_finish_cb_t     tx_finish;
    tran_frm_get_cb_t       frm_get;
	tran_send_ack_cb_t      send_ack;
} tran_cfg_t;


#if M_TRAN_DGB_EN > 0
typedef struct _tran_tracker
{
    uint8_t data_send_start;
    uint8_t data_send_real;
    uint8_t rf_int;
    uint8_t txok_int;

    uint8_t tx_sfd_int;
    uint8_t tx_sfd_cb;
    uint8_t tx_sfd_ok;
    uint8_t txok_cb;
    uint8_t txok_msg_deal;
    uint8_t tx_success_without_ack;
    uint8_t tx_error_cb;
    uint8_t tx_error_msg_deal;

    uint8_t waitting_ack;
    uint8_t ack_tout_cb;
    uint8_t ack_tout_msg_deal;
    uint8_t ack_tout_recved_ack;
    uint8_t tx_resend;
    uint8_t tx_finish_fail;
    uint8_t recv_data;
    uint8_t recv_ack;
    uint8_t tx_success_ack;

    uint8_t tx_und_int;
    uint8_t tx_und_cb;
    uint8_t tx_und_msg_deal;

} tran_tracker_t;

typedef struct _tran_recv_tracker_
{
    uint16_t rxok_int_cb_real_tick;
    uint16_t rxok_int_cb_tick;
    uint16_t rxok_msg_deal_tick;
    uint16_t txok_int_cb_tick;
    uint16_t txok_msg_deal_tick;
} tran_recv_tracker_t;
#endif

/**
 * @brief 设置RX_SFD的状态
 */ 
void tran_rx_sfd_set(bool_t res);

/**
 * @brief 获取RX_SFD的状态
 */
bool_t tran_rx_sfd_get(void);

/**
 * 初始化传输模块
 *
 * @param:
 *
 * @return:
 */
void m_tran_init(osel_task_t *task);

/**
 * 传输模块配置
 *
 * @param tran_cb_reg:指向传输模块参数配置实体
 *
 * @return: 无
 */
void m_tran_cfg(const tran_cfg_t *const tran_cb_reg);

/**
 * 发送一个帧
 *
 * @param  sbuf: 需要发送的sbuf的指针
 * @param  tx_finish: tx_finish回调
* @param  resend_times: 重传次数
 * @return: 无
*/
void m_tran_send(sbuf_t *const sbuf, tran_tx_finish_cb_t tx_finish, uint8_t resend_times);

/**
 * 设置模块为接收状态
 *
 * @param: 无
 *
 * @return: 无
 */
void m_tran_recv(void);

/**
 * 设置模块为IDLE态
 *
 * @param: 无
 *
 * @return: 无
 */
void m_tran_stop(void);

/**
 * 设置模块为休眠状态
 *
 * @param: 无
 *
 * @return: 无
 */
void m_tran_sleep(void);

/**
 * 判断传输模块当前是否可以发送
 *
 * @param: 无
 *
 *@return: 可以发送则返回TRUE， 反之为FALSE
 */
bool_t m_tran_can_send(void);

#endif

