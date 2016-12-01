/**
 * @brief       : .cpp主要处理blu模块的发送处理    
 *
 * @file        : blu_tx.c 
 * @author      : xhy
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      xhy    文件初始版本
 */
#include <gznet.h>
#include <blu_tx.h>
#include <hal_elec_lock.h>

#include <app_frame.h>
#include <hal_rfid.h>
#include <app_task.h>
#include <gps.h>
#include <app_gps.h>
#include "app_send.h"

#include "app_acc_sensor.h"
//#include <app_heartbeat.h>
#include "app_humiture.h"
#include "nfc_reader.h"



/*************宏定义******************/ 
#define BLU_SERIAL_LEN_MAX          100u
#define BLU_SERIAL_LEN_MIN          4u
#define BLU_SEND_CMD_MAX_CNT        3u
#define BLU_NO_DATA_MAX             40000u//60s
#define BLU_PREPARE_CONNECT_MAX     3000u//6s
#define BLU_CONNECT_MAX             80u//10s
#define BLU_SEND_DATA_MAX           5000u//6s

#define  BLU_AT                     "AT\r" 
#define  BLU_AT_RESET				"AT+RESET\r"
#define  BLU_AT_VERSION             "AT+VERSION\r"
#define  BLU_AT_LADDR               "AT+LADDR\r"
#define  BLU_AT_NAME                "AT+NAME\r"
#define  BLU_AT_PIN                 "AT+PIN\r"
#define  BLU_AT_BAUD                "AT+BAUD\r"
#define  BLU_AT_BAUD_115200         "AT+BAUD8\r\n"
#define  BLU_AT_BAUD_9600           "AT+BAUD4\r\n"

#define  BLU_AT_STOP                "AT+STOP\r"
#define  BLU_AT_PARI                "AT+PARI\r"
#define  BLU_AT_DEFAULT             "AT+DEFAULT\r"
#define  BLU_AT_PWRM                "AT+PWRM\r"
#define  BLU_AT_SLEEP               "AT+SLEEP\r"
#define  BLU_AT_ROLE                "AT+ROLE\r"
#define  BLU_AT_INQ                 "AT+INQ\r"
#define  BLU_AT_SHOW                "AT+SHOW\r"
#define  BLU_AT_CONN                "AT+CONN\r"
#define  BLU_AT_POWE                "AT+POWE\r"
#define  BLU_AT_HELP                "AT+HELP\r"

#define  BLU_CR                     "\r"

#define  BLU_UART  HAL_UART_1


//extern sqqueue_ctrl_t auto_recv_sqq;
/*************全局变量******************/
blu_init_cfg_t blu_cfg;
blu_data_type_t blu_data_recv_from_app;
bool_t blu_cur_power_flag = FALSE;
blu_cmd_type_e blu_cmd_type = BLU_CMD_NULL;
//bool_t blu_if_can_send = TRUE;
bool_t blu_if_connected = FALSE;
//bool_t blu_nodatatimer_setflg = FALSE;
bool_t blu_alarm_send_success = FALSE;

/*************私有变量******************/
blu_send_cb_t blu_tx_cb;
blu_recv_cb_t blu_recv_cb;

blu_fsm_t blu_fsm = {0};

static uint8_t blu_recv_array[BLU_DATA_MAXLEN];
static uint8_t blu_recv_pos = 0;

static osel_etimer_t blu_timeout_timer;
static osel_etimer_t blu_no_ack_timer;
static osel_etimer_t blu_no_lock_timer;
extern osel_etimer_t buzzer_cycle_timer;
extern bool_t if_get_password;
uint16_t BluDataSn = 0;

#define BLU_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t blu_event_store[BLU_EVENT_MAX];
static osel_task_t *blu_task_tcb = NULL;

PROCESS(blu_event_process, "blu_event_process");

/*************声明函数*******************/

/**
* @note 添加的缓冲队列
*/
sqqueue_ctrl_t blu_auto_recv_sqq;

/**
* @brief 读取添加的串口缓冲队列中的数据
*/
static uint8_t blu_auto_read(void *buffer, uint16_t len)
{
    uint8_t i = 0;
    uint8_t e;
    uint8_t *buf = (uint8_t *)buffer;
    uint8_t temp_len = blu_auto_recv_sqq.get_len(&blu_auto_recv_sqq);
    if (temp_len >= len)
    {
        for (i = 0; i < len; i++)
        {
            e = *((uint8_t *)blu_auto_recv_sqq.del(&blu_auto_recv_sqq));
            buf[i] = e;
        }
    }
    else
    {
        while ((temp_len != 0) && (i < len))
        {
            e = *((uint8_t *)blu_auto_recv_sqq.del(&blu_auto_recv_sqq));
            buf[i++] = e;
        }
    }

    return i;
}


static void blu_power_on_state(blu_fsm_t *me, blu_sig_event_e sig);

void blu_power_off_state(blu_fsm_t *me, blu_sig_event_e sig);


/**
 *
 * @brief 接收缓冲数组的初始化
 * @param 无
 * @return 无
 * 
 */
void blu_flush_recv_buf_from_app(void)
{
    blu_data_recv_from_app.blu_data_len = 0;
    blu_data_recv_from_app.blu_sn = 0;

    osel_memset(blu_data_recv_from_app.blu_data,0x00, BLU_DATA_MAXLEN * sizeof(uint8_t));
}


/**
 *@brief 向blu发送AT指令和数据
 */
static void blu_flush_recv_buf_from_serial(void)
{
	osel_memset(blu_recv_array, 0x00, sizeof(blu_recv_array));
	blu_recv_pos = 0;

}



void stop_blu_no_lock_timeout(void)
{
    osel_etimer_disarm(&blu_no_lock_timer);
}

/**
*@brief blu ��ȡ��Ȩ֮��Ĳ�����ʱ����ʱ
*/
extern uint8_t box_operation_status_flag;
static void blu_no_lock_timeout_timer_cb(void)
{
	box_frame_t box_frame;

	elec_lock_init();
    buzzer_cycle_timer_start(1000);
    if_get_password = FALSE;//此处未开锁，所以不能得到授权

	box_frame.frame_type = BOX_SENSOR_FRAME;
	box_frame.box_type_frame_u.sensor_info.type = SENSOR_UNLOCK;

	box_blu_send_request(&box_frame); //blutooth

}
/**
*@brief blu ��ȡ��Ȩ֮��Ĳ�����ʱ��
*/
static void blu_no_lock_timeout_set(uint32_t ticks)
{
    stop_blu_no_lock_timeout();

	osel_etimer_arm(&blu_no_lock_timer, (ticks/OSEL_TICK_PER_MS), 0); 

}

void stop_blu_no_ack_timeout(void)
{
    osel_etimer_disarm(&blu_no_ack_timer);
}

void blu_no_ack_timer_cb(void)
{
    stop_blu_no_ack_timeout();

	extern uint16_t blu_alarm_cn;
    blu_alarm_cn = 0;
   
	if(blu_alarm_send_success == TRUE)
	{
		blu_alarm_send_success = FALSE;
        blu_alarm_cn = 1; 
	}
    
	(*blu_tx_cb)(TRUE,blu_data_recv_from_app.blu_sn);
    
    blu_flush_recv_buf_from_serial();
	blu_flush_recv_buf_from_app();
	blu_cmd_type = BLU_CMD_NULL;
}



/**
*@brief 给串口发指令或数据时设置的定时器
*/
void blu_no_ack_timeout_set(uint32_t ticks)
{
    stop_blu_no_ack_timeout();
    osel_etimer_arm(&blu_no_ack_timer, ticks/OSEL_TICK_PER_MS, 0);
}

/**
*@brief blu电源操作中定时器定时时间到的回调处理
*/
static void blu_timeout_timer_cb(void)
{
    osel_event_t event;
	event.sig = BLU_STATE_TRANS_EVENT;
	event.param = (osel_param_t *)BLU_TIMEOUT_SIG;
	osel_post(NULL, &blu_event_process, &event); 
//    osel_post(BLU_STATE_TRANS_EVENT, (void *)BLU_TIMEOUT_SIG, OSEL_EVENT_PRIO_LOW);
}
/**
*@brief blu电源中设定定时器
*/
static void blu_timeout_set(uint32_t ticks)
{
    osel_etimer_arm(&blu_timeout_timer, ticks/OSEL_TICK_PER_MS, 0);
}

/**
 *@brief 查询当前blu模块是否可以发送
 */
bool_t blu_tran_can_send(void)
{
    return 1;//blu_if_can_send;
}


static void blu_send_cmd(uint8_t *cmd, uint8_t len)
{ 
    uint8_t i;
    uint8_t mod;

    mod = 0x13;
    //mod = 12;
    
    for (i = 0; i < (len / mod); i++)
    {
        serial_write(BLU_UART, &cmd[mod * i], mod);
        delay_ms(30);
    }

    serial_write(BLU_UART, &cmd[mod * i], len % mod);
}


/**
 *@brief blu发送数据
 */

static void blu_send_data(void)
{
    blu_send_cmd(blu_data_recv_from_app.blu_data, 
                  blu_data_recv_from_app.blu_data_len);
}



/**
 *@brief blu连接状态处理
 */
void blu_connect_state(blu_fsm_t *me,blu_sig_event_e sig)
{
    osel_event_t event;
	
    switch (sig)
    {
    case BLU_INIT_SIG:
        blu_timeout_set(BLU_CONNECT_MAX);
        break;
        
    case BLU_TIMEOUT_SIG:

        event.sig = BLU_STATE_TRANS_EVENT;
		event.param = (osel_param_t *)BLU_DATA_SEND_SIG;
        osel_post(NULL, &blu_event_process, &event);        
        break;         

    case BLU_DATA_SEND_SIG:
        blu_cmd_type = BLU_SEND_DATA;
        blu_send_data();
        //blu_no_data_timeout_set(BLU_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;

	case BLU_DATA_RECEIVE_SIG:
		blu_cmd_type = BLU_RECV_DATA;
		
    default:
        break;               
    }    
}


/**
 *@brief blu连接准备状态处理
 */
void blu_prepare_connect_state(blu_fsm_t *me, blu_sig_event_e sig)
{
    osel_event_t event;

    switch (sig)
    {
     case BLU_INIT_SIG:
     	blu_timeout_set(BLU_PREPARE_CONNECT_MAX);
     	break;
     	
     case BLU_TIMEOUT_SIG:
		
 		event.sig = BLU_STATE_TRANS_EVENT;
 		event.param = (osel_param_t *)BLU_SEND_CMD_AT_SIG;
 		osel_post(NULL, &blu_event_process, &event);		 

     	break;		  
     	
     case BLU_SEND_CMD_AT_SIG:

     	blu_cmd_type = BLU_CMD_AT;
     	blu_send_cmd(BLU_AT, sizeof(BLU_AT) - 1);
     	//blu_no_data_timeout_set(BLU_NO_DATA_MAX);
     	break;

    case BLU_SEND_CMD_AT_SLEEP_SIG:
        blu_cmd_type = BLU_CMD_AT_SLEEP;
        blu_send_cmd(BLU_AT_SLEEP, sizeof(BLU_AT_SLEEP) - 1);
        break;
        
    case BLU_SEND_CMD_AT_SHOW_SIG:
        blu_cmd_type = BLU_CMD_AT_SHOW;
		//blu_no_data_timeout_set(BLU_NO_DATA_MAX/OSEL_TICK_PER_MS);
        blu_send_cmd(BLU_AT_SHOW, sizeof(BLU_AT_SHOW) - 1);
        break; 

    case BLU_SEND_CMD_AT_CONN_SIG:
        blu_cmd_type = BLU_CMD_AT_CONN;
		//blu_no_data_timeout_set(BLU_NO_DATA_MAX/OSEL_TICK_PER_MS);
        blu_send_cmd(BLU_AT_CONN, sizeof(BLU_AT_CONN) - 1);
        break; 
	
    default:
        break;              
    }
}

/**
 *@brief blu电源打开或关闭之后的处理
 */
void blu_power_operate_proc(blu_power_operate_e flag)
{
    osel_event_t event;
    if (flag == BLU_POWER_ON)
    {//转到连接准备态
        blu_cur_power_flag = TRUE;
        BLU_TRAN(blu_prepare_connect_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        //event.sig = BLU_SEND_EVENT;
        event.param = (osel_param_t *)BLU_SEND_CMD_AT_SIG;
        osel_post(NULL, &blu_event_process, &event);   
    }
    else if (flag == BLU_POWER_OFF)
    {
        blu_cur_power_flag = FALSE;
		blu_cmd_type = BLU_CMD_NULL;
		BLU_TRAN(blu_power_on_state);
		event.sig = BLU_STATE_TRANS_EVENT;
		event.param = (osel_param_t *)BLU_INIT_SIG;
		osel_post(NULL, &blu_event_process, &event);   
    }
}

/**
*@brief blu上电状态处理
*/
void blu_power_on_state(blu_fsm_t *me, blu_sig_event_e sig)
{
    
    switch (sig)
    {
    case BLU_INIT_SIG:
        blu_timeout_set(BLU_TIMEOUT_TIME_PER);

//        POWER_IOA_ON();
//        POWER_IOB_OFF();
        break;

    case BLU_TIMEOUT_SIG:      
		blu_power_operate_proc(BLU_POWER_ON);
        break;

    default:
        break;
    }
}
/**
*@brief blu掉电状态处理
*/
void blu_power_off_state(blu_fsm_t *me, blu_sig_event_e sig)
{
   
    switch (sig)
    {
    case BLU_INIT_SIG:
        blu_timeout_set(BLU_TIMEOUT_TIME_PER);
        //POWER_IOB_ON();
        break;

    case BLU_TIMEOUT_SIG:
		blu_power_operate_proc(BLU_POWER_OFF);
        break;
    }
}

/**
 *
 * @brief blu数据发送
 * @param 无
 * @return 无
 * 
 */
static void blu_send_event_handler(void)
{
    //blu_if_can_send = FALSE;
    osel_event_t event;
    //如果blu未上电，转入上电状态
    if (!blu_cur_power_flag)
    {
        BLU_TRAN(blu_power_on_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t)BLU_INIT_SIG;
        osel_post(NULL, &blu_event_process, &event);        
//        osel_post(BLU_STATE_TRANS_EVENT, (void *)BLU_INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
    else
    {//转入建立连接态
        if (blu_if_connected)//如果当前网络连接正常，则转入发数据态
        {
            BLU_TRAN(blu_connect_state);
            event.sig = BLU_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)BLU_DATA_SEND_SIG;//BLU_INIT_SIG;
            osel_post(NULL, &blu_event_process, &event);            
//            osel_post(BLU_STATE_TRANS_EVENT, (void *)BLU_INIT_SIG, OSEL_EVENT_PRIO_LOW);             
        }
        else        //如果当前网络连接不正常，则转入连接态
        {
            BLU_TRAN(blu_prepare_connect_state);
            event.sig = BLU_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)BLU_INIT_SIG;
            osel_post(NULL, &blu_event_process, &event);            
//            osel_post(BLU_STATE_TRANS_EVENT, (void *)BLU_INIT_SIG, OSEL_EVENT_PRIO_LOW);               
        }       
    }
}


static void blu_cmd_at_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)blu_recv_array, 
                   "OK") != NULL) 
    {
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_SEND_CMD_AT_ROLE_SIG;
        osel_post(NULL, &blu_event_process, &event);        
    }
    else
    {
        //转入掉电状态
        BLU_TRAN(blu_power_off_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_INIT_SIG;
        osel_post(NULL, &blu_event_process, &event);           
    }
    
    //stop_blu_no_data_timeout();
    blu_flush_recv_buf_from_serial();
}


static void blu_cmd_at_role_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)blu_recv_array, 
                   "OK") != NULL) 
    {
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_SEND_CMD_AT_INQ_SIG;
        osel_post(NULL, &blu_event_process, &event);        
    }
    else
    {
        //转入掉电状态
        BLU_TRAN(blu_power_off_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_INIT_SIG;
        osel_post(NULL, &blu_event_process, &event);           
    }
    
    //stop_blu_no_data_timeout();
    blu_flush_recv_buf_from_serial();
}


static void blu_cmd_at_inq_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)blu_recv_array, 
                   "OK") != NULL) 
    {
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_SEND_CMD_AT_CONN_SIG;
        osel_post(NULL, &blu_event_process, &event);        
    }
    else
    {
        //转入掉电状态
        BLU_TRAN(blu_power_off_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_INIT_SIG;
        osel_post(NULL, &blu_event_process, &event);           
    }
    
    //stop_blu_no_data_timeout();
    blu_flush_recv_buf_from_serial();
}

static void blu_cmd_at_conn_recv_handler(void)
{
    osel_event_t event;
    if(my_strstr((const char*)blu_recv_array, 
                   "+CONNECTED") != NULL) 
    {
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_DATA_SEND_SIG;
        osel_post(NULL, &blu_event_process, &event);        
    }
    else
    {
        //转入掉电状态
        BLU_TRAN(blu_power_off_state);
        event.sig = BLU_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)BLU_INIT_SIG;
        osel_post(NULL, &blu_event_process, &event);           
    }
    
    //stop_blu_no_data_timeout();
    blu_flush_recv_buf_from_serial();
}

static void blu_cmd_data_send_handler(void)
{
	stop_blu_no_ack_timeout();

    //if(my_strstr((const char*)blu_recv_array, 
    //             "SEND OK\r\n") != NULL)
    {       
        //给APP层返回发送结果 

		extern uint16_t blu_alarm_cn;
        blu_alarm_cn = 1;
        
        blu_if_connected = TRUE; 
		
		blu_alarm_send_success = TRUE;
        
        (*blu_tx_cb)(TRUE,blu_data_recv_from_app.blu_sn);
    }

    //初始化各变量
    blu_flush_recv_buf_from_serial();
	blu_flush_recv_buf_from_app();
    blu_cmd_type = BLU_CMD_NULL;
	
    //blu_if_can_send = TRUE;
}

typedef unsigned short	USHORT;
typedef unsigned char	UCHAR;

static USHORT checksum (USHORT *buffer,int size)
{
    unsigned long cksum=0;///*32λ������������ͱ���Ϊ0*/
    uint16_t tempsum = 0;
    uint8_t *tempbuf = (uint8_t *)buffer;
    
    while (size>1)
    {
        //cksum +=*buffer++;
        //cksum += ((((*buffer) >> 8)&0x00ff)|(((*buffer) << 8)&0xff00));
        //buffer ++;
        
        tempsum = (uint16_t)(*tempbuf << 8)&0xff00;
        tempbuf++;        
        tempsum |= (uint16_t)(*tempbuf)&0x00ff;
        tempbuf++;
        
        cksum += tempsum; 
        
		size -=sizeof(USHORT);
    }

    if (size) ///*����ʣ���������ֶΣ���Щ�ֶν�С��16λ*/
    {
		//cksum += *(UCHAR *) buffer;
        cksum += *tempbuf;
    }

    ///*��32λת��Ϊ16λ,��16λ���16λ���*/

    while(cksum>>16)
    {      
    	cksum = (cksum>>16) + (cksum & 0xffff);
    }
	
    return (USHORT) (~cksum);
}


static void blu_send_frame(void *data_p, uint8_t len,  uint16_t sn)
{
    uint8_t* blu_data_tran_temp = (uint8_t *)data_p;

	blu_data_recv_from_app.blu_data_len = 2 + 2 + 1 + 1 + 2 + len + 2;
	blu_data_recv_from_app.blu_data[0] = BOX_BLU_CMD_COM_HEAD1;
	blu_data_recv_from_app.blu_data[1] = BOX_BLU_CMD_COM_HEAD2;

	blu_data_recv_from_app.blu_data[2] = 0;
	blu_data_recv_from_app.blu_data[3] = blu_data_recv_from_app.blu_data_len;

	blu_data_recv_from_app.blu_data[4] = BOX_BLU_CMD_COM_VER;

	blu_data_recv_from_app.blu_data[5] = blu_data_tran_temp[0];

	blu_data_recv_from_app.blu_data[6] = (uint8_t)(sn>>8);
	blu_data_recv_from_app.blu_data[7] = (uint8_t)sn;

	
	memcpy((void*)&blu_data_recv_from_app.blu_data[8],(uint8_t*)&blu_data_tran_temp[1],len);

	USHORT temp_blu_crc = 0;
    temp_blu_crc = checksum((USHORT *)blu_data_recv_from_app.blu_data,(blu_data_recv_from_app.blu_data_len -2));

	blu_data_recv_from_app.blu_data[blu_data_recv_from_app.blu_data_len - 2] = (uint8_t)(temp_blu_crc>>8);//CRC0
	blu_data_recv_from_app.blu_data[blu_data_recv_from_app.blu_data_len - 1] = (uint8_t)temp_blu_crc;//CRC1
}


void blu_tran_send(void *data_p, uint8_t len,  uint16_t sn)
{
    osel_event_t event;

	blu_send_frame(data_p, len, sn);

	blu_cmd_type = BLU_SEND_DATA;
    blu_send_data();

}


extern double gps_dm_to_d(double dm);
extern uint8_t box_uid[8];
extern gps_simple_info_t box_location;
extern sensor_info_t box_sensor;

static void blu_cmd_data_recv_handler(void)
{
	uint8_t temp_blu_data[64];
	uint16_t temp_blu_data_len;
	uint16_t temp_blu_data_sn;
	USHORT temp_blu_crc = 0;
	box_frame_t box_frame;
    uint16_t scan_delay;
    
	//if (blu_recv_array[0] == '2')
	//{
      //wangjian
      //  goto TEST_TAG_READER;
    //}
    
	if((blu_recv_array[0] != BOX_BLU_CMD_COM_HEAD1)||(blu_recv_array[1] != BOX_BLU_CMD_COM_HEAD2))
	{
//		OSEL_EXIT_CRITICAL(status);
        blu_flush_recv_buf_from_serial();
		return;
	}
	temp_blu_data_len = ((((uint16_t)blu_recv_array[2]) << 8)| (blu_recv_array[3]));

	if(blu_recv_pos <  ((uint8_t)temp_blu_data_len))
	{
		return;
	}

//    osel_int_status_t status = 0;
//    OSEL_ENTER_CRITICAL(status);    

	stop_blu_no_ack_timeout();
	
	temp_blu_crc = checksum((USHORT *)blu_recv_array,temp_blu_data_len - 2);//(blu_recv_pos - 2));

	if((blu_recv_array[temp_blu_data_len - 2] != (uint8_t)(temp_blu_crc >> 8))
		||(blu_recv_array[temp_blu_data_len - 1] != (uint8_t)temp_blu_crc))
    {
        blu_flush_recv_buf_from_serial();
//		OSEL_EXIT_CRITICAL(status);
		return;	
    }

	if(blu_recv_array[5] == BOX_BLU_CMD_RDBOXID)//0x01)
	{
		temp_blu_data_len = 8;
		
		memcpy((void*)&temp_blu_data[1],box_uid,8);

	}
	else if(blu_recv_array[5] == BOX_BLU_CMD_OPLOCK)//0x02)
	{
		temp_blu_data_len = 2;
		temp_blu_data[1] = 0;
		temp_blu_data[2] = 0;

		if(memcmp(&blu_recv_array[8],box_uid,8) != 0)
			temp_blu_data[2] = 1;
		else
		{
//    TEST_EXTERN_LOCK:
            if_get_password = TRUE;//授权开锁标志位
			((uint8_t*)&box_sensor.user_id)[3] = blu_recv_array[8+8];
			((uint8_t*)&box_sensor.user_id)[2] = blu_recv_array[8+8+1];
			((uint8_t*)&box_sensor.user_id)[1] = blu_recv_array[8+8+2];
			((uint8_t*)&box_sensor.user_id)[0] = blu_recv_array[8+8+3];

			((uint8_t*)&box_sensor.timestamp)[3] = blu_recv_array[8+8+4];
			((uint8_t*)&box_sensor.timestamp)[2] = blu_recv_array[8+8+4+1];
			((uint8_t*)&box_sensor.timestamp)[1] = blu_recv_array[8+8+4+2];
			((uint8_t*)&box_sensor.timestamp)[0] = blu_recv_array[8+8+4+3];

			((uint8_t*)&box_location.longitude)[3] = blu_recv_array[8+8+4+4];
			((uint8_t*)&box_location.longitude)[2] = blu_recv_array[8+8+4+4+1];
			((uint8_t*)&box_location.longitude)[1] = blu_recv_array[8+8+4+4+2];
			((uint8_t*)&box_location.longitude)[0] = blu_recv_array[8+8+4+4+3];

			((uint8_t*)&box_location.latitude)[3] = blu_recv_array[8+8+4+4+4];
			((uint8_t*)&box_location.latitude)[2] = blu_recv_array[8+8+4+4+4+1];
			((uint8_t*)&box_location.latitude)[1] = blu_recv_array[8+8+4+4+4+2];
			((uint8_t*)&box_location.latitude)[0] = blu_recv_array[8+8+4+4+4+3];
			
           buzzer_cycle_timer_start(200);
           
#ifdef USE_EXTERN_LOCK   
            if (elec_lock_State == lock_Open)
            {
              blu_flush_recv_buf_from_serial();
              return;
            }
            
            elec_lock_open();
            elec_lock_State_updata = 1;
            //extern_lock_timeout_set(EXTERN_LOCK_AUTO_LOCK_DELAY); // 10s timeout lock #endif
#else

            blu_no_lock_timeout_set(10000); //10s timeout			

#endif
 
		}
	}
	else if(blu_recv_array[5] == BOX_BLU_CMD_RUNSENSOR)//0x03)
	{
		temp_blu_data_len = 2;
		temp_blu_data[1] = 0;
		temp_blu_data[2] = 0;

		if(memcmp(&blu_recv_array[8],box_uid,8) != 0)
			temp_blu_data[2] = 1;
		else
		{
			box_operation_status_flag = TRUE;


			((uint8_t*)&box_sensor.user_id)[3] = blu_recv_array[8+8];
			((uint8_t*)&box_sensor.user_id)[2] = blu_recv_array[8+8+1];
			((uint8_t*)&box_sensor.user_id)[1] = blu_recv_array[8+8+2];
			((uint8_t*)&box_sensor.user_id)[0] = blu_recv_array[8+8+3];

			((uint8_t*)&box_sensor.timestamp)[3] = blu_recv_array[8+8+4];
			((uint8_t*)&box_sensor.timestamp)[2] = blu_recv_array[8+8+4+1];
			((uint8_t*)&box_sensor.timestamp)[1] = blu_recv_array[8+8+4+2];
			((uint8_t*)&box_sensor.timestamp)[0] = blu_recv_array[8+8+4+3];

			((uint8_t*)&box_location.longitude)[3] = blu_recv_array[8+8+4+4];
			((uint8_t*)&box_location.longitude)[2] = blu_recv_array[8+8+4+4+1];
			((uint8_t*)&box_location.longitude)[1] = blu_recv_array[8+8+4+4+2];
			((uint8_t*)&box_location.longitude)[0] = blu_recv_array[8+8+4+4+3];

			((uint8_t*)&box_location.latitude)[3] = blu_recv_array[8+8+4+4+4];
			((uint8_t*)&box_location.latitude)[2] = blu_recv_array[8+8+4+4+4+1];
			((uint8_t*)&box_location.latitude)[1] = blu_recv_array[8+8+4+4+4+2];
			((uint8_t*)&box_location.latitude)[0] = blu_recv_array[8+8+4+4+4+3];

			box_enable_acc_alg();
			box_humiture_init();   	            
		}
	}
	else if(blu_recv_array[5] == BOX_BLU_CMD_LOSERIGHT)//0x04)
	{
		temp_blu_data_len = 2;
		temp_blu_data[1] = 0;
		temp_blu_data[2] = 0;

		if(memcmp(&blu_recv_array[8],box_uid,8) != 0)
			temp_blu_data[2] = 1;
		else
		{
			//if (box_operation_status_flag)
			{
				//device_info = hal_board_info_get();
				//if (user_id == device_info.box_app_id)
				{
					((uint8_t*)&box_sensor.user_id)[3] = blu_recv_array[8+8];
					((uint8_t*)&box_sensor.user_id)[2] = blu_recv_array[8+8+1];
					((uint8_t*)&box_sensor.user_id)[1] = blu_recv_array[8+8+2];
					((uint8_t*)&box_sensor.user_id)[0] = blu_recv_array[8+8+3];
					
					((uint8_t*)&box_sensor.timestamp)[3] = blu_recv_array[8+8+4];
					((uint8_t*)&box_sensor.timestamp)[2] = blu_recv_array[8+8+4+1];
					((uint8_t*)&box_sensor.timestamp)[1] = blu_recv_array[8+8+4+2];
					((uint8_t*)&box_sensor.timestamp)[0] = blu_recv_array[8+8+4+3];
					
					((uint8_t*)&box_location.longitude)[3] = blu_recv_array[8+8+4+4];
					((uint8_t*)&box_location.longitude)[2] = blu_recv_array[8+8+4+4+1];
					((uint8_t*)&box_location.longitude)[1] = blu_recv_array[8+8+4+4+2];
					((uint8_t*)&box_location.longitude)[0] = blu_recv_array[8+8+4+4+3];
					
					((uint8_t*)&box_location.latitude)[3] = blu_recv_array[8+8+4+4+4];
					((uint8_t*)&box_location.latitude)[2] = blu_recv_array[8+8+4+4+4+1];
					((uint8_t*)&box_location.latitude)[1] = blu_recv_array[8+8+4+4+4+2];
					((uint8_t*)&box_location.latitude)[0] = blu_recv_array[8+8+4+4+4+3];

#ifndef USE_EXTERN_LOCK   
					elec_lock_init();
#endif
					humiture_stop_timer();
					box_disable_acc_alg();
					
					box_frame.frame_type = BOX_SENSOR_FRAME;
					box_frame.box_type_frame_u.sensor_info.len = 0x00;
					box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOSE_RIGHTS;

					box_frame.box_type_frame_u.sensor_info.user_id = box_sensor.user_id;
					box_frame.box_type_frame_u.sensor_info.timestamp = box_sensor.timestamp;
					
					box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
					box_send();
					
					box_operation_status_flag = FALSE;
                    extern void start_gprs_test_stop_timer(void);
					start_gprs_test_stop_timer();
                    //开启锁的中断使能
#ifdef USE_Fingerprints
                        elec_lock_access();
#endif
				}
			}			
		}
	}
	//else if((blu_recv_array[5] == 0x85)||(blu_recv_array[5] == 0x86))
	else if((blu_recv_array[5] == (BOX_BLU_CMD_REPLY_HEAD + BOX_BLU_CMD_ALARM))
		||(blu_recv_array[5] == (BOX_BLU_CMD_REPLY_HEAD + BOX_BLU_CMD_SENSORREPORT))
		||(blu_recv_array[5] == (BOX_BLU_CMD_REPLY_HEAD + BOX_BLU_CMD_BOXSTATE)))
	{
		//0x85--����ָ�����, 0x86--��������Ϣ�ϱ�,0x87--��֪��״̬�ϱ�
		if((blu_recv_array[8] ==0)&&(blu_recv_array[9] ==0))
        {
            blu_cmd_data_send_handler();
        }
//		OSEL_EXIT_CRITICAL(status);
        return;
	}
    else if((blu_recv_array[5] == (BOX_BLU_CMD_TAGINFO)))
	{
		//0x88--RFID TAG READER
//TEST_TAG_READER:
		BluDataSn = (((uint16_t)blu_recv_array[6])<<8) | blu_recv_array[7];
        
        scan_delay = (((uint16_t)blu_recv_array[24])<<8) | blu_recv_array[25];

        NfcReaderTagProcesstimerStart(scan_delay);
        
        NfcReaderRxProcesstimerStart();
        
        blu_flush_recv_buf_from_serial();

        return;
	}
    else
    {
//    	OSEL_EXIT_CRITICAL(status);
        blu_flush_recv_buf_from_serial();
        return;
    }

#if 1
	temp_blu_data[0] = BOX_BLU_CMD_REPLY_HEAD + blu_recv_array[5];
	temp_blu_data_sn = (((uint16_t)blu_recv_array[6])<<8) | blu_recv_array[7] ;
	blu_send_frame((void*)temp_blu_data, temp_blu_data_len,  temp_blu_data_sn);	
#endif

	blu_flush_recv_buf_from_serial();
	blu_send_data();

//    OSEL_EXIT_CRITICAL(status);	
}


/**
* @breif 接收串口处理
*
*/
void blu_uart_event_handler(void)
{
	osel_event_t event;
	uint8_t read_data = 0;
	//uint32_t i = 0;

	while ((blu_auto_read(&read_data, sizeof(uint8_t))))
    {
        blu_recv_array[blu_recv_pos++] = read_data;
    }
	
    //解析接收到的数据
    if(my_strstr((const char*)blu_recv_array, 
                    "+WAKE\r\nOK\r\n") != NULL)
    {
		blu_flush_recv_buf_from_serial();
		event.sig = BLU_STATE_TRANS_EVENT;
		event.param = (osel_param_t *)BLU_INIT_SIG;
		osel_post(NULL, &blu_event_process, &event);	 
		return;
    }
    else
    {
		switch (blu_cmd_type)
		{
			case BLU_CMD_NULL:
				blu_flush_recv_buf_from_serial();
				blu_flush_recv_buf_from_app();
				break;
		
			case BLU_CMD_AT:
				blu_cmd_at_recv_handler();
				break;        

			case BLU_CMD_AT_RESET:
				//blu_cmd_qimux_recv_handler();
				break;

			case BLU_CMD_AT_VERSION:
				//blu_cmd_send_recv_handler();
				break;

			case BLU_CMD_AT_SLEEP:

				break;

			case BLU_CMD_AT_ROLE:
				blu_cmd_at_role_recv_handler();
				break;

			case BLU_CMD_AT_INQ:
				blu_cmd_at_inq_recv_handler();
				break;

			case BLU_CMD_AT_CONN:
				blu_cmd_at_conn_recv_handler();
				break;		   

			case BLU_SEND_DATA:

			case BLU_RECV_DATA:
				blu_cmd_data_recv_handler();
				break;  

			default:
				break; 
		}
    }     
	//blu_flush_recv_buf_from_serial();
}



PROCESS_THREAD(blu_event_process, ev, data)
{
	PROCESS_BEGIN();
	
	while(1)
	{   
		if(ev == BLU_SEND_EVENT)
		{
			blu_send_event_handler();
		}
		else if(ev == BLU_STATE_TRANS_EVENT)
		{
			BLU_FSM_DISPATCH(&blu_fsm, *(blu_sig_event_e *)&data);
		}
		else if(ev == BLU_UART_EVENT)
		{
			blu_uart_event_handler();
		}
        else if(ev == BLU_TIMEOUT_EVENT)
		{
            blu_timeout_timer_cb();			
		}
#if 1		
		else if (BOX_BLU_DATA_SENT_EVENT == ev)
		{
			box_blu_data_sent_event_handle(data);
		}  
#endif
#if 1
		else if(ev == BLU_NO_ACK_EVENT)
		{
			blu_no_ack_timer_cb();
		}
#endif
#if 0		
        else if(ev == BLU_NO_DATA_EVENT)
		{
			blu_no_data_timer_cb();
		}
#endif		

		else if(ev == BLU_NO_LOCK_EVENT)
		{
			blu_no_lock_timeout_timer_cb();
		}
		else
        {
            ;
        }
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}


/**
 *
 * @brief powerkey、gsm_rst、power脚初始化
 * @param 无
 * @return 无
 * 
 */
static void blu_gpio_init(void)
{
//INT-----------------
//	P3SEL &= ~(BIT3); 
//	P3DIR &= ~(BIT3);
//	//P3IES |= BIT3; 
//	P3IES &= ~BIT3; 
//	P3IFG &= ~(BIT3); 
//	P3IE |= (BIT3);

	blu_Init();


//RESET---------------
	P4SEL &= ~(BIT5);
	P4DIR |= (BIT5);
	P4OUT |= BIT5;

//POWER ON-----------
	P4SEL &= ~(BIT2);
	P4DIR |= (BIT2);
	P4OUT &= ~BIT2;
	delay_ms(10);
	P4OUT |= BIT2;


}

/**
 *
 * @brief IO口初始化（gsm_power、powerkey、reset）
 *        发送缓存和接收缓存的初始化、任务建立与消息绑定
 * @param blu_cfg ip和port的配置
 * @return 无
 * 
 */

void blu_init(const blu_init_cfg_t *blu_cfg)
{  
    DBG_ASSERT(blu_cfg != NULL __DBG_LINE);
    DBG_ASSERT(blu_cfg->blu_recv_cb != NULL __DBG_LINE);
    DBG_ASSERT(blu_cfg->blu_send_cb != NULL __DBG_LINE);   

    blu_gpio_init();

	sqqueue_ctrl_init(&blu_auto_recv_sqq, sizeof(uint8_t), 128);

    blu_flush_recv_buf_from_app();  // 清空接收缓冲  
    wsnos_init_printf(NULL, NULL);//仅使用wsnos_sprintf
#if 0
    hal_uart_init(BLU_UART, 9600); 
//wangjian blu
	serial_write(BLU_UART, BLU_AT_BAUD_115200, sizeof(BLU_AT_BAUD_115200)-1);

	delay_ms(200);
	//hal_uart_init(BLU_UART, 115200); 
#else
        hal_uart_init(BLU_UART, 115200); 
	serial_write(BLU_UART, BLU_AT_BAUD_9600, sizeof(BLU_AT_BAUD_9600)-1);
        
        hal_uart_init(BLU_UART, 9600); 
	//serial_write(BLU_UART, BLU_AT_BAUD_115200, sizeof(BLU_AT_BAUD_115200)-1);
	delay_ms(200);
#endif        
    //blu_if_connected = TRUE;
    blu_if_connected = FALSE;

	blu_cur_power_flag = TRUE;

	
    //配置blu模块的ip和port

    blu_recv_cb = blu_cfg->blu_recv_cb;
    blu_tx_cb = blu_cfg->blu_send_cb;
    
    // 创建任务
    blu_task_tcb = osel_task_create(NULL, BLU_TASK_PRIO, blu_event_store, BLU_EVENT_MAX);
	osel_pthread_create(blu_task_tcb, &blu_event_process, NULL);
    // 消息绑定
    osel_etimer_ctor(&blu_timeout_timer, &blu_event_process, BLU_TIMEOUT_EVENT, NULL);
	osel_etimer_ctor(&blu_no_lock_timer, &blu_event_process, BLU_NO_LOCK_EVENT, NULL);	
	osel_etimer_ctor(&blu_no_ack_timer, &blu_event_process, BLU_NO_ACK_EVENT, NULL);
}
