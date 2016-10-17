/***************************************************************************
 * @brief       : 
 *
 * @file        : gprs_defs.h
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/12/31
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/12/31    v0.0.1      WangJifang    some notes

***************************************************************************/
#ifndef __GPRS_DEFS_H__
#define __GPRS_DEFS_H__
//#include "common/dev/sim928a/gprs.h"
/**
 * GPRS�豸�����Ľṹ�嶨��
 */
struct gprs_driver
{
    bool_t (*init)(void);
//    int8_t (*prepare)(const uint8_t *payload, uint8_t len);     //*< ������д��FIFO����������δ����
//    int8_t (*transmit)(uint8_t transmit_len);                   //*< ����֮ǰ�Ѿ�׼���õ�����
//
//    int8_t (*send)(const uint8_t *payload, uint8_t len);        //*< prepare & transmit a packet
//    int8_t (*recv)(uint8_t *const buf, uint8_t len);            //*< ��FIFO���ջ����������ȡָ�����ȵ�����
//
//    rf_result_t (*get_value)(rf_cmd_t cmd, void *value);
//    rf_result_t (*set_value)(rf_cmd_t cmd, uint8_t value);
//
//    bool_t (*int_cfg)(rf_int_t state, rf_int_reg_t int_cb, uint8_t type);
};

#endif