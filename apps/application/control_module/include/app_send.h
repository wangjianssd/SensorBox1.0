 /**
 * @brief       : 数据帧的发送处理
 *
 * @file        : app_send.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#ifndef _APP_SEND_H_
#define _APP_SEND_H_
#include <gznet.h>
//#include <data_type_def.h>
#include <app_frame.h>

#define MAX_WAIT_ACK_TIME           60000//接收下行ACK时需要等待的最大时间  
  
typedef struct
{
    uint8_t  result;
    uint16_t frame_sn;
}data_sent_event_arg_t;

typedef struct
{
    uint16_t len;
    uint16_t sn;
    uint8_t data[0];
}data_received_event_arg_t;

typedef enum
{
    BOX_SEND_SSN_OPEN,
    BOX_SEND_SSN_CLOSE,
}e_ssn_ctrl_type_t;

typedef struct
{
    uint8_t type;
    bool_t res;
}ssn_ctrl_event_arg_t;  
  
     
typedef enum
{
    BOX_SEND_NO_CHANNEL,
    BOX_SEND_GPRS_CHANNEL,
    BOX_SEND_SSN_CHANNEL,
    BOX_SEND_UART_CHANNEL,
}e_box_data_channel_t;

typedef enum
{
    BOX_SEND_LIST_PRIORITY_LOW,
	BOX_SEND_LIST_PRIORITY_HIGH,	
	BOX_SEND_LIST_STATUS_WAITING,
	BOX_SEND_LIST_STATUS_SENDING,
}box_send_list_e;

typedef struct
{
    uint16_t seq;
}box_ack_id_t;

void box_blu_send(void);
void box_blu_send_request(box_frame_t *frame);
bool_t box_blu_send_list_add(box_frame_t *frame);
void box_blu_data_sent_event_handle(void *arg);


void box_send_list_init(uint16_t size);

uint16_t box_send_list_length();

bool_t box_send_list_add(box_frame_t *frame, 
                            uint8_t         priority, 
                            uint8_t         allowed_send_times,
                            bool_t          need_ack);

void box_send_list_fetch(box_frame_t **frame,
                            uint16_t        *frame_id);

void box_send_list_sent(uint16_t frame_id, bool_t is_ok);

bool_t box_send_list_is_ack_needed(uint16_t frame_id);

bool_t box_send_list_ack_received(box_ack_id_t *ack_id,
                                     uint16_t        *frame_id);

void box_send_list_no_ack(uint16_t frame_id);

void box_send(void);
void box_send_request(box_frame_t *frame, 
                         uint8_t         priority, 
                         uint8_t         allowed_send_times,
                         bool_t          need_ack);

void box_data_sent_event_handle(void *arg);
void box_data_received_event_handle(void *arg);
void box_wait_ack_timer_event_handle(void *arg);
void box_ssn_ctrl_event_handle(void *arg);
void ssn_open_cb(bool_t res);
void box_send_init(void);
#endif