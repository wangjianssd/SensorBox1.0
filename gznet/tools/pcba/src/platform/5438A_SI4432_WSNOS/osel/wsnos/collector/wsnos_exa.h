/**
 * @brief       : 这个文件定义了消息类型以及任务的优先级
 *
 * @file        : wsnos_exa.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __WSNOS_EXA_H
#define __WSNOS_EXA_H

enum OSEL_SYSTEM_EVNETS
{
	/* 系统定义初始化消息类型 */
	OSEL_INIT_SIG = 0,
	OS_EVENT,
	/* 用户自定义消息类型*/
	MAC_SWITCH_EVENT,
    MAC_MAINTAIN_NETWORK,
	ASSOC_EVENT,
	MAC_RESTART_EVENT,
	//FINSH_EVENT,
	/* M_TRAN */
	M_TRAN_RESEND_TIMEOUT_EVENT,
	M_TRAN_CSMA_TIMEOUT_EVENT,
	M_TRAN_RXOK_EVENT,
	M_TRAN_TXOK_EVENT,
    M_TRAN_RXOVR_EVENT,
	M_TRAN_TX_ERROR_EVENT,
    M_TRAN_TXUND_EVENT,
	M_TRAN_ACK_TIMEOUT_EVENT,
	M_TRAN_BACKOFF_SUCCESS_EVENT,
	M_TRAN_BACKOFF_FAIL_EVENT,

	/* M_SYNC */
	M_SYNC_BACKGROUND,
//    M_SYNC_CALCULATE_EVENT,
	/* M_SLOT */
	M_SLOT_TIMEOUT_EVENT,

	/* DATABUF */
	M_MAC_PRIM_EVENT,
   
    /* moniter */
    M_MAC_MONI_RESTART_EVENT,
    M_MAC_MONI_START_EVENT,
    M_MAC_MONI_STOP_EVENT,
    M_MAC_MONI_SCAN_EVENT,
    M_MAC_MONI_SYNC_EVENT,
    M_MAC_MONI_STARTUP_EVENT,
    
    /* NWK */
    MAC2NWK_PRIM_EVENT,
    APP2NWK_PRIM_EVENT,
    NWK_HEART_EVENT,
    NWK_INLINE_EVENT,
    
    /* APP */
	APP_SERIAL_EVENT,
    APP_SERIAL_DATA_EVENT,
	APP_SOUTH_DATA_EVENT,
	/* 系统定义停止消息类型*/
	OSEL_STOP_SIG
};



#endif

