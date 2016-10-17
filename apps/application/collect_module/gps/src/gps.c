/**
 * @brief       : 主要是gps模块的驱动
 *
 * @file        : gps.c
 * @author      : zhangzhan
 * @version     : v0.0.2
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1       zhangzhan   文件初始版本
 * - 2015/9/28    v0.0.2       xukai       修改部分内容
 */
#include <gznet.h>
//#include "wsnos.h"
//#include "hal.h"
//#include "lib.h"
//#include "serial.h"
#include "gps.h"
#include <stdio.h>
#include <string.h>

#define GPS_DATA_PERIOD      		10
#define GPS_PROTECT_TIMER_PERIOD  	(65ul)			// 保护定时器:85s
#define GPS_WAIT_FOR_OPEN_TIME  	(60ul)//(9ul)			// GPS等待打开定时器:9s
#define GPS_OPEN_TIME  				(1ul)			// GPS打开后等待配置定时器:1s

#define GPS_CLOSE           	    0u
#define GPS_OPEN            	    1u
#define GPS_NULL            	    2u

#define GPS_SLEEP                   3u
#define GPS_WAKEUP                  4u


#if GPS_DEBUG_INFO == 1
uint8_t gps_cmd_cnt = 0;
uint8_t acc_status_flag = 0;
#endif
      
static void gps_protect_timer_start(void);
static void gps_close_intra(void);
static void gps_open_intra(void);

typedef struct _gps_data_t
{
    gps_cb_t    cb;
    uint16_t    period;
}gps_data_t;

//static hal_timer_t *gps_protect_timer = NULL;   	// GPS故障发现定时
//static hal_timer_t *gps_wait_for_open_timer = NULL; // GPS故障处理定时
//static hal_timer_t *gps_open_timer = NULL; 			// GPS延时打开定时

osel_etimer_t gps_protect_timer;        //*< 该定时器用于GPS故障发现定时
osel_etimer_t gps_wait_for_open_timer;  //*< 该定时器用于GPS故障处理定时
osel_etimer_t gps_open_timer;           //*< 该定时器用于GPS延时打开定时

static osel_task_t *gps_task = NULL;
static osel_event_t gps_task_event_store[10];
PROCESS(gps_task_thread_process, "gps task thread process");

static gps_data_t gps_data;                     // GPS接收
static uint8_t gps_status = GPS_CLOSE;			// GPS模式
static uint8_t gps_data_cnt = 0;                // 记录接收到完整的GPS数据
static bool_t gps_restore = FALSE;              // 标识是否在GPS异常恢复阶段
static uint8_t gps_delay_cmd = GPS_NULL;        // 模块缓冲期间缓存最后的指令

static uint8_t gps_recv_array[GPS_RECV_LEN_MAX];
gps_simple_info_t gps_simple_info;       // 简单GPS信息:经纬度+速度  

//static uint8_t gps_temp_data[] = {"$GPRMC,094330.000,A,3113.3156,N,12121.2686,E,0.51,193.93,171210,,,A*68\r"};

#if GPS_PROTECT_DEBUG
static bool_t flag = TRUE;
static uint8_t count = 0;
#endif
//设定定时器
static void gps_open_timer_start(void)
{
    osel_etimer_arm(&gps_open_timer, (GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS), 0);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS
}

static void gps_wait_for_open_timer_start(void)
{
    osel_etimer_arm(&gps_wait_for_open_timer, (GPS_WAIT_FOR_OPEN_TIME*1000/OSEL_TICK_PER_MS), 0);
}

static void gps_protect_timer_start(void)
{
    osel_etimer_arm(&gps_protect_timer, (GPS_PROTECT_TIMER_PERIOD*1000/OSEL_TICK_PER_MS), 0);
}
//取消定时器
static void gps_open_timer_close(void)
{
    osel_etimer_disarm(&gps_open_timer);
}

static void gps_wait_for_open_timer_close(void)
{
    osel_etimer_disarm(&gps_wait_for_open_timer);
}

static void gps_protect_timer_close(void)
{
    osel_etimer_disarm(&gps_protect_timer);
}

// 十六进制数转化为ASCII码
STATIC uint8_t hex2char(uint8_t value_hex)
{
    uint8_t value_char = 0;
    if (value_hex <= 9)
    {
        value_char = '0' + value_hex;
    }
    else if ((value_hex >= 10) && (value_hex <= 15))
    {
        value_char = 'A' + value_hex - 10;
    }
    return value_char;
}

/*
** @change from char to hex
*/
STATIC uint8_t char2hex(uint8_t v_char)
{
    uint8_t v_hex = 0;
    if ((v_char >= '0') && (v_char <= '9'))
    {
        v_hex = v_char - '0';
    }
    else if ((v_char >= 'A') && (v_char <= 'F'))
    {
        v_hex = v_char - 'A' + 10;
    }
	else if ((v_char >= 'a') && (v_char <= 'f'))
	{
		v_hex = v_char - 'a' + 10;
	}
    return v_hex;
}

// 计算异或和，GPS校验中使用
STATIC uint8_t crc8(uint8_t *p, uint8_t len)
{
    uint8_t crc = 0;
    if (p != NULL && len > 0 && len < 128)
    {
        for (uint8_t i = 0; i < len; i++)
        {
            crc ^= *p++;
        }
    }
    return crc;
}

/**
 *
 * @brief 2字节整数打印，类似于printf("%d", num)
 *
 * @param[in] num 被格式化的2字节整数
 * @param[in] *p 字符串首地址
 *
 * @return 格式化字符串长度
 */
STATIC uint8_t get_median(uint16_t num, uint8_t *p)
{
    uint8_t len = 0;
    if (num != 0)
    {
        if (num / 10000 != 0)
        {
            len = 5;
        }
        else if (num < 10000 && num >= 1000)
        {
            len = 4;
        }
        else if (num < 1000 && num >= 100)
        {
            len = 3;
        }
        else if (num < 100 && num >= 10)
        {
            len = 2;
        }
        else if (num < 10)
        {
            len = 1;
        }
        
        for (uint8_t i = 0; i < len; i++)
        {
            *p++ = num % 10;
            num /= 10;
        }
    }
    return len;
}

// SIM928A模块使用PMTK指令配置
// 1,输出数据类型仅RMC数据，可设置周期（单位s）
static void gps_cfg_nmea_output(const uint16_t period)
{
    // 取出周期的各位数值
    DBG_ASSERT(period != 0 __DBG_LINE);
    uint8_t median[5] = {0, 0, 0, 0, 0};
    uint8_t median_len = get_median(period, &median[0]);
    
    DBG_ASSERT(median_len > 0 __DBG_LINE);
    DBG_ASSERT(median_len <= 5 __DBG_LINE);
    
    // 命令帧帧头部分
    uint8_t cmd[GPS_CMD_LEN_MAX] = "$PMTK314,0,";
    uint8_t len = 11;
    
    // RMC周期部分
    uint8_t i;
    for (i=0; i<median_len; i++)
    {
        cmd[len] = hex2char(median[median_len-i-1]);
        len ++;
    }
    
    // 剩余部分
    uint8_t cmd_add[] = ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
    for (i=0; i<34; i++)
    {
        cmd[len] = cmd_add[i];
        len ++;
    }
    
    uint8_t crc = crc8(&cmd[1], len - 1);
    uint8_t crc_high = (uint8_t)(crc>>4);
    uint8_t crc_low  = (uint8_t)(crc&0x0F);
    crc_high = hex2char(crc_high);
    crc_low  = hex2char(crc_low);
    
    cmd[len] = '*';
    len ++;
    cmd[len] = crc_high;
    len ++;
    cmd[len] = crc_low;
    len ++;
    cmd[len] = GPS_CR;
    len ++;
    cmd[len] = GPS_LF;
    len ++;
    
    hal_uart_send_string(GPS_UART, &cmd[0], len);
}

static void gps_wait_for_open_handler(void)
{
    if ((gps_status==GPS_OPEN) && (gps_restore==TRUE))
	{
		gps_open_intra();
	}
}

static void gps_protect_timer_handler(void)
{
#if GPS_DEBUG
	serial_write(HAL_UART_1, "GPS_PROTECT_TIMER_HANDLER\r\n", 27);
#endif
	
	if (gps_data_cnt == 0)
	{
#if GPS_DEBUG_INFO == 1
        uint8_t debug_data_read[150];
        uint8_t debug_read_len = 0;
        uint8_t gie_status = 0;
        uint8_t uart_status = 0;
        
		gie_status = (__get_SR_register()&GIE)>>3;
		uart_status = UCA2IE&UCRXIE;
		debug_read_len = 0;
		debug_read_len = serial_read(GPS_UART, &debug_data_read[0], 150);  
        
        gps_simple_info.latitude = (gie_status*128 + uart_status);
		gps_simple_info.longitude = (gps_status*128 + debug_read_len);
		gps_simple_info.speed = 11.0;
		
		if (gps_data.cb != NULL)
		{
			gps_data.cb(gps_simple_info);
		}
		gps_cmd_cnt = 0;
#endif
		
		if (gps_status == GPS_OPEN) // 在恢复之前已经被关闭则不处理
		{
            gps_restore = TRUE;
            gps_close_intra();
            
            //启动等待GPS故障处理定时器
            gps_wait_for_open_timer_start();
		}
	}
	else
	{
		gps_data_cnt = 0;
		
		if (gps_status == GPS_OPEN)
		{
			gps_protect_timer_start();
		}
	}
}

static bool_t gps_rmc_crc(uint8_t *p_data, uint8_t len)
{
	DBG_ASSERT(p_data != NULL __DBG_LINE);
	
	uint8_t crc;
	uint8_t high;
	uint8_t low;
	bool_t flag = FALSE;
	
	if ((len < GPS_RMC_LEN_MIN) || (len > GPS_RECV_LEN_MAX))
	{
		return flag;
	}

	if ((len > (GPS_RMC_LEN_MIN + 4)) && (p_data[len-2] == GPS_CR) && (p_data[len-5] == GPS_CRC_PROMPT))
	{
		crc	= crc8(&p_data[1], len-6);
		high = hex2char((uint8_t)(crc>>4));
		low  = hex2char((uint8_t)(crc&0x0F));
		
		if ((high == p_data[len-4]) && (low == p_data[len-3]))
		{
			flag = TRUE;
		}
	}
	return flag;
}

fp64_t get_rmc_value(uint8_t *p_data, uint8_t type)
{
	DBG_ASSERT(p_data != NULL __DBG_LINE);
	
	location_t local;
	fp64_t value = 0.0;
	
	if ((type < GPS_DATA_TYPE_LAT) || (type > GPS_DATA_TYPE_LON))
	{
		return value;
	}
	
	switch (type)
	{
	case GPS_DATA_TYPE_LAT:
		local.deg = char2hex(p_data[0])*10 
			+ char2hex(p_data[1]);
		local.min_dec = char2hex(p_data[2])*10 
			+ char2hex(p_data[3]);
		local.min_fract = char2hex(p_data[5])*1000 
			+ char2hex(p_data[6])*100
			+ char2hex(p_data[7])*10 
			+ char2hex(p_data[8]);
		value = local.deg*100 + local.min_dec + ((fp64_t)local.min_fract)/10000;
		break;
		
	case GPS_DATA_TYPE_LON:
		local.deg = char2hex(p_data[0])*100 
			+ char2hex(p_data[1])*10 
			+ char2hex(p_data[2]);
		local.min_dec = char2hex(p_data[3])*10 
			+ char2hex(p_data[4]);
		local.min_fract = char2hex(p_data[6])*1000 
			+ char2hex(p_data[7])*100
			+ char2hex(p_data[8])*10 
			+ char2hex(p_data[9]);
		value = local.deg*100 + local.min_dec + ((fp64_t)local.min_fract)/10000;
		break;
	}
	return value;
}

/**
 *
 * @brief 从GPRMC中获得设备当前纬度和经度结果
 *
 * @param[in] data GPRMC nmea语句字符串指针
 * @param[in] len GPRMC nmea语句长度
 *
 * @return 定位是否成功
 */
STATIC bool_t gps_parse_rmc(uint8_t *data, uint8_t len, gps_simple_info_t *info)
{
	DBG_ASSERT(data != NULL __DBG_LINE);
	
	bool_t result = FALSE;
	if ((len < GPS_RMC_LEN_MIN) || (len > GPS_RECV_LEN_MAX))
	{
		return result;
	}
	
	if (gps_rmc_crc(data, len))
	{
		uint8_t num = 0;
		uint8_t i = 0;
		uint8_t blank[GPS_RMC_BLANK_NUM] = {0, };
		
        // 计算逗号的个数
		while (i < len)
		{
			if (data[i] == ',')
			{
				blank[num] = i;
				num++;
			}
			i++;
		}
		
		if (num == GPS_RMC_BLANK_NUM)
		{
			uint8_t status = data[blank[1] + 1];
			
			if (status == 'A')
			{
                /* 定位成功 */
				if ( (blank[2] + GPS_LAT_LEN + 1 == blank[3]) 
					&& (blank[4] + GPS_LON_LEN + 1 == blank[5]) )
				{
					info->latitude = get_rmc_value(&data[blank[2] + 1], GPS_DATA_TYPE_LAT);
					info->longitude = get_rmc_value(&data[blank[4] + 1], GPS_DATA_TYPE_LON);
					info->speed = 0.00;
					result = TRUE;
				}
			}
			else if (status == 'V')
			{
                /* 定位不成功 */				
				info->latitude = 0.00;
				info->longitude = 0.00;
				info->speed = 0.00;
				result = TRUE;
			}
			else
			{
				;
			}
		}
	}
	
	return result;
}

static void gps_uart_handle(void)
{
    uint8_t frame_len = 0;
    uint8_t read_data = 0;
    osel_memset(gps_recv_array, 0x00, sizeof(gps_recv_array));

    while (serial_read(GPS_UART, &read_data, sizeof(uint8_t)))
    {
        gps_recv_array[frame_len++] = read_data;
        if (read_data == GPS_LF)
        {
            break;
        }
		
		if (frame_len >= GPS_CMD_LEN_MAX)
		{
			frame_len = 0;
			break;
		}
    }
	
	serial_clear(GPS_UART);
	
#if GPS_DATA_DEBUG
	for (uint8_t i=0; i<frame_len; i++)
	{
		printf("%c", gps_recv_array[i]);
	}
#endif

#if 0

	memcpy((void*)gps_recv_array,(void*)gps_temp_data,sizeof(gps_temp_data));

	frame_len = sizeof(gps_temp_data);

	NOP();
	NOP();

#endif


    gps_simple_info_t info;
	// 接收到有效GPS数据
	if (gps_parse_rmc(gps_recv_array, frame_len, &info))
	{
		// 程序经过修改，把info结果赋值到全部变量 gps_simple_info中
        memcpy((void*)&gps_simple_info, (void*)&info, sizeof(gps_simple_info_t));
        gps_data_cnt++;
		
		if (gps_data.cb != NULL)
		{
#if GPS_DEBUG
			serial_write(HAL_UART_1, "GPS_DATA\r\n", 10);
#endif
			
#if GPS_PROTECT_DEBUG
			count++;
			printf("count=%d\r\n",count);
			if (count%7 == 0)
			{
				hal_uart_recv_disable(GPS_UART);
			}
#endif
			
#if GPS_DATA_DEBUG
			printf("lat=%f lon=%f speed=%f\r\n", gps_simple_info.latitude, gps_simple_info.longitude, gps_simple_info.speed);
#endif
			gps_data.cb(gps_simple_info);
		}
	}
}

static void gps_serial_cb(void)
{
    osel_event_t event;
//    osel_post(GPS_UART_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
    event.sig = GPS_UART_EVENT;
    event.param = NULL;
    osel_post(NULL, &gps_task_thread_process, &event);    
}

static void gps_uart_interface_config(void)
{
    serial_reg_t gps_serial_reg;

    gps_serial_reg.sd.valid = TRUE;
    gps_serial_reg.sd.len = 6;
    gps_serial_reg.sd.pos = 0;
    gps_serial_reg.sd.data[0] = '$';
	gps_serial_reg.sd.data[1] = 'G';
	gps_serial_reg.sd.data[2] = 'P';
	gps_serial_reg.sd.data[3] = 'R';
	gps_serial_reg.sd.data[4] = 'M';
	gps_serial_reg.sd.data[5] = 'C';

    gps_serial_reg.ld.valid = FALSE;

    gps_serial_reg.argu.len_max = 100;
    gps_serial_reg.argu.len_min = 2;

    gps_serial_reg.ed.valid = TRUE;
    gps_serial_reg.ed.len = 1;
    gps_serial_reg.ed.data[0] = GPS_LF;   	// 'LF', 换行符

    gps_serial_reg.echo_en = FALSE;
    gps_serial_reg.func_ptr = gps_serial_cb;

    serial_fsm_init(GPS_UART);
    serial_reg(GPS_UART, gps_serial_reg);
}

static void gps_power_init(void)
{
	P1SEL &=~BIT4;
	P1DIR |= BIT4;
	P1OUT &= ~BIT4;
}

static void gps_power_on(void)
{
	P1OUT |= BIT4;
}

static void gps_power_off(void)
{
	P1OUT &= ~BIT4;
}

static void gps_close_intra(void)
{
    gps_power_off();		
    serial_clear(GPS_UART);		
    hal_uart_recv_disable(GPS_UART);
    gps_open_timer_close();
    gps_wait_for_open_timer_close();
    gps_protect_timer_close();
    
//    if (gps_protect_timer != NULL)
//    {
//        hal_timer_cancel(&gps_protect_timer);
//    }
//    
//    if (gps_wait_for_open_timer != NULL)
//    {
//        hal_timer_cancel(&gps_wait_for_open_timer);
//    }
//    
//    if (gps_open_timer != NULL)
//    {
//        hal_timer_cancel(&gps_open_timer);
//    }
}

void gps_close(void)
{
#if GPS_DEBUG
	serial_write(HAL_UART_1, "GPS_CLOSE\r\n", 11);
#endif

    if (gps_restore == TRUE)
    {
        gps_delay_cmd = GPS_CLOSE;
    }
    else if (gps_status == GPS_OPEN)
    {
        gps_status = GPS_CLOSE;	//!需放在定时器取消之前，回调中会使用该标志位       
        gps_power_off();		
		serial_clear(GPS_UART);		
		hal_uart_recv_disable(GPS_UART);
        gps_open_timer_close();
        gps_wait_for_open_timer_close();
        gps_protect_timer_close();        
        
//        if (gps_protect_timer != NULL)
//        {
//            hal_timer_cancel(&gps_protect_timer);
//        }
//        
//        if (gps_wait_for_open_timer != NULL)
//        {
//            hal_timer_cancel(&gps_wait_for_open_timer);
//        }
//        
//        if (gps_open_timer != NULL)
//        {
//            hal_timer_cancel(&gps_open_timer);
//        }        
    }
}

void gps_sleep(void)
{
#if GPS_DEBUG
	serial_write(HAL_UART_1, "GPS_CLOSE\r\n", 11);
#endif

    if (gps_restore == TRUE)
    {
        gps_delay_cmd = GPS_SLEEP;
    }
    else if (gps_status == GPS_OPEN)
    {
//        gps_status = GPS_SLEEP;	//!需放在定时器取消之前，回调中会使用该标志位       
//		hal_uart_send_string(GPS_UART, "", len);
//		serial_clear(GPS_UART);		
//        gps_open_timer_close();
//        gps_wait_for_open_timer_close();
//        gps_protect_timer_close();            
    }
}


static void gps_uart_cfg(void)
{
    hal_uart_init(GPS_UART, 9600);
    hal_uart_recv_enable(GPS_UART);
    gps_cfg_nmea_output(GPS_DATA_PERIOD);
    serial_clear(GPS_UART);
}

static void gps_open_handler(void)
{
    if (gps_restore == TRUE)     
    {
#if GPS_DEBUG_INFO == 1	
		gps_simple_info.latitude = gps_cmd_cnt + 1;
		gps_simple_info.longitude = (gps_status*128 + gps_delay_cmd + acc_status_flag);
		gps_simple_info.speed = 11.0;
		gps_cmd_cnt = 0;
        acc_status_flag = 0;
		
		if (gps_data.cb != NULL)
		{
			gps_data.cb(gps_simple_info);
		}
#endif
		
        gps_restore = FALSE;
        if (gps_delay_cmd == GPS_CLOSE)
        {
            gps_close();//模拟外部调用，需要改变状态，不使用内部接口。
        }
        else
        {
           /*GPS保护结束后默认是OPEN状态，所以非关闭缓存指令只需配置串口即可，状态保持OPEN态*/
            gps_uart_cfg();
            gps_protect_timer_start();
        }
        
        gps_delay_cmd = GPS_NULL;

    }
    else if (gps_status == GPS_OPEN)
    {
        gps_uart_cfg();
        gps_protect_timer_start();
    }  
}

static void gps_open_intra(void)
{
    gps_power_on();
    gps_open_timer_start();
}

void gps_open(void)
{
#if GPS_DEBUG
	serial_write(HAL_UART_1, "GPS_OPEN\r\n", 10);
#endif
	
    if (gps_restore == TRUE)
    {
        gps_delay_cmd = GPS_OPEN;
    }
    else if (gps_status==GPS_CLOSE)
	{
		gps_status = GPS_OPEN;
		
		gps_power_on();
        //打开gps电源
        gps_open_timer_start();
	}
}

//void gps_task(void *e)
//{
//    DBG_ASSERT(e != OSEL_NULL __DBG_LINE);
//    switch ( ((event_block_t *)e)->sig )
//    {        
//    case GPS_UART_EVENT:
//        gps_uart_handle();
//        break;
//			
//	case GPS_OPEN_EVENT:
//		gps_open_handler();
//		break;
//		
//	case GPS_PROTECT_EVENT:
//        gps_protect_timer_handler();
//        break;
//		
//	case GPS_WAIT_FOR_OPEN_EVENT:
//		gps_wait_for_open_handler();
//		break;
//		
//    default:
//        break;
//        
//    }
//}

PROCESS_THREAD(gps_task_thread_process,ev,data)
{
    PROCESS_BEGIN();
    while (1)
    {
        if (GPS_UART_EVENT == ev)
        {
            gps_uart_handle();
        }
        else if (GPS_OPEN_EVENT == ev)
        {
            gps_open_handler();
        }
        else if (GPS_PROTECT_EVENT == ev)
        {
            gps_protect_timer_handler();
        }
        else if (GPS_WAIT_FOR_OPEN_EVENT == ev)
        {
            gps_wait_for_open_handler();
        }
        
        PROCESS_YIELD();     //*< 释放线程控制权，进行任务切换
    }
    PROCESS_END();
}


void gps_init(gps_cb_t gps_cb)
{	
	DBG_ASSERT(gps_cb != NULL __DBG_LINE);
	if (gps_cb != NULL)
	{
		gps_data.cb = gps_cb;
	}
	gps_power_init();	
    gps_uart_interface_config();
    
#if 0	
    osel_task_tcb *gps_task_handle = osel_task_create(&gps_task, GPS_TASK_PRIO);
	
    osel_subscribe(gps_task_handle, GPS_UART_EVENT);
	osel_subscribe(gps_task_handle, GPS_OPEN_EVENT);

	osel_subscribe(gps_task_handle, GPS_WAIT_FOR_OPEN_EVENT);
	osel_subscribe(gps_task_handle, GPS_PROTECT_EVENT);
#endif

    gps_task = osel_task_create(NULL,GPS_TASK_PRIO,gps_task_event_store,10);
    osel_pthread_create(gps_task,&gps_task_thread_process,NULL);
	osel_etimer_ctor(&gps_open_timer,&gps_task_thread_process,GPS_OPEN_EVENT,NULL);
    osel_etimer_ctor(&gps_wait_for_open_timer,&gps_task_thread_process,GPS_WAIT_FOR_OPEN_EVENT,NULL);
    osel_etimer_ctor(&gps_protect_timer,&gps_task_thread_process,GPS_PROTECT_EVENT,NULL);
}
