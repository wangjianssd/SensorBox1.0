/**
 * @brief       : cpp主要处理gprs模块的接收处理   
 *
 * @file        : gprs_rx.c 
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */
#include <gznet.h>
#include <gprs_tx.h>
#include <gprs_rx.h>
#include "gps.h"
//#include <sqqueue.h>
//#include <coap.h>
#include <stdio.h>
extern gprs_cmd_type_e gprs_cmd_type;
extern gprs_data_type_t gprs_data_recv_from_app;
extern gprs_send_cb_t gprs_tx_cb;
extern gprs_recv_cb_t gprs_recv_cb;
extern bool_t gprs_if_can_send;
extern bool_t gprs_if_connected;
static uint8_t gprs_cmd_recv_array[GPRS_RECV_CMD_SIZE_MAX]; //GPRS 接收缓冲区数组

#define GPRS_CMD_CGATT_MAX_NUM     6u //附着最大次数定义
#define GPRS_CMD_CIPSTART_MAX_NUM     3u //建立连接最大次数定义
#define GPRS_CMD_SEND_LEN_MAX_NUM     5u //发送数据长度最大次数
uint8_t gprs_cmd_send_cgatt_count = 0;//附着连接次数
uint8_t gprs_cmd_send_cipstart_count = 0;//建立连接次数
uint8_t gprs_cmd_send_len_count = 0;//建立连接次数
static uint8_t gprs_cmd_recv_pos = 0;

#define GPRS_CMD_ATE0_MAX_NUM   5u
uint8_t gprs_cmd_send_ate0_count = 0;

#ifdef USE_GPRS_M26

#define GPRS_CMD_QIOPEN_MAX_NUM     3u //建立连接最大次数定义
uint8_t gprs_cmd_send_qiopen_count = 0;//建立连接次数

#endif

extern fsm_t fsm_gprs;

/**
* @note 添加的缓冲队列
*/
sqqueue_ctrl_t auto_recv_sqq;

/**
* @brief 读取添加的串口缓冲队列中的数据
*/
static uint8_t auto_read(void *buffer, uint16_t len)
{
    uint8_t i = 0;
    uint8_t e;
    uint8_t *buf = (uint8_t *)buffer;
    uint8_t temp_len = auto_recv_sqq.get_len(&auto_recv_sqq);
    if (temp_len >= len)
    {
        for (i = 0; i < len; i++)
        {
            e = *((uint8_t *)auto_recv_sqq.del(&auto_recv_sqq));
            buf[i] = e;
        }
    }
    else
    {
        while ((temp_len != 0) && (i < len))
        {
            e = *((uint8_t *)auto_recv_sqq.del(&auto_recv_sqq));
            buf[i++] = e;
        }
    }

    return i;
}

/**
* @breif 清空接收buf
*
*/
void gprs_flush_recv_buf_from_serial(void)
{
    osel_memset(gprs_cmd_recv_array, 0x00, sizeof(gprs_cmd_recv_array));
    gprs_cmd_recv_pos = 0;
}

/**
* @breif 当发送了AT指令后对串口的解析处理
*
*/
static void gprs_cmd_at_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                   "OK") != NULL) 
    {
        //转入发送ATE0状态
//        TRAN(gprs_prepare_connect_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
      #ifdef USE_GPRS_M26
        event.param = (osel_param_t *)SEND_ATQ0;
	  #else
        event.param = (osel_param_t *)SEND_ATE0;
	  #endif
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_ATE0, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);        
    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}
/**
* @breif 当发送了ATE0指令后对串口的解析处理
*
*/
static void gprs_cmd_ate0_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    { 
        TRAN(gprs_prepare_connect_state);
        //转入发送CGATT状态
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *) SEND_CGATT;
        osel_post(NULL, &gprs_event_process, &event);
		
//		extern gprs_cmd_type_e gprs_cmd_type;
//		gprs_cmd_type = GPRS_CMD_CGATT;
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CIPRXGET, OSEL_EVENT_PRIO_LOW);//SEND_CIPMUX
    }
    else
    {
		gprs_cmd_send_ate0_count++;   //出现+CGATT：0加1,直到出现+CGATT：1清零		  
		if(gprs_cmd_send_ate0_count < GPRS_CMD_ATE0_MAX_NUM)
		{
			delay_ms(100);
			//转入发送CGATT状态
			TRAN(gprs_prepare_connect_state);
			event.sig = GPRS_STATE_TRANS_EVENT;
			event.param = (osel_param_t *)SEND_ATE0;
			osel_post(NULL, &gprs_event_process, &event);			  
//			  osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
		}
		else
		{
	        //转入掉电状态
	        //hal_uart_recv_disable(UART_3);
	        TRAN(gprs_power_off_state);
	        event.sig = GPRS_STATE_TRANS_EVENT;
	        event.param = (osel_param_t *)INIT_SIG;
	        osel_post(NULL, &gprs_event_process, &event);
	        
	//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
		}
    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}

/**
* @breif 当发送了AT+CIPMUX=0指令后对串口的解析处理
*
*/
static void gprs_cmd_cipmux_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_CIPRXGET;
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CIPRXGET, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);  
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}

/**
* @breif 当发送了AT+CIPRXGET=1指令后对串口的解析处理
*
*/
static void gprs_cmd_ciprxget_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);//SEND_CIPQRCLOSE
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);         
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}

/**
* @breif 当发送了AT+CIPQRCLOSE=1指令后对串口的解析处理
*
*/
static void gprs_cmd_cipqrclose_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}

/**
* @breif 当发送了CGATT指令后对串口的解析处理
*        对于附着的查询这里查询6次，如果失败断电源,再上电，再重新附着
*
*/
static void gprs_cmd_cgatt_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                 "+CGATT: 1") != NULL)
    {
        gprs_cmd_send_cgatt_count = 0;
        //转入发送SEND_CIPSTART建立连接状态
        TRAN(gprs_connect_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
#ifdef USE_GPRS_M26
		event.param = (osel_param_t *)SEND_QIOPEN;
#else
		event.param = (osel_param_t *)SEND_CIPSTART;
#endif
        osel_post(NULL, &gprs_event_process, &event);         
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);        
    }
//    else if((my_strstr((const char*)gprs_cmd_recv_array, 
//                        "0") != NULL)
//          ||(my_strstr((const char*)gprs_cmd_recv_array, 
//              "ERROR") != NULL ))
    else
    {
        gprs_cmd_send_cgatt_count++;   //出现+CGATT：0加1,直到出现+CGATT：1清零        
        if(gprs_cmd_send_cgatt_count < GPRS_CMD_CGATT_MAX_NUM)
        {
            //转入发送CGATT状态
            TRAN(gprs_prepare_connect_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);             
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
        }
        else                               
        {
            gprs_cmd_send_cgatt_count = 0;
            //转入掉电状态
            TRAN(gprs_power_off_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);             
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
        }
    }
//    else
//    {
//        gprs_cmd_send_cgatt_count = 0;
//        //转入掉电状态
//        TRAN(gprs_power_off_state);
//        event.sig = GPRS_STATE_TRANS_EVENT;
//        event.param = (osel_param_t *)INIT_SIG;
//        osel_post(NULL, &gprs_event_process, &event);              
//    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}

/**
* @breif 当发送了CIPSTART指令后对串口的解析处理
*        建立连接3次，若失败断开电源,再上电，再发送重新建立连接
*
*/
static void gprs_cmd_cipstart_recv_handler(void)
{
    osel_event_t event;
    if((my_strstr((const char*)gprs_cmd_recv_array, 
                    "CONNECT OK") != NULL )
       || (my_strstr((const char*)gprs_cmd_recv_array, 
                       "ALREADY CONNECT") != NULL)
       || (my_strstr((const char*)gprs_cmd_recv_array, 
                       "OK") != NULL))
    {
        gprs_cmd_send_cipstart_count = 0;     
        //转入发送数据长度状态
        TRAN(gprs_send_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
    else if(my_strstr((const char*)gprs_cmd_recv_array, 
                      "CONNECT FAIL") != NULL)   //OK就是说还没有连接上远程主机
    {
        gprs_cmd_send_cipstart_count++;           //出现FAIL加1，出现CONNECT OK清零       
        if(gprs_cmd_send_cipstart_count < GPRS_CMD_CIPSTART_MAX_NUM)
        {
            //转入发送SEND_CIPSTART建立连接状态
//            TRAN(gprs_connect_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);              
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
        }
        else                                         //连接3次失败返回无网络
        {
            gprs_cmd_send_cipstart_count = 0;
            //转入掉电状态
            TRAN(gprs_power_off_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);              
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
        }
    }
    else  
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}

#ifdef USE_GPRS_M26

static void gprs_cmd_qimux_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_CIPRXGET;
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CIPRXGET, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);  
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}

static void gprs_cmd_qindi_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);//SEND_CIPQRCLOSE
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);         
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}


static void gprs_cmd_qiclose_recv_handler(void)
{
    osel_event_t event;
    if (my_strstr((const char*)gprs_cmd_recv_array, 
                        "OK") != NULL)
    {
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
    }

    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();    
}

static void gprs_cmd_qiopen_recv_handler(void)
{
    osel_event_t event;
    if((my_strstr((const char*)gprs_cmd_recv_array, 
                    "CONNECT OK") != NULL )
       || (my_strstr((const char*)gprs_cmd_recv_array, 
                       "ALREADY CONNECT") != NULL))
//       || (my_strstr((const char*)gprs_cmd_recv_array, 
//                       "OK") != NULL))
    {
        gprs_cmd_send_qiopen_count = 0;     
        //转入发送数据长度状态
        TRAN(gprs_send_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_QISEND_LEN;//INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);    
        
//		extern gprs_cmd_type_e gprs_cmd_type;
//		gprs_cmd_type = GPRS_CMD_SEND_LEN;        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
//    else if(my_strstr((const char*)gprs_cmd_recv_array, 
//                      "CONNECT FAIL") != NULL)   //OK就是说还没有连接上远程主机
    else
    {
        gprs_cmd_send_qiopen_count++;           //出现FAIL加1，出现CONNECT OK清零       
        if(gprs_cmd_send_qiopen_count < GPRS_CMD_QIOPEN_MAX_NUM)
        {
            //转入发送SEND_CIPSTART建立连接状态
            TRAN(gprs_connect_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);              
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
        }
        else                                         //连接3次失败返回无网络
        {
            gprs_cmd_send_qiopen_count = 0;
            //转入掉电状态
            TRAN(gprs_power_off_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);              
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
        }
    }
//    else  
//    {
        //转入掉电状态
//        TRAN(gprs_power_off_state);
//        event.sig = GPRS_STATE_TRANS_EVENT;
//        event.param = (osel_param_t *)INIT_SIG;
//        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
//    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}



#endif
/**
* @breif 当发送了SEND_CIPSEND_LEN指令后对串口的解析处理
*
*/
static void gprs_cmd_send_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                   ">") != NULL)
    {
        gprs_cmd_send_len_count = 0;
        //转入发送数据状态
//        TRAN(gprs_send_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_DATA;
        osel_post(NULL, &gprs_event_process, &event);         
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_DATA, OSEL_EVENT_PRIO_LOW);
    }
    //发送AT+CIPSEND指令返回ERROR或者其他可能连接处于IPSHUT状态，查询连接状态
    else  
    {
        if (gprs_cmd_send_len_count++ <= GPRS_CMD_SEND_LEN_MAX_NUM)
        {
            //转入发送数据长度状态
//            TRAN(gprs_send_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);             
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);            
        }
        else
        {
            //转入掉电状态
            TRAN(gprs_power_off_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);               
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);             
        }
    }
    
    stop_gprs_no_data_timeout();
    gprs_flush_recv_buf_from_serial();
}

/**
* @breif 当发送了数据后对串口的解析处理
*
*/
static void gprs_cmd_data_recv_handler(void)
{
    osel_event_t event;
    gprs_send_mode_e data_mode_temp;
    data_mode_temp = gprs_data_recv_from_app.gprs_data_mode;

    //判断当前模式，如果是TCP_SINGLE或UDP_SINGLE，转入掉电状态
    if ((data_mode_temp == TCP_SINGLE)||(data_mode_temp == UDP_SINGLE))
    {
        //转入掉电状态
        TRAN(gprs_power_off_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);          
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);             
    } 
        
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                 "SEND OK\r\n") != NULL)
    {       
        //给APP层返回发送结果 
        (*gprs_tx_cb)(TRUE,gprs_data_recv_from_app.gprs_sn);
        gprs_if_connected = TRUE; 

#if 1
		extern osel_etimer_t gprs_test_stop_timer;
		osel_etimer_disarm(&gprs_test_stop_timer);
		osel_etimer_arm(&gprs_test_stop_timer,(30000/OSEL_TICK_PER_MS),0);
#endif

		
//        extern void box_send_list_init(uint16_t size);
//         box_send_list_init(15);  //for test
    }
    else
    {
        //给APP层返回发送结果 
        (*gprs_tx_cb)(FALSE,gprs_data_recv_from_app.gprs_sn);
        gprs_if_connected = FALSE; 
    }
    
//    if (my_strstr((const char*)gprs_cmd_recv_array, 
//                    "+CIPRXGET:1") != NULL)
//    {
//        //转入接收服务器数据的状态
//        TRAN(gprs_recv_state);
//        event.sig = GPRS_STATE_TRANS_EVENT;
//        event.param = (osel_param_t *)INIT_SIG;
//        osel_post(NULL, &gprs_event_process, &event);          
////        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
//    }
    
    stop_gprs_no_data_timeout();
    //初始化各变量
    gprs_flush_recv_buf_from_serial();
    gprs_cmd_type = GPRS_CMD_NULL;
    gprs_if_can_send = TRUE;
}

/**
* @breif 当接收了服务器的数据时对串口的解析处理
*
*/
static void gprs_cmd_server_data_recv_handler(void)
{
    uint8_t server_data[128] = {0x00};
    osel_memcpy(server_data,&gprs_cmd_recv_array[21],gprs_data_recv_from_app.gprs_data_len);
    
    gprs_receive_t gprs_recv_param;
    gprs_recv_param.gprs_data = server_data;
    gprs_recv_param.sn = gprs_data_recv_from_app.gprs_sn;
    gprs_recv_param.len = (gprs_data_recv_from_app.gprs_data_len);
   
    (*gprs_recv_cb)(gprs_recv_param);//给APP层回复响应

    stop_gprs_no_data_timeout(); 
    //初始化各变量
    gprs_flush_recv_buf_from_serial();
    gprs_flush_recv_buf_from_app();
    gprs_flush_recv_buf_from_app();
    gprs_cmd_type = GPRS_CMD_NULL;
    gprs_if_can_send = TRUE;    
}


#ifdef USE_GPRS_M26

extern fp64_t get_rmc_value(uint8_t *p_data, uint8_t type);
extern gps_simple_info_t gps_simple_info;

static void gprs_cmd_qcellloc_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                 "+QCELLLOC:") != NULL)
    {       
		gps_simple_info.latitude = get_rmc_value(&gprs_cmd_recv_array[10], GPS_DATA_TYPE_LAT);
		gps_simple_info.longitude = get_rmc_value(&gprs_cmd_recv_array[16], GPS_DATA_TYPE_LON);      
    }
    
    stop_gprs_no_data_timeout();
    //初始化各变量
    gprs_flush_recv_buf_from_serial();
    gprs_cmd_type = GPRS_CMD_NULL;
    gprs_if_can_send = TRUE;

}


#endif


/**
* @breif 接收串口处理
*
*/
void gprs_uart_event_handler(void)
{
    uint8_t read_data = 0;
	osel_event_t event;
    //uint16_t i = 0;
    
    while (auto_read(&read_data, sizeof(uint8_t)))
    {
        gprs_cmd_recv_array[gprs_cmd_recv_pos++] = read_data;
    }
    
    if(gprs_cmd_recv_pos == 1)
    {
      gprs_flush_recv_buf_from_serial();
      return;
    }
    //解析接收到的数据
    if(my_strstr((const char*)gprs_cmd_recv_array, 
                    "\r\nCHARGE-ONLY MODE\r\n") != NULL)
    {
         gprs_flush_recv_buf_from_serial();
		 return;
    }
    else if(my_strstr((const char*)gprs_cmd_recv_array, 
                    "\r\nCall Ready\r\n") != NULL)
    {
		event.sig = GPRS_STATE_TRANS_EVENT;
		event.param = (osel_param_t *)SEND_CGATT;
		osel_post(NULL, &gprs_event_process, &event);
        gprs_flush_recv_buf_from_serial();
		return;
    }
    else if(my_strstr((const char*)gprs_cmd_recv_array, 
                         "NORMAL POWER DOWN\r\n") != NULL)
    {
 		osel_event_t event;
 		TRAN(gprs_power_off_state);
 		event.sig = GPRS_STATE_TRANS_EVENT;
 		event.param = (osel_param_t *)INIT_SIG;
 		osel_post(NULL, &gprs_event_process, &event);
 
 		stop_gprs_no_data_timeout();
 		gprs_flush_recv_buf_from_serial();

		 return;
    }
    else if(my_strstr((const char*)gprs_cmd_recv_array, 
                        "\r\nFrom CHARGE-ONLY MODE to NORMAL MODE\r\n") != NULL) 
    {
        gprs_flush_recv_buf_from_serial();  
		return;
    }
    //没有插入卡，返回GPRS错误
    else if(my_strstr((const char*)gprs_cmd_recv_array,
                        "NOT INSERTED") != NULL) 
    {
        gprs_flush_recv_buf_from_serial();
		return;
    } 
    else if(my_strstr((const char*)gprs_cmd_recv_array, 
                         "+PDP: DEACT\r\n") != NULL)
    {
         gprs_flush_recv_buf_from_serial();
		 return;
    }
    else
    {
         switch (gprs_cmd_type)
         {
         case GPRS_CMD_AT:
             gprs_cmd_at_recv_handler();
             break;
         
         case GPRS_CMD_ATE0:
             gprs_cmd_ate0_recv_handler();
             break;

         case GPRS_CMD_CIPMUX:
             gprs_cmd_cipmux_recv_handler();
             break;
             
         case GPRS_CMD_CIPRXGET:
             gprs_cmd_ciprxget_recv_handler();
             break;

         case GPRS_CMD_CIPQRCLOSE:
             gprs_cmd_cipqrclose_recv_handler();
             break;
             
         case GPRS_CMD_CGATT:
             gprs_cmd_cgatt_recv_handler();
             break;
         
         case GPRS_CMD_CIPSTART:
             gprs_cmd_cipstart_recv_handler();
             break;           

       #ifdef USE_GPRS_M26
         
         case GPRS_CMD_QIMUX:
            gprs_cmd_qimux_recv_handler();
            break;
            
         case GPRS_CMD_QINDI:
            gprs_cmd_qindi_recv_handler();
            break;
         
         case GPRS_CMD_QICLOSE:
            gprs_cmd_qiclose_recv_handler();
            break;
            
         case GPRS_CMD_QIOPEN:
            gprs_cmd_qiopen_recv_handler();
            break;

         case GPRS_CMD_QCELLLOC:
			gprs_cmd_qcellloc_recv_handler();
			break;
		   
       #endif

	   case GPRS_CMD_SEND_LEN:
		   gprs_cmd_send_recv_handler();
		   break;
	   
	   case GPRS_SEND_DATA:
		   gprs_cmd_data_recv_handler();
		   break;
	   
	   case GPRS_RECV_DATA:
		   gprs_cmd_server_data_recv_handler();
		   break;  

	   
         default:
             break; 
         }
    }     
}



