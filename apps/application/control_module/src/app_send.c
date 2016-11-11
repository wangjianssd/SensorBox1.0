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
#include <gznet.h>
#include <app_send_list.h>
#include <app_send.h>
#include <string.h>
//#include <list.h>
//#include <pbuf.h>
#include <app_frame.h>
#include <ssn.h>
#include <gprs_tx.h>
#include <gps.h>
#include "blu_tx.h"
#include "elec_lock.h"
#include <hal_rfid.h>
#include <app_task.h>

extern osel_etimer_t wait_ack_timer;
void box_send(void);
typedef struct
{
    uint8_t        priority;
    uint16_t       frame_id;
    uint8_t        status;
    uint8_t        allowed_send_times;
    bool_t         need_ack;
    box_frame_t frame;
}box_send_list_obj_t;

//发送模块全局变量定义
static send_list_t box_send_list;
static send_list_t box_blu_send_list;

static uint16_t box_send_list_size = 0;
static uint16_t box_blu_send_list_size = 0;

e_box_data_channel_t box_current_data_channel = BOX_SEND_SSN_CHANNEL;
//static bool_t is_sending = FALSE;
bool_t is_sending = FALSE;
bool_t blu_is_sending = FALSE;


uint16_t sending_frame_sn = 0;
uint16_t blu_sending_frame_sn = 0;


uint16_t blu_alarm_cn=0;
extern uint8_t queclocator;
extern bool_t blu_if_connected;
extern bool_t blu_alarm_send_success;
//extern bool_t blu_if_can_send;

extern blu_cmd_type_e blu_cmd_type;

//box_frame_t globalframe;
//uint8_t box_uid[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06};//0x0000000000000001 
uint8_t box_uid[8] = {0x00};
gps_simple_info_t box_location;
sensor_info_t box_sensor;



#define BUILD_IP_ADDRESS(b3, b2, b1, b0) ((uint32_t)(b3) << 24) | \
        ((uint32_t)(b2) << 16) | ((uint32_t)(b1) << 8) | ((uint32_t)(b0))
            
/**
* @breif 比较两个发送对象的优先级
* @return TTRUE  escort_obj_a应该排在box_obj_b的前面, 优先发送
* -       FALSE  escort_obj_a应该排在box_obj_b的后面, 推迟发送
*/
static bool_t box_send_list_priority_cmp(void *box_obj_a,
                                            void *box_obj_b)
{
    box_send_list_obj_t *a;
    box_send_list_obj_t *b;
    
    a = (box_send_list_obj_t *)box_obj_a;
    b = (box_send_list_obj_t *)box_obj_b;
    
    return (a->priority > b->priority);
}

/**
* @breif 检查发送对象的标识
*/
static bool_t box_send_list_check_frame_id(void *escort_obj,
                                              void *frame_id)
{
    box_send_list_obj_t *obj;
    uint16_t *id;
    
    obj = (box_send_list_obj_t *)escort_obj;
    id = (uint16_t *)frame_id;
    
    return (obj->frame_id == *id);
}

/**
* @breif 检查是否是对应的ACK
*/
static bool_t box_send_list_check_ack(void *escort_obj,
                                         void *ack_id)
{
    box_send_list_obj_t *obj;
//    box_ack_id_t *id;
    uint16_t *id;
    
    obj = (box_send_list_obj_t *)escort_obj;
    id = (uint16_t *)ack_id;
    
    return (obj->frame_id == *id);
}
                                  
/**
* @breif 检查是否是等待发送的状态
*/
static bool_t box_send_list_check_waiting(void *escort_obj, void *arg)
{
    box_send_list_obj_t *obj;
    
    (void)arg;
    obj = (box_send_list_obj_t *)escort_obj;
    
    return (obj->status == BOX_SEND_LIST_STATUS_WAITING);
}

/**
* @breif 检查优先级是否是 BOX_SEND_LIST_PRIORITY_LOW
*/
static bool_t box_send_list_check_priority_low(void *escort_obj, void *arg)
{
    box_send_list_obj_t *obj;
    
    (void)arg;
    obj = (box_send_list_obj_t *)escort_obj;
    
    return (obj->priority == BOX_SEND_LIST_PRIORITY_LOW);
}

/**
* @breif 检查优先级是否是 BOX_SEND_LIST_PRIORITY_HIGH
*/
static bool_t box_send_list_check_priority_high(void *escort_obj, void *arg)
{
    box_send_list_obj_t *obj;
    
    (void)arg;
    obj = (box_send_list_obj_t *)escort_obj;
    
    return (obj->priority == BOX_SEND_LIST_PRIORITY_HIGH);
}

/**
* @breif 获取链表的长度
*/
uint16_t box_send_list_length()
{
    return send_list_length(&box_send_list);
}

/**
* @breif 把新对象插入链表
*/
bool_t box_send_list_add(box_frame_t *frame, 
                            uint8_t         priority, 
                            uint8_t         allowed_send_times,
                            bool_t          need_ack)
{   
    box_send_list_obj_t send_obj;
    
    send_obj.priority = priority;
    send_obj.allowed_send_times = allowed_send_times;
    send_obj.need_ack = need_ack;
    send_obj.frame = *frame;

	if((sending_frame_sn == 0)||(sending_frame_sn == 0xffff))
    {
        sending_frame_sn = 1;
    }
    else
    {
        sending_frame_sn++;
    }
	
	send_obj.frame_id = sending_frame_sn;
	
    send_obj.status = BOX_SEND_LIST_STATUS_WAITING;
    
    if (send_list_length(&box_send_list) == box_send_list_size)
    {
        box_send_list_obj_t *obj;
        
        obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                        box_send_list_check_priority_low,
                                        NULL);
        if (NULL != obj)
        {
            send_list_remove(&box_send_list, obj);
        }
        else
        {
            obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                            box_send_list_check_priority_high,
                                            NULL);
            DBG_ASSERT(obj != NULL __DBG_LINE);
            if (NULL != obj)
            {
                send_list_remove(&box_send_list, obj);
            }
        }
    } 
    
    return send_list_add(&box_send_list, &send_obj);
}

/**
* @breif 从链表中拿取对象，并读取此对象的序列号
*/
void box_send_list_fetch(box_frame_t **frame,
                            uint16_t        *frame_id)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                    box_send_list_check_waiting, NULL);
    if (NULL != obj)
    {
        obj->status = BOX_SEND_LIST_STATUS_WAITING;
        
        obj->allowed_send_times--;
        *frame = &obj->frame;
        *frame_id = obj->frame_id;        
        return;
    }
    
    *frame = NULL;
    *frame_id = 0xFFFF;
    
    return;
}

/**
* @breif 对象根据发送成功与否来做是否从链表中删除的操作
*/
void box_send_list_sent(uint16_t frame_id, bool_t is_ok)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                    box_send_list_check_frame_id, &frame_id);
    if (NULL != obj)
    {
        if (is_ok)
        {
            send_list_remove(&box_send_list, obj);
        }
        else
        {
            obj->status = BOX_SEND_LIST_STATUS_WAITING;
        }
    }
}

/**
* @breif 判断对象是否需要下行的ACK
*/
bool_t box_send_list_is_ack_needed(uint16_t frame_id)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                    box_send_list_check_frame_id, &frame_id);
    if (NULL != obj)
    {
        if (obj->need_ack)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
* @breif 根据下行确认的对象，从链表中找到此对象，并删除
*/
bool_t box_send_list_ack_received(box_ack_id_t *ack_id,
                                     uint16_t        *frame_id)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                    box_send_list_check_ack, ack_id);
    if (NULL != obj)
    {
        DBG_ASSERT(obj->need_ack __DBG_LINE);
        *frame_id = obj->frame_id;
        send_list_remove(&box_send_list, obj);
        
        return TRUE;
    }
    
    return FALSE;
}

/**
* @breif 当一个对象，没有得到下行的ACK后，根据序列号在链表中找到此对象，并不删除
* - 等待下次的重传
*/
void box_send_list_no_ack(uint16_t frame_id)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_send_list, 
                                    box_send_list_check_frame_id, &frame_id);
    if (NULL != obj)
    {
        DBG_ASSERT(obj->need_ack __DBG_LINE);
        obj->status = BOX_SEND_LIST_STATUS_WAITING;
        if (obj->allowed_send_times == 0)
        {
            send_list_remove(&box_send_list, obj);
        }
    }
}

/**
* @breif 根据链表的大小，初始化业务需要的链表
*/
void box_send_list_init(uint16_t size)
{
    box_send_list_size = size;
	box_blu_send_list_size = size;
	
    sending_frame_sn = 0;
	blu_sending_frame_sn = 0;
	
    send_list_init(&box_send_list, sizeof(box_send_list_obj_t), 
                   box_send_list_priority_cmp);

    send_list_init(&box_blu_send_list, sizeof(box_send_list_obj_t), 
                   box_send_list_priority_cmp);
	
}


//-----------------------------------------------
/**
* @breif 把新对象插入链表
*/
bool_t box_blu_send_list_add(box_frame_t *frame)	
{   
    box_send_list_obj_t send_obj;
    send_obj.frame = *frame;

	if((blu_sending_frame_sn == 0)||(blu_sending_frame_sn == 0xffff))
		blu_sending_frame_sn = 1;
	else
		blu_sending_frame_sn += 1;
	
	send_obj.frame_id = blu_sending_frame_sn;
	
    send_obj.status = BOX_SEND_LIST_STATUS_WAITING;
    
    if (send_list_length(&box_blu_send_list) == box_blu_send_list_size)
    {
        box_send_list_obj_t *obj;
        
        obj = (box_send_list_obj_t *)send_list_find_first(&box_blu_send_list, 
                                        box_send_list_check_priority_low,
                                        NULL);
        if (NULL != obj)
        {
            send_list_remove(&box_blu_send_list, obj);
        }
        else
        {
            obj = (box_send_list_obj_t *)send_list_find_first(&box_blu_send_list, 
                                            box_send_list_check_priority_high,
                                            NULL);
            DBG_ASSERT(obj != NULL __DBG_LINE);
            if (NULL != obj)
            {
                send_list_remove(&box_blu_send_list, obj);
            }
        }
    } 
    
    return send_list_add(&box_blu_send_list, &send_obj);
}

/**
* @breif 从链表中拿取对象，并读取此对象的序列号
*/
void box_blu_send_list_fetch(box_frame_t **frame,
                            uint16_t        *frame_id)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_blu_send_list, 
                                    box_send_list_check_waiting, NULL);
    if (NULL != obj)
    {
        obj->status = BOX_SEND_LIST_STATUS_WAITING;
        
        obj->allowed_send_times--;
        *frame = &obj->frame;
        *frame_id = obj->frame_id;        
        return;
    }
    
    *frame = NULL;
    *frame_id = 0xFFFF;
    
    return;
}

/**
* @breif 对象根据发送成功与否来做是否从链表中删除的操作
*/
void box_blu_send_list_sent(uint16_t frame_id, bool_t is_ok)
{
    box_send_list_obj_t *obj;
    
    obj = (box_send_list_obj_t *)send_list_find_first(&box_blu_send_list, 
                                    box_send_list_check_frame_id, &frame_id);
    if (NULL != obj)
    {
        if (is_ok)
        {
            send_list_remove(&box_blu_send_list, obj);
        }
        else
        {
            obj->status = BOX_SEND_LIST_STATUS_WAITING;
        }
    }
}


#if 0
/**
* @breif 接收下行数据时需要启动定时器,等待ACK
*/
static void start_wait_ack_timer(uint32_t time, void *arg)
{
    osel_etimer_arm(&wait_ack_timer,(time/OSEL_TICK_PER_MS),0);
}
#endif
                                         
static void close_wait_ack_timer(void)
{
    osel_etimer_disarm(&wait_ack_timer);
}
void box_wait_ack_timer_event_handle(void *arg)
{
    uint16_t frame_sn;
    
    frame_sn = *(uint16_t *)arg;
    box_send_list_sent(frame_sn, FALSE);
    is_sending = FALSE;
    box_send();
}

/**
* @breif gprs回调处理
*/
static void gprs_send_cb(uint16_t param,uint16_t sn)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_sent_event_arg_t *arg;    
    pbuf = pbuf_alloc(sizeof(data_sent_event_arg_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_sent_event_arg_t *)pbuf->data_p;
    arg->result = param;
    arg->frame_sn = sn;    
//    osel_post(BOX_DATA_SENT_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    
    event.sig = BOX_DATA_SENT_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);    
}

static void gprs_receive_cb(gprs_receive_t param)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_received_event_arg_t *arg;
    
    pbuf = pbuf_alloc(sizeof(data_received_event_arg_t) + param.len __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_received_event_arg_t *)pbuf->data_p;
    arg->len = param.len;
    arg->sn = param.sn;
    memcpy(arg->data, param.gprs_data, param.len);
    
//    osel_post(BOX_DATA_RECEIVED_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    event.sig = BOX_DATA_RECEIVED_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);     
}


/**
* @breif gprs回调处理
*/
static void blu_send_cb(uint16_t param,uint16_t sn)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_sent_event_arg_t *arg;    
    pbuf = pbuf_alloc(sizeof(data_sent_event_arg_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_sent_event_arg_t *)pbuf->data_p;
    arg->result = param;
    arg->frame_sn = sn;    
//    osel_post(BOX_DATA_SENT_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    
    event.sig = BOX_BLU_DATA_SENT_EVENT;
    event.param = (osel_param_t *)pbuf;
    //osel_post(NULL, &app_task_thread_process, &event);
	osel_post(NULL, &blu_event_process, &event);

}

static void blu_receive_cb(blu_receive_t param)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_received_event_arg_t *arg;
    
    pbuf = pbuf_alloc(sizeof(data_received_event_arg_t) + param.len __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_received_event_arg_t *)pbuf->data_p;
    arg->len = param.len;
    arg->sn = param.sn;
    memcpy(arg->data, param.blu_data, param.len);
    
//    osel_post(BOX_DATA_RECEIVED_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
//    event.sig = BOX_DATA_RECEIVED_EVENT;
//    event.param = (osel_param_t *)pbuf;
//    osel_post(NULL, &app_task_thread_process, &event);     
}

/**
* @breif ssn回调处理
*/
static void ssn_send_cb(uint8_t *data, uint8_t len, uint8_t res, uint8_t mode)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_sent_event_arg_t *arg;
	uint8_t buf[MAX_BOX_FRAME_LENGTH];    

    pbuf = pbuf_alloc(sizeof(data_sent_event_arg_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_sent_event_arg_t *)pbuf->data_p;
    osel_memcpy(buf, data, len);
    arg->result = res;
    arg->frame_sn = 0x00;
//    osel_post(BOX_DATA_SENT_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    event.sig = BOX_DATA_SENT_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);     
}

static void ssn_receive_cb(uint8_t *data, uint8_t len)
{
    osel_event_t event;
    pbuf_t *pbuf;
    data_received_event_arg_t *arg;
    
    pbuf = pbuf_alloc(sizeof(data_received_event_arg_t) + len __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (data_received_event_arg_t *)pbuf->data_p;
    arg->len = len;
    memcpy(arg->data, data, len);
    
//    osel_post(BOX_DATA_RECEIVED_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    event.sig = BOX_DATA_RECEIVED_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);    
}

void ssn_open_cb(bool_t res)
{
    osel_event_t event;
    pbuf_t *pbuf;
    ssn_ctrl_event_arg_t *arg;
    
    pbuf = pbuf_alloc(sizeof(ssn_ctrl_event_arg_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (ssn_ctrl_event_arg_t *)pbuf->data_p;
    arg->type = BOX_SEND_SSN_OPEN;
    arg->res = res;
    
//    osel_post(BOX_SSN_CONTROL_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    event.sig = BOX_SSN_CONTROL_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);     
}

void ssn_close_cb(bool_t res)
{
    osel_event_t event;
    pbuf_t *pbuf;
    ssn_ctrl_event_arg_t *arg;
    
    pbuf = pbuf_alloc(sizeof(ssn_ctrl_event_arg_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
    arg = (ssn_ctrl_event_arg_t *)pbuf->data_p;
    arg->type = BOX_SEND_SSN_CLOSE;
    arg->res = res;
    
//    osel_post(BOX_SSN_CONTROL_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    event.sig = BOX_SSN_CONTROL_EVENT;
    event.param = (osel_param_t *)pbuf;
    osel_post(NULL, &app_task_thread_process, &event);     
}

/**
* @breif ssn打开或关闭的回调处理
*/
void box_ssn_ctrl_event_handle(void *arg)
{
    pbuf_t *pbuf;
    ssn_ctrl_event_arg_t *cb_arg;
    box_frame_t box_frame;
    application_info_t application_info;
    uint8_t profile_md5[16] = {0x00};
    
    pbuf = (pbuf_t *)arg;
    cb_arg = (ssn_ctrl_event_arg_t *)pbuf->data_p;
    if (cb_arg->type == BOX_SEND_SSN_OPEN)
    {
        if (cb_arg->res == TRUE)
        {
//            box_current_data_channel = BOX_SEND_SSN_CHANNEL;
            //把设备启动帧发给后台
            uint8_t ret = hal_rfid_updata_application_info(&application_info);
            DBG_ASSERT(TRUE == ret __DBG_LINE);
            
            box_frame.frame_type = BOX_REG_DEVICE_FRAME;
            osel_memcpy(box_frame.box_type_frame_u.box_device_info.profile_id,
                        application_info.profile_info.id,4*sizeof(uint8_t));
            //调用接口算出profile的MD5值
            
            osel_memcpy(box_frame.box_type_frame_u.box_device_info.profile_md5,
                        profile_md5,
                        16*sizeof(uint8_t));
            
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
			box_send();
        }
        else
        {
//            box_current_data_channel = BOX_SEND_GPRS_CHANNEL;
        }          
    }
    else if(cb_arg->type == BOX_SEND_SSN_CLOSE)
    {
//        box_current_data_channel = BOX_SEND_GPRS_CHANNEL;
    }
    
    is_sending = FALSE;
    box_send();    
    pbuf_free(&pbuf __PLINE1);
    DBG_ASSERT(pbuf == NULL __DBG_LINE);
}

/**
* @breif ssn和GPRS发送回调处理
*/
void box_data_sent_event_handle(void *arg)
{
    pbuf_t *pbuf;
    data_sent_event_arg_t *cb_arg;
    uint16_t sn = 0;
    
    pbuf = (pbuf_t *)arg;
    cb_arg = (data_sent_event_arg_t *)pbuf->data_p;
    sn = cb_arg->frame_sn;
    
    if (cb_arg->result == TRUE)
    {
        box_send_list_sent(sn, TRUE);
    }
    else
    {
        box_send_list_sent(sn, FALSE);        
    }
    
    is_sending = FALSE;
    box_send();    
    pbuf_free(&pbuf __PLINE1);
    DBG_ASSERT(pbuf == NULL __DBG_LINE);
}

/**
* @breif ssn和GPRS接收回调处理
*/
void box_data_received_event_handle(void *arg)
{
    pbuf_t *pbuf;
    data_received_event_arg_t *cb_arg;
    uint16_t data_sn;
    uint16_t data_len;
    uint8_t server_data[128] = {0x00};
    coap_pdu_t coap_pdu = {server_data,0,80};
    uint8_t code_class = 0;
   // uint8_t code_detail = 0;   

    pbuf = (pbuf_t *)arg;
    cb_arg = (data_received_event_arg_t *)pbuf->data_p;
    data_sn = cb_arg->sn;
    data_len = cb_arg->len;
    
    osel_memcpy(server_data,cb_arg->data,data_len);
    coap_pdu.buf = server_data;
    coap_pdu.len = data_len;

    code_class = coap_get_code_class(&coap_pdu);
 //   code_detail = coap_get_code_detail(&coap_pdu);
//    static uint8_t temp;
//    temp = code_detail;
    
    close_wait_ack_timer();
//    if (NULL != wait_ack_timer)
//    {
//        hal_timer_cancel(&wait_ack_timer);
//        DBG_ASSERT(wait_ack_timer == NULL __DBG_LINE);
//    }
    
    if (2 == code_class)
    {
        box_send_list_sent(data_sn, TRUE);
    }
    else
    {
        box_send_list_sent(data_sn, FALSE); 
    }  
    
    is_sending = FALSE;
    box_send();    
    pbuf_free(&pbuf __PLINE1);
    DBG_ASSERT(pbuf == NULL __DBG_LINE);
}

/**
* @breif 组帧操作
*/
static uint8_t box_reg_device_frame_proc(box_frame_t *frame,
                                      coap_pdu_t *pdu,uint64_t uid,uint16_t frame_sn)
{
    box_device_info_t device_info;
    uint8_t payload[32];
    uint8_t payload_len = 0;
    uint8_t coap_request_len = 0; 

    device_info.user_id = 0;
 #if 0
    device_info.user_id = frame->box_type_frame_u.box_device_info.user_id;        // 用户ID 0x11223344
    device_info.timestamp = frame->box_type_frame_u.box_device_info.timestamp;      // 时间戳  0xAABBCCDD
    osel_memcpy(device_info.profile_id,
                frame->box_type_frame_u.box_device_info.profile_id,4);
    osel_memcpy(device_info.profile_md5,
                frame->box_type_frame_u.box_device_info.profile_md5,16);
#endif    
    // payload的最大长度为32字节，调用make_xxx可返回payload真实长度
    payload_len = make_device_payload(device_info, payload, 32);
    coap_request_len = coap_post_device_request(pdu, uid, payload, payload_len,frame_sn);
    return coap_request_len;
}

static uint8_t box_sensor_frame_proc(box_frame_t *frame,
                                  coap_pdu_t *pdu,uint64_t uid,uint16_t frame_sn)
{
    sensor_info_t sensor_info;
    uint8_t payload[32];
    uint8_t payload_len = 0;
    uint8_t coap_request_len = 0;

    sensor_info.type = frame->box_type_frame_u.sensor_info.type;      // 类型SENSOR_UNLOCK
    sensor_info.len = frame->box_type_frame_u.sensor_info.len;                   // 长度
    sensor_info.user_id = frame->box_type_frame_u.sensor_info.user_id;      // 用户ID 0x11223344
    sensor_info.timestamp = frame->box_type_frame_u.sensor_info.timestamp;    // 时间戳  0x01234567
  
    if (SENSOR_CARGO == sensor_info.type)
    {
        osel_memcpy(sensor_info.content,frame->box_type_frame_u.sensor_info.content,16);//货物信息的MD5值
    }  
	else if (sensor_info.type == SENSOR_TMP_HUM)
	{
        osel_memcpy(sensor_info.content,frame->box_type_frame_u.sensor_info.content,8);//温湿度传感器信息值，4Byte 温度，4Byte湿度
	}

    // payload的最大长度为32字节，调用make_xxx可返回payload真实长度
    payload_len = make_sensor_payload(sensor_info, payload, 34);
    coap_request_len = coap_post_sensor_request(pdu, uid, payload, payload_len,frame_sn);
    return coap_request_len;    
}

static uint8_t box_alarm_frame_proc(box_frame_t *frame,
                                 coap_pdu_t *pdu,uint64_t uid,uint16_t frame_sn)
{
    alarm_info_t alarm_info;
    uint8_t payload[4];
    uint8_t payload_len = 0;
    uint8_t coap_request_len = 0;

    alarm_info.type = frame->box_type_frame_u.alarm_info.type;//ALARM_T_OVERRUN

    if(blu_Int_In() != 0)
    {
        alarm_info.sn = TRUE;
    }
    else
    {
        alarm_info.sn = FALSE;
    }
	//alarm_info.sn = blu_alarm_cn;
	
    // payload的最大长度为32字节，调用make_xxx可返回payload真实长度
    payload_len = make_alarm_payload(alarm_info, payload, 4);
    coap_request_len = coap_post_alarm_request(pdu, uid, payload, payload_len,frame_sn);
    return coap_request_len;
}

static uint8_t box_location_frame_proc(box_frame_t *frame,
                                    coap_pdu_t *pdu,uint64_t uid,uint16_t frame_sn)
{
    location_info_t location_info;
    uint8_t payload[32];
    uint8_t payload_len = 0;
    uint8_t coap_request_len = 0;
    
    location_info.type = frame->box_type_frame_u.location_info.type;              // 类型
    location_info.len = frame->box_type_frame_u.location_info.len;                  // 长度
    location_info.gps_info.latitude = frame->box_type_frame_u.location_info.gps_info.latitude;
    location_info.gps_info.longitude = frame->box_type_frame_u.location_info.gps_info.longitude;

    // payload的最大长度为32字节，调用make_xxx可返回payload真实长度
    payload_len = make_location_payload(location_info, payload, 32);
    coap_request_len = coap_post_location_request(pdu, uid, payload, payload_len,frame_sn);    
    return coap_request_len;
}

static uint8_t box_heart_frame_proc(box_frame_t *frame,
                                 coap_pdu_t *pdu,uint64_t uid,uint16_t frame_sn)
{
    heart_info_t heart_info;
    uint8_t payload[8];
    uint8_t payload_len = 0;
    uint8_t coap_request_len = 0;
    
    //heart_info.ssn_remain_energy = frame->box_type_frame_u.heart_info.ssn_remain_energy; 
    heart_info.lock_remain_energy = frame->box_type_frame_u.heart_info.lock_remain_energy;  
	heart_info.box_operation_status = frame->box_type_frame_u.heart_info.box_operation_status;
	  
    // payload的最大长度为32字节，调用make_xxx可返回payload真实长度
    payload_len = make_heart_payload(heart_info, payload, 6);
    coap_request_len = coap_post_heart_request(pdu, uid, payload, payload_len,frame_sn);
    return coap_request_len;
}

extern void stop_blu_no_ack_timeout(void); 
extern blu_data_type_t blu_data_recv_from_app;

void box_blu_send(void)
{
    uint8_t buf[12 * 20 + 2] = {0}; 
	uint8_t coap_request_len = 0;
	box_frame_t *frame;
	osel_event_t event;

    if (blu_is_sending)
    {
        _NOP();
        return;
    }

    delay_ms(10);
    //链表取数据
    box_blu_send_list_fetch(&frame, &blu_sending_frame_sn);    
    if (NULL == frame)
    {
        blu_is_sending = FALSE;
        blu_sending_frame_sn = 0;
        
   		stop_blu_no_ack_timeout();
		
        is_sending = FALSE;
        box_send();        
        return;
    }

	if(blu_Int_In() != 0)
    {
	    blu_if_connected = TRUE;
    }
    else
	{
		blu_is_sending = FALSE;
        blu_if_connected = FALSE;

		blu_data_recv_from_app.blu_sn = blu_sending_frame_sn;
		event.sig = BLU_NO_ACK_EVENT;
		osel_post(NULL, &blu_event_process, &event);
		
        return;
    }
    
	if(blu_if_connected == TRUE)
	{
		switch (frame->frame_type)
		{
		    case BOX_SENSOR_FRAME:

				switch(frame->box_type_frame_u.sensor_info.type)
				{
					case SENSOR_UNLOCK://开锁

						coap_request_len = sizeof(box_uid)+ 1;  //8+1;
	
						buf[0] = BOX_BLU_CMD_BOXSTATE;

						buf[9] = 0x02;
						if(elec_lock_State == lock_Open)
							buf[9] = 0x01;
						
						break;
#if 0				
					case SENSOR_TMP_HUM://暂时由于开发变更，不在使用这个分支
						memcpy((void*)&buf[9],(void*)&frame->box_type_frame_u.sensor_info.temperature_value,4);
						memcpy((void*)&buf[9+4],(void*)&frame->box_type_frame_u.sensor_info.humidity_value,4);
						coap_request_len = sizeof(box_uid)+ sizeof(float) + sizeof(float);  //8+4+4;
						buf[0] = BOX_BLU_CMD_SENSORREPORT;						
						break;
#endif						
					default:
						return;
				}
				
		        break;
				
		    case BOX_ALARM_FRAME://开启预警
				buf[9] = frame->box_type_frame_u.alarm_info.type;
				
				buf[0] = BOX_BLU_CMD_ALARM;//0x05;

				coap_request_len = 8+1+2;
				blu_alarm_cn = 1;
				buf[10] = (uint8_t)(blu_alarm_cn>>8);
				buf[11] = (uint8_t)blu_alarm_cn;
				
		        break;
		     case BOX_TAG_INFO_FRAME:
                buf[0] = BOX_BLU_CMD_TAGINFO + BOX_BLU_CMD_REPLY_HEAD;
                buf[9] = frame->box_type_frame_u.tag_info.tag_count;
                coap_request_len = 8 + 1 + 12 * buf[9];   
                memcpy((void*)&buf[10],(void*)&frame->box_type_frame_u.tag_info.data, 12 * buf[9]);
                break;
		    default :
				blu_is_sending = FALSE;
		        return;
		        
		}

		memcpy((void*)&buf[1],(void*)box_uid,sizeof(box_uid));
		
		blu_cmd_type = BLU_SEND_DATA;
        
		blu_alarm_send_success = FALSE;
		blu_is_sending = TRUE;
		blu_data_recv_from_app.blu_sn = blu_sending_frame_sn;
        blu_no_ack_timeout_set(6000);
        
		blu_tran_send(buf,coap_request_len,blu_sending_frame_sn);
	}	
	else 
	{
		blu_is_sending = FALSE;

		blu_data_recv_from_app.blu_sn = blu_sending_frame_sn;
		event.sig = BLU_NO_ACK_EVENT;
		osel_post(NULL, &blu_event_process, &event);
		
		return;
	}
}

void box_blu_send_request(box_frame_t *frame)
{
    bool_t result;
	
    result = box_blu_send_list_add(frame);
    DBG_ASSERT(result == TRUE __DBG_LINE);
	
    box_blu_send();
}


void box_blu_data_sent_event_handle(void *arg)
{
    pbuf_t *pbuf;
    data_sent_event_arg_t *cb_arg;
    uint16_t sn = 0;
    
    pbuf = (pbuf_t *)arg;
    cb_arg = (data_sent_event_arg_t *)pbuf->data_p;
    sn = cb_arg->frame_sn;
    
    if (cb_arg->result == TRUE)
    {
        box_blu_send_list_sent(sn, TRUE);
    }
    else
    {
        box_blu_send_list_sent(sn, FALSE);        
    }
    
    blu_is_sending = FALSE;
    box_blu_send();    
    pbuf_free(&pbuf __PLINE1);
    DBG_ASSERT(pbuf == NULL __DBG_LINE);
}



void box_send(void)
{ 
    box_frame_t *frame;// = globalframe;
    uint16_t frame_sn;
    uint8_t buf[MAX_BOX_FRAME_LENGTH];    
    coap_pdu_t pdu = {buf, 0, MAX_BOX_FRAME_LENGTH};
    uint8_t coap_request_len = 0;
    uint64_t uid = 0;//0x1234567890AABBCC; //0x0000000000000001;//     // UID长度为8字节    
    gprs_send_mode_e gprs_send_mode = INVALID_MODE;
    
    if (is_sending)
    {
        _NOP();
        return;
    }
	
    if (box_current_data_channel == BOX_SEND_NO_CHANNEL)
    {
        is_sending = FALSE;
        
        return;
    }
		
    //先判断当前需要选择的网络
    if (TRUE == ssn_get_status())
    {
        box_current_data_channel = BOX_SEND_SSN_CHANNEL;
    }
    else
    {
        box_current_data_channel = BOX_SEND_GPRS_CHANNEL;                      
    }  
    //链表取数据
    box_send_list_fetch(&frame, &frame_sn);

//如果蓝牙模块正在发送并且未发送成功  blu_if_connected = TRUE;  
    //if ((blu_alarm_send_success == FALSE)&&(blu_is_sending == TRUE))
//    if (blu_if_connected == TRUE)
    if (blu_is_sending == TRUE)        
    {
        gprs_send_cb(FALSE,frame_sn);
    }
    
    if (NULL == frame)
    {
        is_sending = FALSE;
        
        return;
    }
    
   	for(uint8_t i=0; i<8; i++)
	{
        uid <<= 8;    
		uid |= box_uid[i];
	}
    
    //组装coap帧
    switch (frame->frame_type)
    {
        case BOX_REG_DEVICE_FRAME:
            coap_request_len = box_reg_device_frame_proc(frame,&pdu,uid,frame_sn);
            break;
        case BOX_SENSOR_FRAME:
            coap_request_len = box_sensor_frame_proc(frame,&pdu,uid,frame_sn);
            break;
        case BOX_ALARM_FRAME:
			coap_request_len = box_alarm_frame_proc(frame,&pdu,uid,frame_sn);
            break;
        case BOX_LOCATION_FRAME:
            coap_request_len = box_location_frame_proc(frame,&pdu,uid,frame_sn);
            break;
        case BOX_HEART_FRAME:
            coap_request_len = box_heart_frame_proc(frame,&pdu,uid,frame_sn);
            break;
            
        default :
            break;
            
    }

  
    // 选择通道发送
    if (box_current_data_channel == BOX_SEND_SSN_CHANNEL)
    {
        ssn_send(ssn_send_cb, buf,coap_request_len,RELIABLE_MODE);//RELIABLE_MODE  UNRELIABLE_MODE
    }
    else if (box_current_data_channel == BOX_SEND_GPRS_CHANNEL)
    {
        gprs_send_mode = UDP_CONTINE;//UDP_CONTINE
        gprs_tran_send(buf,coap_request_len,gprs_send_mode,frame_sn);
    }

    is_sending = TRUE;
}


/**
* @breif 插入链表的接口
*/
void box_send_request(box_frame_t *frame, 
                         uint8_t         priority, 
                         uint8_t         allowed_send_times,
                         bool_t          need_ack)
{
    bool_t result;
	
    result = box_send_list_add(frame, priority, allowed_send_times, 
                                  need_ack);
    DBG_ASSERT(result == TRUE __DBG_LINE);
}

/**
* @breif 发送初始化
*/
void box_send_init(void)
{
    gprs_init_cfg_t gprs_cfg;

	box_sensor.user_id = 0x11223344;
	box_sensor.timestamp = 0x55667788;
	box_sensor.temperature_value = 0;
	box_sensor.humidity_value = 0;

//����ʡ��������������·18�Ź�������԰˫����A ��
	box_location.longitude = 0;//120.3801434053; //baidu  //120.373694; �ߵ�
	box_location.latitude =  0;//31.4958789655;  //baidu  //31.489608; �ߵ�
	
    is_sending = FALSE;
    box_current_data_channel = BOX_SEND_SSN_CHANNEL;
    box_send_list_init(15);

    //配置gprs模块
    gprs_cfg.gprs_send_cb = gprs_send_cb;
    gprs_cfg.gprs_recv_cb = gprs_receive_cb;

	//gprs_cfg.ip_addr = BUILD_IP_ADDRESS(123, 59, 86, 188);  //old
    gprs_cfg.ip_addr = BUILD_IP_ADDRESS(43,241,222,234);   //new
	
    gprs_cfg.port = 5683;                                                     //other

	gprs_tran_init(&gprs_cfg);
    
    // 配置SSN模块
    ssn_cb_t ssn_cb;
    ssn_cb.tx_cb = ssn_send_cb;
    ssn_cb.rx_cb = ssn_receive_cb;
    ssn_cb.open_cb = ssn_open_cb;
    ssn_cb.close_cb = ssn_close_cb;
    ssn_config(&ssn_cb);   


	blu_init_cfg_t blu_cfg;

    //initial bluetooth inf
    blu_cfg.blu_send_cb = blu_send_cb;
    blu_cfg.blu_recv_cb = blu_receive_cb;
    blu_init(&blu_cfg);
}
