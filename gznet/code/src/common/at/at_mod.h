#pragma once
#include "common/lib/data_type_def.h"
#include "at_common.h"

/**
* @brief AT模块注册
* @param[in] 串口发送的回调函数
* @param[in] 接收数据的回调函数
* @return	
*/
//void at_mod_init(at_send_cb send_cb, at_recv_cb recv_cb);

void at_mod_parse(uint8_t *buf, uint8_t len);

void at_uart_recv(uint8_t ch);


/**< AT外部指令回调函数接口 */
void ver_cmd_register(config_callback config, select_callback select);			/**< 版本号信息 */                                                                      
void rfpwr_cmd_register(config_callback config, select_callback select);		/**< rf功率 */
void ch_cmd_register(config_callback config, select_callback select);			/**< rf信道 */
void rfsd_cmd_register(config_callback config, select_callback select);			/**< rf发送数据 */
void factory_reset_cmd_register(config_callback config, select_callback select);/**< 还原出厂模式 */
void restart_cmd_register(config_callback config, select_callback select);		/**< 复位 */

/**< AT内部指令回调函数接口 */
void license_cmd_register(config_callback config, select_callback select);		/**< license */
void device_cmd_register(config_callback config, select_callback select);		/**< 设备类型 */
//void id_cmd_register(config_callback config, select_callback select);			/**< ID */
void ip_cmd_register(config_callback config, select_callback select);			/**< IP */

