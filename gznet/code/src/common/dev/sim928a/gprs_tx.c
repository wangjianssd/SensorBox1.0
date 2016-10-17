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

#include "common/dev/sim928a/gprs.h"

/*************宏定义******************/ 
#define GPRS_SERIAL_LEN_MAX          100u
#define GPRS_SERIAL_LEN_MIN          4u
#define GPRS_SEND_CMD_MAX_CNT        3u
#define GPRS_NO_DATA_MAX             60000u
#define GPRS_PREPARE_CONNECT_MAX     6000u
#define GPRS_CONNECT_MAX             10000u
#define GPRS_SEND_DATA_MAX           6000u

#define  GPRS_AT                     "AT\r"
#define  GPRS_ATE0                   "ATE0\r"
#define  GPRS_CIPSPRT                "AT+CIPSPRT=0\r"
#define  GPRS_CGATT                  "AT+CGATT?\r"
#define  GPRS_CPIN                   "AT+CPIN?\r"
#define  GPRS_CIPSTART               "AT+CIPSTART="
#define  GPRS_CIPSEND                "AT+CIPSEND="
#define  GPRS_CIPSTATUS              "AT+CIPSTATUS\r"
#define  GPRS_CIPSHUT                "AT+CIPSHUT\r"
#define  GPRS_CIPCLOSE               "AT+CIPCLOSE\r"
#define  GPRS_CR                     "\r"

#define  GPRS_RESTARTING             "gprs restart\r\n"
#define  GPRS_OPENING                "gprs open\r\n"
#define  GPRS_CLOSEING               "gprs close\r\n"
#define  GPRS_POWERSWITCH            "gpsr power switch\r\n"

extern sqqueue_ctrl_t auto_recv_sqq;
/*************全局变量******************/
gprs_init_cfg_t gprs_cur_cfg;
gprs_data_type_t gprs_data_recv_from_app;
gprs_data_type_t gprs_data_recv_from_app_last;
bool_t gprs_cur_power_flag = FALSE;
gprs_cmd_type_e gprs_cmd_type = GPRS_CMD_NULL;
bool_t gprs_if_can_send = TRUE;
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

#define GPRS_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t gprs_event_store[GPRS_EVENT_MAX];
static osel_task_t *gprs_task_tcb = NULL;

PROCESS(gprs_event_process, "gprs_event_process");

/*************声明函数*******************/
static void gprs_power_on_state(fsm_t *me, sig_event_e sig);

void pow_iob_on()
{
    P1OUT |= BIT2;
}

void pow_iob_off()
{
    P1OUT &= ~BIT2;
}
void stop_gprs_no_data_timeout(void)
{
    osel_etimer_disarm(&gprs_no_data_timer); 
}
/**
*@brief gprs给串口发指令或数据时设置的定时器回调
*/
static void gprs_no_data_timer_cb(void)
{
    TRAN(gprs_power_off_state);
    osel_event_t event;
    event.sig = GPRS_STATE_TRANS_EVENT;
    event.param = (osel_param_t *)INIT_SIG;
    osel_post(NULL, &gprs_event_process, &event);
    
    //转入掉电状态
//    TRAN(gprs_power_off_state);
//    osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW); 
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
    
//    gprs_timeout_timer = NULL;
//    osel_post(GPRS_STATE_TRANS_EVENT, (void *)TIMEOUT_SIG, OSEL_EVENT_PRIO_LOW);
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
static void gprs_send_cmd(uint8_t *cmd, uint8_t len)
{
    DBG_ASSERT(cmd != NULL __DBG_LINE);
    DBG_ASSERT(len != 0 __DBG_LINE);
    
    serial_write(HAL_UART_3, cmd, len);
}

/**
 *@brief gprs建立udp或tcp连接
 */
static void gprs_cmd_cipstart(void)
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
                      "AT+CIPSTART=%s,%s\r", 
                      (char *)ip_tcp_mode, 
                      (char *)ip_config);        
    }//UDP
    else if ((data_mode_temp == UDP_SINGLE)||(data_mode_temp == UDP_CONTINE))
    {
        wsnos_sprintf((char *)cmd, 
                      "AT+CIPSTART=%s,%s\r", 
                      (char *)ip_udp_mode, 
                      (char *)ip_config);        
    }
        
    gprs_send_cmd(cmd, mystrlen((char *)cmd));
}

/**
 *@brief gprs发送数据长度
 */
static void gprs_cmd_cipsend_len(void)
{
    uint8_t len;
    uint8_t cmd[128] = {0};
    osel_memset(cmd, 0x00, sizeof(cmd));
    
    len = gprs_data_recv_from_app.gprs_data_len;
    wsnos_sprintf((char *)cmd, "AT+CIPSEND=%d\r", len); //AT+CIPSEND=LEN\r
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
        event.param = (osel_param_t *)SEND_CIPSEND_LEN;
        osel_post(NULL, &gprs_event_process, &event);
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CIPSEND_LEN, OSEL_EVENT_PRIO_LOW);
        break;
        
    case SEND_CIPSEND_LEN:
        gprs_cmd_type = GPRS_CMD_SEND_LEN;
        gprs_cmd_cipsend_len();
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;

    case SEND_DATA:
        gprs_cmd_type = GPRS_SEND_DATA;
        gprs_send_data();
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;
        
    default:
        break;               
    }     
}

/**
 *@brief gprs连接状态处理
 */
void gprs_connect_state(fsm_t *me,sig_event_e sig)
{
    osel_event_t event;
    switch (sig)
    {
    case INIT_SIG:
        timeout_set(GPRS_CONNECT_MAX/OSEL_TICK_PER_MS);
        break;
        
    case TIMEOUT_SIG:
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_CIPSTART;
        osel_post(NULL, &gprs_event_process, &event);
        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CIPSTART, OSEL_EVENT_PRIO_LOW);
        break;         
        
    case SEND_CIPSTART:
//        printf("GPRS_CMD_CIPSTART");
        gprs_cmd_type = GPRS_CMD_CIPSTART;
        gprs_cmd_cipstart();
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
    case SEND_AT:
        gprs_cmd_type = GPRS_CMD_AT;
        gprs_send_cmd(GPRS_AT, sizeof(GPRS_AT) - 1);
        break;
        
    case SEND_ATE0:
//        printf("SEND_ATE0");
        gprs_cmd_type = GPRS_CMD_ATE0;
        gprs_send_cmd(GPRS_ATE0, sizeof(GPRS_ATE0) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break; 

    case INIT_SIG:
        timeout_set(GPRS_PREPARE_CONNECT_MAX/OSEL_TICK_PER_MS);
        break;
        
    case TIMEOUT_SIG:
        event.sig = GPRS_STATE_TRANS_EVENT;
        event.param = (osel_param_t *)SEND_CGATT;
        osel_post(NULL, &gprs_event_process, &event);
        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_CGATT, OSEL_EVENT_PRIO_LOW);
        break;        
        
    case SEND_CGATT:
//        printf("SEND_CGATT");
        gprs_cmd_type = GPRS_CMD_CGATT;
        gprs_send_cmd(GPRS_CGATT, sizeof(GPRS_CGATT) - 1);
        gprs_no_data_timeout_set(GPRS_NO_DATA_MAX/OSEL_TICK_PER_MS);
        break;
    
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
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)SEND_ATE0, OSEL_EVENT_PRIO_LOW);
    }
    else if (flag == GPRS_POWER_OFF)
    {
        gprs_cur_power_flag = FALSE;
        //如果是建立连接失败或者是串口未回应">",则转入上电态
        if ((gprs_cmd_type == GPRS_CMD_AT)
            ||(gprs_cmd_type == GPRS_CMD_ATE0)
            ||(gprs_cmd_type == GPRS_CMD_CGATT)
            ||(gprs_cmd_type == GPRS_CMD_SEND_LEN)
            ||(gprs_cmd_type == GPRS_CMD_CIPSTART))
        {
            gprs_cmd_type = GPRS_CMD_NULL;
            TRAN(gprs_power_on_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);
            
//            osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);           
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
//        printf("POWER ON");
        timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
        POWER_IOA_ON();
        POWER_IOB_OFF();
        break;

    case TIMEOUT_SIG:
        on_timeout_cnt++;
        if (on_timeout_cnt == 1)
        {
            POWER_IOB_ON();
            timeout_set(GPRS_TIMEOUT_TIME_PER/OSEL_TICK_PER_MS);
        }
        else if (on_timeout_cnt == 2)
        {
            POWER_IOB_OFF();
            timeout_set((GPRS_TIMEOUT_TIME_PER * 2)/OSEL_TICK_PER_MS);
        }
        else if (on_timeout_cnt == 3)
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
        
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);
    }
    else
    {//转入建立连接态
        if(gprs_data_recv_from_app_last.gprs_data_mode != gprs_data_recv_from_app.gprs_data_mode)
        {
            TRAN(gprs_connect_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)INIT_SIG;
            osel_post(NULL, &gprs_event_process, &event);
        }
        if(gprs_data_recv_from_app_last.gprs_data_mode == gprs_data_recv_from_app.gprs_data_mode)
        {
            TRAN(gprs_send_state);
            event.sig = GPRS_STATE_TRANS_EVENT;
            event.param = (osel_param_t *)SEND_CIPSEND_LEN;
            osel_post(NULL, &gprs_event_process, &event);
        }
//        osel_post(GPRS_STATE_TRANS_EVENT, (void *)INIT_SIG, OSEL_EVENT_PRIO_LOW);      
    }
}

/**
 *
 * @brief gprs事件处理
 * @param e 事件内容
 * @return 无
 * 
 */
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
		else
        {
            
        }
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}


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
    DBG_ASSERT(data_p != NULL __DBG_LINE);
    DBG_ASSERT(len != 0 __DBG_LINE);
    osel_event_t event;
    
    //接收来至app的数据
    OSEL_ISR_ENTRY();
    gprs_data_recv_from_app.gprs_sn = sn;
    gprs_data_recv_from_app.gprs_data_len = len;
    gprs_data_recv_from_app.gprs_data_mode = connect_mode;
    osel_memcpy(gprs_data_recv_from_app.gprs_data,data_p,len);
    OSEL_ISR_EXIT();
    
    //发给GPRS任务处理
    event.sig = GPRS_SEND_EVENT;
    event.param = NULL;
    osel_post(NULL, &gprs_event_process, &event);
    
//    osel_post(GPRS_SEND_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
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

void gprs_pow_close(void)
{
    P1SEL &=~BIT3;//Close power
    P1DIR |= BIT3;
    P1OUT &= ~BIT3; 
}

void gprs_pow_open(void)
{
    P1SEL &=~BIT3;//Close power
    P1DIR |= BIT3;
    P1OUT |= BIT3; 
}

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
    sim928a_gprs_init();    
    gprs_flush_recv_buf_from_app();  // 清空接收缓冲  
    wsnos_init_printf(NULL, NULL);   // 仅使用wsnos_sprintf
    
    hal_uart_init(HAL_UART_3, 38400);

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
}
