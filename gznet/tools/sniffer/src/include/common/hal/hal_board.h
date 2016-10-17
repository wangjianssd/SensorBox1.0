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
#include <data_type_def.h>
#include <driver.h>

#define HAL_LED_BLUE          0x01
#define HAL_LED_RED           0x02
#define HAL_LED_GREEN         0x03
#define HAL_LED_POWER         0x04
#define HAL_LED_ERROR         0x05

#define DEVICE_INFO_ADDR                (INFO_B_ADDR)

#pragma pack(1)
typedef struct
{
    // 地磁二期通用设备信息配置表
    uint8_t device_id[8];
    uint8_t device_ch[8];

    uint8_t function_type;          // 功能-测流量，测速，电子警察(通过License来识别，此功能没有用了)
    uint8_t semaphore_type;         // 信号机类型    (用不上)
    uint8_t debug_info;             //调试信息开关(0:不启动调试信息 1:调试信息  2:调试信息+数据 3:持续复位)
    uint16_t heartbeat_cycle;       // 心跳周期(秒)
    uint16_t avg_speed_cycle;       // 平均车速周期(秒)
    uint16_t traffic_flow_cycle;    // 车流量周期(秒)
    uint8_t license[22];
    uint8_t alg_mode;               // single or axis
    uint8_t id_filter;              // ID是否过滤
    
    /** 地磁二期是没有这个属性，所以上位机配置的时候会因为device_info_t结构体修改
    写入的数据不正确，所以在初始化的时候需要填写默认的值 */
    
    uint8_t intra_ch_cnt;
    
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
