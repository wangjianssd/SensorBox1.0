/**
 * @brief       : .cpp主要处理gprs模块的发送处理    
 *
 * @file        : gprs_tx.c 
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
//#include <pbuf.h>
//#include <printf.h>

/*************宏定义******************/ 
#define GPRS_SERIAL_LEN_MAX          100u
#define GPRS_SERIAL_LEN_MIN          4u
#define GPRS_SEND_CMD_MAX_CNT        3u
#define GPRS_NO_DATA_MAX             20000u//60s
#define GPRS_PREPARE_CONNECT_MAX     1000u//6s
#define GPRS_CONNECT_MAX             8000u//10s
#define GPRS_SEND_DATA_MAX           500u//6s 此处延迟500ms
#define GPRS_SEND_DATA_ERROR_MAX     3000u//3s
#define GPRS_SEND_DATA_COUNT_MAX     30u//怀疑TCP/UDP断开连接，数据发送的最大次数

#define  GPRS_AT                     "AT\r"
#define  GPRS_ATE0                   "ATE0\r"
#define  GPRS_CIPMUX                 "AT+CIPMUX=0\r" //设置单链接模式
#define  GPRS_CIPRXGET               "AT+CIPRXGET=1\r"
#define  GPRS_CIPQRCLOSE             "AT+CIPQRCLOSE=1\r"
#define  GPRS_CIPSPRT                "AT+CIPSPRT=0\r"
#define  GPRS_CGATT                  "AT+CGATT?\r"
#define  GPRS_CPIN                   "AT+CPIN?\r"
#define  GPRS_CIPSTART               "AT+CIPSTART="
#define  GPRS_CIPSEND                "AT+CIPSEND="
#define  GPRS_CIPSTATUS              "AT+CIPSTATUS\r"
#define  GPRS_CIPSHUT                "AT+CIPSHUT\r"
#define  GPRS_CIPCLOSE               "AT+CIPCLOSE\r"
#define  GPRS_CR                     "\r"

#ifdef USE_GPRS_M26 
#define GPRS_QIMUX                   "AT+QIMUX:0\r" 
#define GPRS_QINDI                   "AT+QINDI=1\r" 
#define GPRS_QIOPEN                  "AT+QIOPEN=" 
#define GPRS_QISTAT                  "AT+QISTAT\r" 
#define GPRS_QIDEACT                 "AT+QIDEACT\r" 
#define GPRS_QICLOSE                 "AT+QICLOSE\r"
#define GPRS_QISEND                  "AT+QISEND="
#define GPRS_QIPROMPT                "AT+QIPROMPT=0\r"
#define GPRS_QCELLLOC                "AT+QCELLLOC=1\r"
#endif

#define  GPRS_RESTARTING             "gprs restart\r\n"
#define  GPRS_OPENING                "gprs open\r\n"
#define  GPRS_CLOSEING               "gprs close\r\n"
#define  GPRS_POWERSWITCH            "gpsr power switch\r\n"

extern sqqueue_ctrl_t auto_recv_sqq;
/*************全局变量******************/
gprs_init_cfg_t gprs_cur_cfg;
gprs_data_type_t gprs_data_recv_from_app;
bool_t gprs_cur_power_flag = FALSE;
gprs_cmd_type_e gprs_cmd_type = GPRS_CMD_NULL;
bool_t gprs_if_can_send = TRUE;
bool_t gprs_if_connected = FALSE;
/*************私有变量******************/
//远程主机地址
static uint8_t ip_config[48]; 
static const uint8_t ip_tcp_mode[] = {"\"TCP\""};     
static const uint8_t ip_udp_mode[] = {"\"UDP\""}; 

//定义一个回调指针
gprs_send_cb_t gprs_tx_cb;
gprs_recv_cb_t gprs_recv_cb;

fsm_t fsm_gprs = {0};
static osel_etimer_t gprs_timeout_timer;
static osel_etimer_t gprs_no_data_timer;//给串口发指令或数据时设置的定时器
static osel_etimer_t gprs_send_data_error_timer;//发送数据超时失败定时器

#define GPRS_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t gprs_event_store[GPRS_EVENT_MAX];
static osel_task_t *gprs_task_tcb = NULL;

PROCESS(gprs_event_process, "gprs_event_process");

/*************声明函数*******************/
static void gprs_power_on_state(fsm_t *me, sig_event_e sig);
uint8_t doubt_no_connect_count = 0;//怀疑断开连接的次数

//发送数据超时失败定时器
void stop_gprs_send_data_error_timeout(void)
{
    osel_etimer_disarm(&gprs_send_data_error_timer);
}

static void gprs_send_data_error_timer_cb(void)
{
    //做数据重传
    (*gprs_tx_cb)(FALSE,gprs_data_recv_from_app.gprs_sn);
    gprs_if_connected = TRUE;
    doubt_no_connect_count ++;
}

static void gprs_send_data_error_timeout_set(uint32_t ticks)
{
    stop_gprs_send_data_error_timeout();
    osel_etimer_arm(&gprs_send_data_error_timer, ticks, 0);
}

//给串口发指令或数据时设置的定时器
void stop_gprs_no_data_timeout(void)
{
    osel_etimer_disarm(&gprs_no_data_timer);
}
/**
*@brief gprs给串口发指令或数据时设置的定时器回调
*/
static void gprs_no_data_timer_cb(void)
{
    //转入重传状态
    osel_event_t event;
    (*gprs_tx_cb)(FALSE,gprs_data_recv_from_app.gprs_sn);
//    TRAN(gprs_power_off_state);
//    event.sig = GPRS_STATE_TRANS_EVENT;
//    event.param = (osel_param_t *)INIT_SIG;
//    osel_post(NULL, &gprs_event_process, &event);  
}
/**
*@brief 给串口发指令或数据时设置的定时器
*/
static void gprs_no_data_timeout_set(uint32_t ticks)
{
    stop_gprs_no_data_timeout();
    osel_etimer_arm(&gprs_no_data_timer, ticks, 0);
}

/**
*@brief gprs电源操作中定时器定时时间到的回调处理
*/
static void gprs_timeout_timer_cb(void)
{
    osel_event_t event;
	event.sig = GPRS_STATE_TRANS_EVENT;
	event.param = (osel_param_t *)TIMEOUT_SIG;
	osel_post(NULL, &gprs_event_process, &event); 
}
/**
*@brief gprs电源中设定定时器
*/
static void timeout_set(uint32_t ticks)
{
    osel_etimer_arm(&gprs_timeout_timer, ticks, 0);
}

/**
 *@brief 查询当前gprs模块是否可以发送
 */
bool_t gprs_tran_can_send(void)
{
    return gprs_if_can_send;
}


/**
 *@brief 向gprs发送AT指令和数据
 */
extern void gprs_flush_recv_buf_from_serial(void);
static void gprs_send_cmd(uint8_t *cmd, uint8_t len)
{
    //gprs_flush_recv_buf_from_serial();
    DBG_ASSERT(cmd != NULL __DBG_LINE);
    DBG_ASSERT(len != 0 __DBG_LINE);
    
    serial_write(UART_3, cmd, len);
}

/**
 *@brief gprs建立udp或tcp连接
 */
#ifdef USE_GPRS_M26
static void gprs_cmd_qiopen(void)
#else
static void gprs_cmd_cipstart(void)
#endif
{
    uint8_t cmd[0x30] = {0};
    uint8_t ip[4] = {0};
    uint16_t  port = 0;
    gprs_send_mode_e data_mode_temp;
        
    osel_memset(cmd, 0x00, sizeof(cmd));
    
    port = gprs_cur_cfg.port; 
    ip[0]   = HI_1_UINT32(gprs_cur_cfg.ip_addr);
    ip[1]   = HI_2_UINT32(gprs_cur_cfg.ip_addr);
    ip[2]   = HI_3_UINT32(gprs_cur_cfg.ip_addr);
    ip[3]   = HI_4_UINT32(gprs_cur_cfg.ip_addr);
    wsnos_sprintf((char *)ip_config, 
                  "\"%d.%d.%d.%d\",%u", 
                  ip[0], ip[1], ip[2], ip[3], port);      
    data_mode_temp = gprs_data_recv_from_app.gprs_data_mode;
    //TCP
    if ((data_mode_temp == TCP_SINGLE)||(data_mode_temp == TCP_CONTINE))
    {
        wsnos_sprintf((char *)cmd, 
			         #ifdef USE_GPRS_M26
                      "AT+QIOPEN=%s,%s\r",
					 #else
                      "AT+CIPSTART=%s,%s\r",
                     #endif 
                      (char *)ip_tcp_mode, 
                      (char *)ip_config);        
    }//UDP
    else if ((data_mode_temp == UDP_SINGLE)||(data_mode_temp == UDP_CONTINE))
    {
        wsnos_sprintf((char *)cmd, 
                     #ifdef USE_GPRS_M26
                      "AT+QIOPEN=%s,%s\r",
					 #else
                      "AT+CIPSTART=%s,%s\r",
                     #endif
                      (char *)ip_udp_mode, 
                      (char *)ip_config);        
    }
        
    gprs_send_cmd(cmd, mystrlen((char *)cmd));
}


/**
 *@brief gprs发送数据长度
 */
#ifdef USE_GPRS_M26
static void gprs_cmd_qisend_len(void)
#else
static void gprs_cmd_cipsend_len(void)
#endif
{
    uint8_t len;
    uint8_t cmd[128] = {0};
    osel_memset(cmd, 0x00, sizeof(cmd));
    
    len = gprs_data_recv_from_app.gprs_data_len;
    wsnos_sprintf((char *)cmd,
                #ifdef USE_GPRS_M26
                  "AT+QISEND=%d\r",
                #else
                  "AT+CIPSEND=%d\r", 
                #endif
                  len); //AT+CIPSEND=LEN\r
    gprs_send_cmd(cmd, mystrlen((char *)cmd));  
}


/**
 *@brief gprs发送数据
 */

static void gprs_send_data(void)
{
    gprs_send_cmd(gprs_data_recv_from_app.gprs_data, 
                  gprs_data_recv_from_app.gprs_data_len);
}

static void gprs_send_ciprxget_2(void)
{
    uint8_t len;
    uint8_t cmd[128] = {0};
    osel_memset(cmd, 0x00, sizeof(cmd));
    
    len = gprs_data_recv_from_app.gprs_data_len;
    wsnos_sprintf((char *)cmd, "AT+CIPRXGET=2,%d\r",len);
    gprs_send_cmd(cmd, mystrlen((char *)cmd));    
}

#ifdef USE_GPRS_M26

static void gprs_send_qird(void)
{
    uint8_t len;
    uint8_t cmd[128] = {0};
    osel_memset(cmd, 0x00, sizeof(cmd));
    
    len = gprs_data_recv_from_app.gprs_data_len;
    wsnos_sprintf((char *)cmd, "AT+QIRD=0,1,0,%d\r",len);
    gprs_send_cmd(cmd, mystrlen((char *)cmd));    
}

#endif


/**
 *@brief gprs接收服务器数据状态处理
 */
void gprs_recv_state(fsm_t *me,sig_event_e sig)
{
    osel_event_t event;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(500);//500TICK=5000ms
        break;
        
    case TIMEOUT_SIG:
        event.sig = GPRS_STATE_TRANS_EVENT;
       #ifdef USE_GPRS_M26		
        event.param = (osel_param_t *)SEND_QIRD;
	   #else
        event.param = (osel_param_t *)SEND_CIPRXGET_2;
	   #endif
        osel_post(NULL, &gprs_event_process, &event);        
        break;
        
    case SEND_CIPRXGET_2:
        gprs_cmd_type = GPRS_RECV_DATA;
        gprs_send_ciprxget_2();
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;

   #ifdef USE_GPRS_M26
    case SEND_QIRD:
        gprs_cmd_type = GPRS_RECV_DATA;
        gprs_send_qird();
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);

   #endif

    default:
        break;
    }
}

/**
 *@brief gprs发送状态处理
 */
void gprs_send_state(fsm_t *me,sig_event_e sig)
{
    osel_event_t event;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(GPRS_SEND_DATA_MAX/OSEL_TICK_PER_MS);
        break;
        
    case TIMEOUT_SIG:
        event.sig = GPRS_STATE_TRANS_EVENT;
      #ifdef USE_GPRS_M26
        event.param = (osel_param_t *)SEND_QISEND_LEN;
	  #else
        event.param = (osel_param_t *)SEND_CIPSEND_LEN;
	  #endif
        osel_post(NULL, &gprs_event_process, &event);        
        break;
		
   #ifdef USE_GPRS_M26
    case SEND_QISEND_LEN:
        gprs_flush_recv_buf_from_serial();
        gprs_cmd_type = GPRS_CMD_SEND_LEN;		
        gprs_cmd_qisend_len();
        gprs_send_data_error_timeout_set(GPRS_SEND_DATA_ERROR_MAX/OSEL_TICK_PER_MS);
        break;
	case SEND_QCELLLOC:
        gprs_cmd_type = GPRS_CMD_QCELLLOC;
		gprs_send_cmd(GPRS_QCELLLOC, sizeof(GPRS_QCELLLOC) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        
		break;
   #else
    case SEND_CIPSEND_LEN:
        gprs_cmd_type = GPRS_CMD_SEND_LEN;
        gprs_cmd_cipsend_len();
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;
   #endif
	  
    case SEND_DATA:
        gprs_cmd_type = GPRS_SEND_DATA;
        gprs_send_data();
        gprs_send_data_error_timeout_set(GPRS_SEND_DATA_ERROR_MAX/OSEL_TICK_PER_MS);        
        break;
        
    default:
        break;               
    }     
}

/**
 *@brief gprs连接状态处理
 */
#define GPRS_CONNECT_TIMEOUT_COUNT
void gprs_connect_state(fsm_t *me,sig_event_e sig)
{
    osel_event_t event;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(GPRS_CONNECT_MAX/OSEL_TICK_PER_MS);
        break;
        
    case TIMEOUT_SIG:
      
        #ifdef GPRS_CONNECT_TIMEOUT_COUNT
        uint8_t static GprsTimeoutCount = 0;

        GprsTimeoutCount++;

        if (GprsTimeoutCount > 3)
        {
            TRAN(gprs_power_off_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);  
            break;   
        }
        #endif
        
        event.sig = GPRS_STATE_TRANS_EVENT;
      #ifdef USE_GPRS_M26
        event.param = (osel_param_t *)SEND_QIOPEN;
	  #else
        event.param = (osel_param_t *)SEND_CIPSTART;
	  #endif
        osel_post(NULL, &gprs_event_process, &event);        
        break;         

   #ifdef USE_GPRS_M26
    case SEND_QIOPEN:
        gprs_cmd_type = GPRS_CMD_QIOPEN;
		gprs_cmd_qiopen();
   #else
    case SEND_CIPSTART:
        gprs_cmd_type = GPRS_CMD_CIPSTART;
        gprs_cmd_cipstart();
   #endif
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;
    
    default:
        break;               
    }    
}


/**
 *@brief gprs连接准备状态处理
 */
void gprs_prepare_connect_state(fsm_t *me, sig_event_e sig)
{
    osel_event_t event;
    switch (sig)
    {
     case INIT_SIG:
     	timeout_set(GPRS_PREPARE_CONNECT_MAX/OSEL_TICK_PER_MS);
     	break;
     	
     case TIMEOUT_SIG:		
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_CGATT;
        osel_post(NULL, &gprs_event_process, &event);		 
     	break;		  
     	
     case SEND_CGATT:
     	gprs_cmd_type = GPRS_CMD_CGATT;
     	gprs_send_cmd(GPRS_CGATT, sizeof(GPRS_CGATT) - 1);
     	gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
     	break;
	
    case SEND_AT:
        gprs_cmd_type = GPRS_CMD_AT;
        gprs_send_cmd(GPRS_AT, sizeof(GPRS_AT) - 1);
        break;
        
    case SEND_ATE0:
        gprs_cmd_type = GPRS_CMD_ATE0;
		gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        gprs_send_cmd(GPRS_ATE0, sizeof(GPRS_ATE0) - 1);
        
        break; 

    case SEND_CIPMUX:
        gprs_cmd_type = GPRS_CMD_CIPMUX;
        gprs_send_cmd(GPRS_CIPMUX, sizeof(GPRS_CIPMUX) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break; 
        
    case SEND_CIPRXGET://设置从服务器来的响应和串口响应分开
        gprs_cmd_type = GPRS_CMD_CIPRXGET;
        gprs_send_cmd(GPRS_CIPRXGET, sizeof(GPRS_CIPRXGET) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;  

    case SEND_CIPQRCLOSE://设置加速远程断开连接
        gprs_cmd_type = GPRS_CMD_CIPQRCLOSE;
        gprs_send_cmd(GPRS_CIPQRCLOSE, sizeof(GPRS_CIPQRCLOSE) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;

#ifdef USE_GPRS_M26

    case SEND_QIMUX:
        gprs_cmd_type = GPRS_CMD_QIMUX;
        gprs_send_cmd(GPRS_QIMUX, sizeof(GPRS_QIMUX) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break; 

    case SEND_QINDI:
        gprs_cmd_type = GPRS_CMD_QINDI;
        gprs_send_cmd(GPRS_QINDI, sizeof(GPRS_QINDI) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break; 
    
//    case SEND_QIOPEN:
//        gprs_cmd_type = GPRS_CMD_QIOPEN;
//        gprs_send_cmd(GPRS_QIOPEN, sizeof(GPRS_QIOPEN) - 1);
//        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
//        break; 
    
    case SEND_QISTAT:
        gprs_cmd_type = GPRS_CMD_QISTAT;
        gprs_send_cmd(GPRS_QISTAT, sizeof(GPRS_QISTAT) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break; 
    
    case SEND_QIDEACT:
       gprs_cmd_type = GPRS_CMD_QIDEACT;
       gprs_send_cmd(GPRS_QIDEACT, sizeof(GPRS_QIDEACT) - 1);
       gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
       break; 
    
    case SEND_QICLOSE:
       gprs_cmd_type = GPRS_CMD_QICLOSE;
       gprs_send_cmd(GPRS_QICLOSE, sizeof(GPRS_QICLOSE) - 1);
       gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
       break; 

    case SEND_QIPROMPT:
       gprs_cmd_type = GPRS_CMD_QIPROMPT;
       gprs_send_cmd(GPRS_QIPROMPT, sizeof(GPRS_QIPROMPT) - 1);
       gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
       break;
    
#endif
	
    default:
        break;              
    }
}

/**
 *@brief gprs电源打开或关闭之后的处理
 */
void gprs_power_operate_proc(power_operate_e flag)
{
    osel_event_t event;
    if (flag == GPRS_POWER_ON)
    {//转到连接准备态
        gprs_cur_power_flag = TRUE;
        TRAN(gprs_prepare_connect_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_ATE0;
        osel_post(NULL, &gprs_event_process, &event);   
    }
    else if (flag == GPRS_POWER_OFF)
    {
        gprs_cur_power_flag = FALSE;
        //如果是建立连接失败或者是串口未回应">",则转入上电态
        if ((gprs_cmd_type == GPRS_CMD_AT)
            ||(gprs_cmd_type == GPRS_CMD_ATE0)
            ||(gprs_cmd_type == GPRS_CMD_CGATT)			
          #ifdef USE_GPRS_M26
            ||(gprs_cmd_type == GPRS_CMD_QIOPEN)
          #else
            ||(gprs_cmd_type == GPRS_CMD_CIPSTART)
		  #endif
            ||(gprs_cmd_type == GPRS_CMD_SEND_LEN))
        {
            gprs_cmd_type = GPRS_CMD_NULL;
            TRAN(gprs_power_on_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);                       
        }
        else if (gprs_cmd_type == GPRS_SEND_DATA)
        {
            gprs_cmd_type = GPRS_CMD_NULL;
        }
    }
}

/**
*@brief gprs上电状态处理
*/
void gprs_power_on_state(fsm_t *me, sig_event_e sig)
{
    static uint8_t on_timeout_cnt = 0;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);

		extern uint8_t gprs_cmd_send_cgatt_count;//附着连接次数
		extern uint8_t gprs_cmd_send_cipstart_count;//建立连接次数
		extern uint8_t gprs_cmd_send_len_count;//建立连接次数
        extern uint8_t gprs_cmd_send_qiopen_count; 
		gprs_cmd_send_cgatt_count = 0;
		gprs_cmd_send_cipstart_count = 0;
		gprs_cmd_send_qiopen_count = 0;
		gprs_cmd_send_len_count = 0;
        break;

    case TIMEOUT_SIG:      
        on_timeout_cnt++;
        if (on_timeout_cnt == 1)
        {
            POWER_IOA_ON();
            POWER_IOB_OFF();
            timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
        }
        else if (on_timeout_cnt == 2)
        {
            POWER_IOB_ON();
            timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
        }
        else if (on_timeout_cnt == 3)
        {
            POWER_IOB_OFF();
            timeout_set((GPRS_TIMEOUT_TIME_PER)/OSEL_TICK_PER_MS);
        }
        else if (on_timeout_cnt == 4)
        {
            timeout_set((GPRS_TIMEOUT_TIME_PER)/OSEL_TICK_PER_MS);
        }        
        else if (on_timeout_cnt == 5)
        {
            on_timeout_cnt = 0;
            gprs_power_operate_proc(GPRS_POWER_ON);
        }        
        else
        {
            on_timeout_cnt = 0;
        }
        break;

    default:
        break;
    }
}
/**
*@brief gprs掉电状态处理
*/
void gprs_power_off_state(fsm_t *me, sig_event_e sig)
{
    static uint8_t off_timeout_cnt = 0;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
        POWER_IOB_ON();
        break;

    case TIMEOUT_SIG:
        off_timeout_cnt++;
        if (off_timeout_cnt == 1)
        {
            timeout_set((GPRS_TIMEOUT_TIME_PER*2)/OSEL_TICK_PER_MS);
            POWER_IOB_OFF();
        }
        else if (off_timeout_cnt == 2)
        {
            off_timeout_cnt = 0;
            POWER_IOA_OFF();
            gprs_cur_power_flag = FALSE;
            timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
            gprs_power_operate_proc(GPRS_POWER_OFF);
        }
        else if (off_timeout_cnt == 3)
        {
            gprs_power_operate_proc(GPRS_POWER_OFF);
        }
        else
        {
            off_timeout_cnt = 0;
        }
        break;
    }
}

/**
 *
 * @brief gprs数据发送
 * @param 无
 * @return 无
 * 
 */
static void gprs_send_event_handler(void)
{
    gprs_if_can_send = FALSE;
    osel_event_t event;
    //如果gprs未上电，转入上电状态
    if (!gprs_cur_power_flag)
    {
        TRAN(gprs_power_on_state);
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t)INIT_SIG;
        osel_post(NULL, &gprs_event_process, &event);        
    }
    else
    {//转入建立连接态
        if (doubt_no_connect_count >= GPRS_SEND_DATA_COUNT_MAX)//如果数据发送失败10次,则直接转入到重新建立连接
        {
            TRAN(gprs_connect_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);             
        }
        else
        {
            if (gprs_if_connected)//如果当前网络连接正常，则转入发数据态
            {
                TRAN(gprs_send_state);
                event.sig = GPRS_STATE_TRANS_EVENT;
                event.param = (osel_param_t *)INIT_SIG;//TIMEOUT_SIG   INIT_SIG
                osel_post(NULL, &gprs_event_process, &event);                       
            }
            else //如果当前网络连接不正常，则转入连接态
            {
                TRAN(gprs_connect_state);
                event.sig = GPRS_STATE_TRANS_EVENT;
                event.param = (osel_param_t *)INIT_SIG;
                osel_post(NULL, &gprs_event_process, &event);                         
            }            
        }       
    }
}

PROCESS_THREAD(gprs_event_process, ev, data)
{
	PROCESS_BEGIN();
	
	while(1)
	{   
		if(ev == GPRS_SEND_EVENT)
		{
			gprs_send_event_handler();
		}
		else if(ev == GPRS_STATE_TRANS_EVENT)
		{
			FSM_DISPATCH(&fsm_gprs, *(sig_event_e *)&data);
		}
		else if(ev == GPRS_UART_EVENT)
		{
			gprs_uart_event_handler();
		}
        else if(ev == GPRS_TIMEOUT_EVENT)
		{
            gprs_timeout_timer_cb();			
		}
         else if(ev == GPRS_NO_DATA_EVENT)
		{
			gprs_no_data_timer_cb();
		}
		else if (ev == GPRS_SEND_DATA_ERROR_EVENT)
        {
            gprs_send_data_error_timer_cb();
        }
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}

//暂时未使用
#ifdef USE_GPRS_M26
void gprs_qcellloc_get(void)
{
	osel_event_t event;

    event.sig = GPRS_SEND_EVENT;
    event.param = (osel_param_t *)SEND_QCELLLOC;
    osel_post(NULL, &gprs_event_process, &event);  
}
#endif

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
void gprs_tran_send(void *data_p, uint8_t len,  
                      gprs_send_mode_e connect_mode,
                      uint16_t sn)
{
    osel_event_t event;
    DBG_ASSERT(data_p != NULL __DBG_LINE);
    DBG_ASSERT(len != 0 __DBG_LINE);
    
    //接收来至app的数据
    osel_int_status_t s;
   
    OSEL_ENTER_CRITICAL(s);
    gprs_data_recv_from_app.gprs_sn = sn;
    gprs_data_recv_from_app.gprs_data_len = len;
    gprs_data_recv_from_app.gprs_data_mode = connect_mode;
    osel_memcpy(gprs_data_recv_from_app.gprs_data,data_p,len);
    OSEL_EXIT_CRITICAL(s);
    
    //发给GPRS任务处理
    event.sig = GPRS_SEND_EVENT;
    event.param = NULL;
    osel_post(NULL, &gprs_event_process, &event);    
}

/**
 *
 * @brief 接收缓冲数组的初始化
 * @param 无
 * @return 无
 * 
 */
void gprs_flush_recv_buf_from_app(void)
{
    gprs_data_recv_from_app.gprs_data_len = 0;
    gprs_data_recv_from_app.gprs_sn = 0;
    gprs_data_recv_from_app.gprs_data_mode = INVALID_MODE;
    osel_memset(gprs_data_recv_from_app.gprs_data,0x00, GPRS_DATA_MAXLEN * sizeof(uint8_t));
}


#ifdef USE_GPRS_M26
static void m26_gprs_init(void)
{
    //powerkey引脚初始化
    P1SEL &= ~BIT2;
    P1DIR |=  BIT2;
    //电源引脚:关闭电源
    P1SEL &=~BIT3;//Close power
    P1DIR |= BIT3;
    P1OUT |= BIT3;
    
    pin_id_t pin_id;
	pin_id.pin = 1;
	pin_id.bit = 7;
	gpio_sel(pin_id);
	gpio_make_input(pin_id);    
}
#else
/**
 *
 * @brief powerkey、gsm_rst、power脚初始化
 * @param 无
 * @return 无
 * 
 */
static void sim928a_gprs_init(void)
{
    //powerkey引脚初始化
    P1SEL &= ~BIT2;
    P1DIR |=  BIT2;
	//GSM_RST脚
    P1DIR |= BIT7;
    P1OUT |= BIT7;
    //电源引脚:关闭电源
    P1SEL &=~BIT3;//Close power
    P1DIR |= BIT3;
    P1OUT &= ~BIT3;
    
    pin_id_t pin_id;
	pin_id.pin = 1;
	pin_id.bit = 7;
	gpio_sel(pin_id);
	gpio_make_input(pin_id);    
}
#endif

/**
 *
 * @brief IO口初始化（gsm_power、powerkey、reset）
 *        发送缓存和接收缓存的初始化、任务建立与消息绑定
 * @param gprs_cfg ip和port的配置
 * @return 无
 * 
 */
void gprs_tran_init(const gprs_init_cfg_t *gprs_cfg)
{  
    DBG_ASSERT(gprs_cfg != NULL __DBG_LINE);
    DBG_ASSERT(gprs_cfg->gprs_recv_cb != NULL __DBG_LINE);
    DBG_ASSERT(gprs_cfg->gprs_send_cb != NULL __DBG_LINE);   

    sqqueue_ctrl_init(&auto_recv_sqq, sizeof(uint8_t), 128);
#ifdef USE_GPRS_M26
    m26_gprs_init();
#else
    sim928a_gprs_init();  
#endif
    gprs_flush_recv_buf_from_app();  // 清空接收缓冲  
    wsnos_init_printf(NULL, NULL);//仅使用wsnos_sprintf
    
    hal_uart_init(UART_3, 38400);

    gprs_if_can_send = TRUE;
    gprs_if_connected = FALSE;
    doubt_no_connect_count = 0;
    
    //配置gprs模块的ip和port
    gprs_cur_cfg.ip_addr = gprs_cfg->ip_addr;
    gprs_cur_cfg.port = gprs_cfg->port;
    gprs_recv_cb = gprs_cfg->gprs_recv_cb;
    gprs_tx_cb = gprs_cfg->gprs_send_cb;
    
    // 创建任务
    gprs_task_tcb = osel_task_create(NULL, GPRS_TASK_PRIO, gprs_event_store, GPRS_EVENT_MAX);
	osel_pthread_create(gprs_task_tcb, &gprs_event_process, NULL);
    // 消息绑定
    osel_etimer_ctor(&gprs_timeout_timer, &gprs_event_process, GPRS_TIMEOUT_EVENT, NULL);
	osel_etimer_ctor(&gprs_no_data_timer, &gprs_event_process, GPRS_NO_DATA_EVENT, NULL); 
    osel_etimer_ctor(&gprs_send_data_error_timer, &gprs_event_process, GPRS_SEND_DATA_ERROR_EVENT, NULL); 
}
