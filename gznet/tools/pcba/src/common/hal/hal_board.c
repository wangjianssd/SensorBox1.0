/**
* @brief       : 
*
* @file        : hal_board.c
* @author      : gang.cheng
*
* Version      : v0.0.1
* Date         : 2015/5/7
* Change Logs  :
*
* Date        Version      Author      Notes
* 2015/5/7    v0.0.1      gang.cheng    first version
*/

#include <driver.h>
#include <hal.h>
#include <platform.h>

device_info_t device_info;
static bool_t delay_save_flag = FALSE;
void hal_led_init(void)
{
	led_init();
}

void hal_led_open(uint8_t color)
{
    switch(color)
    {
    case HAL_LED_BLUE:
        LED_OPEN(BLUE);
        break;
    case HAL_LED_RED:
        LED_OPEN(RED);
        break;
    case HAL_LED_GREEN:
        LED_OPEN(GREEN);
        break;
    case HAL_LED_POWER:
        LED_OPEN(RED);
        break;
    case HAL_LED_ERROR:
        LED_OPEN(BLUE);
        break;
    default :
        break;
    }
}

void hal_led_close(uint8_t color)
{
    switch(color)
    {
    case HAL_LED_BLUE:
        LED_CLOSE(BLUE);
        break;
    case HAL_LED_RED:
        LED_CLOSE(RED);
        break;
    case HAL_LED_GREEN:
        LED_CLOSE(GREEN);
        break;
    case HAL_LED_POWER:
        LED_CLOSE(RED);
        break;
    case HAL_LED_ERROR:
        LED_CLOSE(BLUE);
        break;
    default :
        break;
    }
}

void hal_led_toggle(uint8_t color)
{
    switch(color)
    {
    case HAL_LED_BLUE:
        LED_TOGGLE(BLUE);
        break;
    case HAL_LED_RED:
        LED_TOGGLE(RED);
        break;
    case HAL_LED_GREEN:
        LED_TOGGLE(GREEN);
        break;
    case HAL_LED_POWER:
        LED_TOGGLE(RED);
        break;
    case HAL_LED_ERROR:
        LED_TOGGLE(BLUE);
        break;
    default :
        break;
    }
}

void hal_board_reset(void)
{
	board_reset();
}

static void hal_board_info_init(void)
{
	hal_nvmem_read((uint8_t *)DEVICE_INFO_ADDR, (uint8_t *)&device_info, sizeof(device_info));
    //@note 兼容地磁二代，默认有4个信道可选
    device_info.intra_ch_cnt = 2;
    
#if DEBUG_DEVICE_INFO_EN > 0
    osel_memset(&device_info.device_id,0,sizeof(device_info_t));
    device_info.device_id[0] = LO_UINT16(NODE_ID);
    device_info.device_id[1] = HI_UINT16(NODE_ID);
    
    device_info.heartbeat_cycle = APP_DATA_TRAN_PERIOD;
    uint8_t license[]={'P','-','I','T','C','S','1','3','0','4','9','I','T','C','S','M','1','0','7','A','0','E'};
    osel_memcpy(device_info.license, license, 22);
    
    device_info.device_ch[0] = CH_SN;
    device_info.intra_ch_cnt = 1;
#if NODE_TYPE == NODE_TYPE_DETECTOR    
    if(ALGORITHM_MODE_TYPE == 0x03)
    {
        device_info.function_type = 0x03; // DETECT_TYPE_CAR_LANE_SIMULATION
    }
#endif   
    
#endif
    
#if NODE_TYPE == NODE_TYPE_DETECTOR
    if(device_info.function_type == 0x03)
    {
        
    }
    else
    {
        switch(device_info.license[15])
        {
        case 'M'://流量
            device_info.function_type = 0;
            break;
        case 'S'://速度
            device_info.function_type = 1;
            break;
        default:
            device_info.function_type = 0;
            break;
        }
    }
#endif
}

bool_t hal_hex_to_ascii(uint8_t *buf, uint8_t dat)
{
    uint8_t dat_buff;
    
    dat_buff = dat;
    dat = dat & 0x0f;
    if (dat <= 9)
    {
        dat += 0x30;
    }
    else
    {
        dat += 0x37;
    }
    
    buf[1] = dat;
    
    dat = dat_buff;
    dat >>= 4;
    dat = dat & 0x0f;
    if (dat <= 9)
    {
        dat += 0x30;
    }
    else
    {
        dat += 0x37;
    }
    
    buf[0] = dat;
    
    return TRUE;
}

bool_t hal_ascii_to_hex(uint8_t hi, uint8_t lo, uint8_t *hex)
{
    *hex = 0;
    if ((hi >= 0x30) && (hi <= 0x39))
    {
        hi -= 0x30;
    }
    else if ((hi >= 0x41) && (hi <= 0x46))
    {
        hi -= 0x37;
    }
    else
    {
        return FALSE;
    }
    *hex |= (hi << 4);
    
    if ((lo >= 0x30) && (lo <= 0x39))
    {
        lo -= 0x30;
    }
    else if ((lo >= 0x41) && (lo <= 0x46))
    {
        lo -= 0x37;
    }
    else
    {
        return FALSE;
    }
    *hex |= lo;
    
    return TRUE;
}


uint16_t hal_get_device_short_addr(const uint8_t *device_id)
{
    uint16_t short_id;
    uint16_t serial = BUILD_UINT16(device_id[0],device_id[1]);
    uint16_t batch_day = device_id[2];
    
    short_id = (serial & 0x03FF) | (batch_day << 10); // 取流水号10bits和批次6bits
    return short_id;
}


uint16_t hal_board_get_device_short_addr(void)
{
    return hal_get_device_short_addr(&device_info.device_id[0]);
}

uint8_t hal_board_get_device_ch(uint8_t index)
{
    return device_info.device_ch[index];
}

bool_t hal_license_verification()
{
    uint16_t key = 0;
    uint16_t key_verification = 0;
    uint16_t version = 0;
    version = (uint16_t)(device_info.license[16]<<8) +device_info.license[17];
    uint8_t key1 = 0;
    uint8_t key2 = 0;
    hal_ascii_to_hex(device_info.license[18],device_info.license[19],&key1);
    hal_ascii_to_hex(device_info.license[20],device_info.license[21],&key2);
    key_verification = (uint16_t)(key1<<8) + key2;
    if(version == 0x3130)
    {
        for (int i = 0; i < 9;i++ )
        {
            key = key << 1;
            key = key ^ (((uint16_t)device_info.license[i * 2] << 8) + device_info.license[i * 2 + 1]);
        }
        if(key == key_verification)
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
    return FALSE;
}

bool_t hal_board_info_save(device_info_t *p_info, bool_t flag)
{
    osel_memcpy((uint8_t *)&device_info, (uint8_t *)p_info, sizeof(device_info_t));
    
    if (flag)
    {
        hal_nvmem_erase((uint8_t *)DEVICE_INFO_ADDR, 0);
        hal_nvmem_write((uint8_t *)DEVICE_INFO_ADDR,
                        (uint8_t *)p_info, sizeof(device_info_t));
    }
    else
    {
        delay_save_flag = TRUE;
    }
    
    return TRUE;
}

bool_t hal_board_info_delay_save(void)
{
    if (delay_save_flag)
    {
        delay_save_flag = FALSE;
        hal_nvmem_erase((uint8_t *)DEVICE_INFO_ADDR, 0);
        hal_nvmem_write((uint8_t *)DEVICE_INFO_ADDR,
                        (uint8_t *)&device_info, sizeof(device_info_t));
    }
    
    return TRUE;
}

device_info_t hal_board_info_get(void)
{
    hal_nvmem_read((uint8_t *)DEVICE_INFO_ADDR,
                   (uint8_t *)&device_info, sizeof(device_info));
    
    return device_info;
}

device_info_t hal_board_info_look(void)
{
    return device_info;
}

bool_t hal_board_save_device_info(uint8_t len, uint8_t *p_info)
{
    DBG_ASSERT(p_info != NULL __DBG_LINE);
    if(p_info == NULL)
    {
        return FALSE;
    }
    hal_nvmem_erase((uint8_t *)DEVICE_INFO_ADDR, 0);
    hal_nvmem_write((uint8_t *)DEVICE_INFO_ADDR, p_info, len);
    return TRUE;
}

void hal_board_init(void)
{
    board_init();
	hal_gpio_init();
    hal_led_init();
    
	hal_clk_init(SYSCLK_8MHZ);
	hal_board_info_init();
	hal_wdt_clear(10);
	hal_uart_init(HAL_UART_1, 9600); 

	hal_timer_init();
	
#if (NODE_TYPE == NODE_TYPE_DETECTOR)
//	uint8_t sensor_init_num = 0;
//	hal_led_close(HAL_LED_GREEN);
//	while (FALSE == hal_sensor_init())
//    {
//        if(sensor_init_num ++ >= 10)
//        {
//            break;
//        }
//    }
#endif
    debug_init(DBG_LEVEL_TRACE | DBG_LEVEL_ORIGIN | DBG_LEVEL_INFO);
}
