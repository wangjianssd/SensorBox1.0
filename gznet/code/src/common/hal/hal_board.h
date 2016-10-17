/**
 * @brief       : 
 *
 * @file        : hal_board.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */


#ifndef __HAL_BOARD_H
#define __HAL_BOARD_H
#include "common/lib/data_type_def.h"
#include "platform/platform.h"

#define HAL_LED_BLUE          0x01
#define HAL_LED_RED           0x02
#define HAL_LED_GREEN         0x03
#define HAL_LED_POWER         0x04
#define HAL_LED_ERROR         0x05

#define DEVICE_INFO_ADDR                (INFO_B_ADDR)

#pragma pack(1)
typedef struct
{
    uint8_t device_id[8];
    uint8_t device_ch[4];
	uint8_t license[22];
    uint8_t power_sn;
    uint8_t intra_ch_cnt;
    uint8_t intra_data_simu;        // sensor data simulate or real
    uint32_t heartbeat_cycle;       // heartbeat cycle(s)
    int16_t offset_z;
    uint8_t profile_id[4];  //profile ID
    uint8_t profile_md5[16];  //profile MD5
    uint32_t box_activation;	// 感知箱是否激活
	uint32_t box_app_id;			// 感知箱用户ID
	uint32_t box_cfg_timer; 		// 感知箱配置时间戳
    uint32_t heartbeat_interval_in_room; //在仓库中的心跳间隔
    uint32_t heartbeat_interval_out_room; //用户使用时心跳间隔
    uint32_t location_info_interval_in_room;//仓库中的位置信息上报间隔
    uint32_t location_info_interval_out_room;//用户使用时位置信息上报间隔
    uint8_t  temperature_threshold;     //温度阈值
    uint8_t  humidity_threshold;        //湿度阈值
	uint16_t acceleration_threshold;	// 加速度阀值
	uint16_t illumination_threshold;	// 光照阀值	       	
} device_info_t;
#pragma pack()

extern device_info_t device_info;


/**
* 获取设备短地址
*
* @param: 无
*
* @return: 设备短地址
*/
uint16_t hal_board_get_device_short_addr(void);

/**
* 获取信道
*
* @param: 信道索引
*
* @return: 信道
*/
uint8_t hal_board_get_device_ch(uint8_t index);

/**
* 系统重启
*
* @param: 无
*
* @return: 无
*/
void hal_board_reset(void);

/**
* LED初始化管脚配置
*
* @param: 无
*
* @return: 无
*/
void hal_led_init(void);

/**
* 点亮LED
*
* @param color: 需要点亮的LED的颜色
*
* @return: 无
*/
void hal_led_open(uint8_t color);

/**
* 熄灭LED
*
* @param color: 需要熄灭的LED的颜色
*
* @return: 无
*/
void hal_led_close(uint8_t color);

/**
* LED反转(亮-->灭  或  灭-->亮)
*
* @param color: 需要反转的LED的颜色
*
* @return: 无
*/
void hal_led_toggle(uint8_t color);

device_info_t hal_board_info_look(void);

/**
* 保存设备信息
*
* @param: p_info: 新的设备信息的地址
*
* @return: 是否保存成功
*/
bool_t hal_board_save_device_info(uint8_t len, uint8_t *p_info);

/**
* 板级初始化，初始化系统时钟，外围设备等
*
* @param: 无
*
* @return: 无
*/
void hal_board_init(void);

/**
* 获取设备的短ID
*
* @param: 设备ID
*
* @return: 无
*/
uint16_t hal_get_device_short_addr(const uint8_t *device_id);

/**
* License 验证
*
* @param:  无
*
* @return: 无
*/
bool_t hal_license_verification();

device_info_t hal_board_info_get(void);
bool_t hal_ascii_to_hex(uint8_t hi, uint8_t lo, uint8_t *hex);
bool_t hal_hex_to_ascii(uint8_t *buf, uint8_t dat);
bool_t hal_board_info_save(device_info_t *p_info, bool_t flag);
bool_t hal_board_info_delay_save(void);
#endif
