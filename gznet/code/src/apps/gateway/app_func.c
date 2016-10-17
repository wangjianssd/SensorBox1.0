/**
 * @brief       : 
 *
 * @file        : app_func.c
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/12/31
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/12/31    v0.0.1      WangJifang    some notes
 */
#include "apps/gateway/app.h"
#include "apps/gateway/app_func.h"


static volatile bool_t cnn_state = FALSE; //platform cnn state
#define REGISTER_TIME   (60u)                       //注册间隔(秒)
#define AT_START        "AT"            //!< 命令的起始字符串
#define CMD_LINK_CHAR   "&"             //!< 命令间隔连接字符
#define CMD_DEV_ID_TYPE  "ID"        //!<DEVICE ID的配置指令
#define CMD_IP_TYPE  	"IP"
#define AT_OK           "OK\r\n"        //!< 指令输入成功以后的返回

void reverse_id_endian(uint8_t *id)
{
    for (uint8_t i = 0, j = 7; i < j; i++, j--)
    {
        uint8_t temp;
        temp = id[i];
        id[i] = id[j];
        id[j] = temp;
    }
}

static void north_frame_cb(void)
{
    osel_event_t event;
	event.sig = APP_UART_EVENT;
	event.param = NULL;
	osel_post(NULL, &app_process, &event);
    
//    osel_post(APP_UART_EVENT, NULL, OSEL_EVENT_PRIO_LOW);
}

void uart_frame_cfg(void)
{
	serial_reg_t app_serial_reg;
	
    app_serial_reg.sd.valid = TRUE;
    app_serial_reg.sd.len = 2;
    app_serial_reg.sd.pos = 0;
    app_serial_reg.sd.data[0] = 'A';
    app_serial_reg.sd.data[1] = 'T';
	
    app_serial_reg.ld.valid = FALSE;
	
    app_serial_reg.argu.len_max = UART_LEN_MAX;
    app_serial_reg.argu.len_min = 4;
	
    app_serial_reg.ed.valid = TRUE;
    app_serial_reg.ed.len = 1;
    app_serial_reg.ed.data[0] = 0x0D;   // 'CR', enter
	
    app_serial_reg.echo_en = FALSE;
    app_serial_reg.func_ptr = north_frame_cb;
	
    serial_fsm_init(SERIAL_1);
    serial_reg(SERIAL_1, app_serial_reg);
	
}

bool_t platform_state(void)
{
    return cnn_state;
}

void cnn_platform(void)
{
    cnn_state = TRUE;
}

void discnn_platform(void)
{
    cnn_state = FALSE;
}

uint8_t converthexchar(uint8_t ch)
{
    if ( (ch >= '0') && (ch <= '9'))
    {
        return (ch - 0x30);
    }
    else if ((ch >= 'A') && (ch <= 'F'))
    {
        return (ch - 'A' + 10);
    }
    else if ((ch >= 'a') && (ch <= 'f'))
    {
        return (ch - 'a' + 10);
    }
    else
    {
        return -1;
    }
}

void auto_dev_id_set(uint8_t const * const buf)
{
    device_info_t device_info = hal_board_info_get();
    uint8_t buf_temp_1[16] = {0x00};
    uint8_t buf_temp_2[8] = {0x00};
    osel_memcpy(buf_temp_1, buf, 2*sizeof(uint64_t));
    
    for (uint8_t i = 0;i < 16;i++)
    {
        buf_temp_1[i] = converthexchar(buf[i]);
    }
	
    buf_temp_2[7] = (buf_temp_1[0] << 4) + buf_temp_1[1];
    buf_temp_2[6] = (buf_temp_1[2] << 4) + buf_temp_1[3];
    buf_temp_2[5] = (buf_temp_1[4] << 4) + buf_temp_1[5];
    buf_temp_2[4] = (buf_temp_1[6] << 4) + buf_temp_1[7];
    buf_temp_2[3] = (buf_temp_1[8] << 4) + buf_temp_1[9];
    buf_temp_2[2] = (buf_temp_1[10] << 4) + buf_temp_1[11];
    buf_temp_2[1] = (buf_temp_1[12] << 4) + buf_temp_1[13];
    buf_temp_2[0] = (buf_temp_1[14] << 4) + buf_temp_1[15];
    
    osel_memcpy(device_info.device_id, buf_temp_2, sizeof(uint64_t));
	osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
    hal_board_info_save(&device_info, TRUE);
	serial_write(HAL_UART_1, AT_OK, sizeof(AT_OK) - 1);  
	OSEL_EXIT_CRITICAL(s);
}

static int8_t auto_device_id_cfg_parse(uint8_t const * const buf, uint8_t len)
{
    uint8_t offset = 0;           
    if(osel_memcmp((uint8_t *)&buf[offset], CMD_DEV_ID_TYPE, sizeof(CMD_DEV_ID_TYPE) - 1))
    {
        offset += sizeof(CMD_DEV_ID_TYPE)-1;
        if(!osel_memcmp((uint8_t *)&buf[offset], CMD_LINK_CHAR, sizeof(CMD_LINK_CHAR) - 1))
        {             
            return -1;
        }
        offset += sizeof(CMD_LINK_CHAR)-1;
		
        auto_dev_id_set((uint8_t *)&buf[offset]);         
        hal_board_reset();
        return TRUE;  
    }
    else
    {
        return -3;
    }    
}

void auto_cmd_parse(uint8_t *buf, uint8_t len)
{
	uint8_t offset = 0;
    // AT
    if(!osel_memcmp((uint8_t *)&buf[offset], AT_START, sizeof(AT_START) - 1))
    {
        return;  //!< 
    }
    offset += sizeof(AT_START)-1;   
    // &
    if(!osel_memcmp((uint8_t *)&buf[offset], CMD_LINK_CHAR, sizeof(CMD_LINK_CHAR) - 1))
    {
        return;
    }
    offset += sizeof(CMD_LINK_CHAR)-1;
	
	//判断类型
	if(osel_memcmp((uint8_t *)&buf[offset], CMD_DEV_ID_TYPE, sizeof(CMD_DEV_ID_TYPE) - 1))
	{
		if(auto_device_id_cfg_parse(&buf[offset], len) != -3)return;
	}
	else if(osel_memcmp((uint8_t *)&buf[offset], CMD_IP_TYPE, sizeof(CMD_IP_TYPE) - 1))
	{
		;
	}
}

void uart_event_handle(void)
{
	uint8_t frame_len = 0;
    uint8_t read_data = 0;
    
    static uint8_t cmd_recv_array[50];
    osel_memset(cmd_recv_array, 0x00, sizeof(cmd_recv_array));
    
    while (serial_read(HAL_UART_1, &read_data, sizeof(uint8_t)))
    {
        cmd_recv_array[frame_len++] = read_data;
        if (read_data == 0x0A)
        {
            break;
        }
    }
	auto_cmd_parse(&cmd_recv_array[0], frame_len);
}

bool_t judge_self_id(uint8_t *judge_id)
{
    device_info_t device_info = hal_board_info_look();
    uint8_t id[8] =
    {
        0
    };
    osel_memcpy(&id[0], &device_info.device_id[0], 8);
    reverse_id_endian(&id[0]);
    if (osel_memcmp(&judge_id[0], &id[0], 8))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}