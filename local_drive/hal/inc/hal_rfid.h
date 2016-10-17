 /**
 * @brief       : RFID 中间层代码
 *
 * @file        : hal_rfid.h
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1    cuihongpeng    first version
 */

#ifndef __HAL_RFID_H_
#define __HAL_RFID_H_

#include <rfid.h>

#define ADDRESS_EACH_BLOCK_SIZE					(0x04)

/****************************数据信息基地址定义***************************/
#define ADDRESS_DEVICE_ID_INFO_BASE				(0x0000)// 感知箱ID首地址
#define ADDRESS_OPERATION_INFO_BASE				(0x0080)// 操作信息首地址
#define ADDRESS_CARGO_INFO_BASE					(0x0100)// 货物信息首地址
#define ADDRESS_ABNORMAL_INFO_BASE			    (0x0E00)// 异常信息首地址
#define ADDRESS_APPLICATION_INFO_BASE			(0x1000)// 应用信息首地址
#define ADDRESS_PROFILE_INFO_BASE				(0x1080)// profile信息首地址
#define ADDRESS_USER_INFO_BASE				    (0x1B00)// 用户信息首地址

/****************************操作信息地址定义***************************/
#define ADDRESS_PHONE_OPERATION_INFO			(0x0084)
#define ADDRESS_BOX_ID_INFO			        (0x0088)
#define ADDRESS_OPERATION_USER_ID_INFO		        (0x0090)
#define ADDRESS_OPERATION_TIMER_INFO			(0x0094)
#define ADDRESS_LOCATION_LONG_INFO			(0x0098)
#define ADDRESS_LOCATION_LAT_INFO			(0x009C)
#define ADDRESS_BOX_ACK_INFO			        (0x00A0)  
      
#define ADDRESS_OPERATION_MODIFY_INFO			(ADDRESS_OPERATION_INFO_BASE)
#define ADDRESS_LOCK_STATUS_INFO			(ADDRESS_OPERATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*1)
//#define ADDRESS_OPERATION_USER_ID_INFO		(ADDRESS_OPERATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*2)
//#define ADDRESS_OPERATION_TIMER_INFO			(ADDRESS_OPERATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*3)
#define ADDRESS_ACCESS_INFO				(ADDRESS_OPERATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*4)

/****************************货物信息地址定义***************************/
#define ADDRESS_CARGO_LEN_INFO					(ADDRESS_CARGO_INFO_BASE)
#define ADDRESS_CARGO_INFO						(ADDRESS_CARGO_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*1)

/****************************异常LOG信息地址定义***************************/
#define ADDRESS_ABNORMAL_INFO					(ADDRESS_ABNORMAL_INFO_BASE)

/****************************应用信息地址定义***************************/
#define ADDRESS_APPLICATION_MODIFY_INFO			(ADDRESS_APPLICATION_INFO_BASE)
#define ADDRESS_ACTIVATION_INFO					(ADDRESS_APPLICATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*1)
#define ADDRESS_APPLICATION_USER_ID_INFO		(ADDRESS_APPLICATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*2)
#define ADDRESS_APPLICATION_TIMER_INFO			(ADDRESS_APPLICATION_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*3)

/****************************Profile信息地址定义***************************/
#define ADDRESS_PROFILE_ID_INFO					(ADDRESS_PROFILE_INFO_BASE)
#define ADDRESS_PROFILE_LEN_INFO				(ADDRESS_PROFILE_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*4)
#define ADDRESS_PROFILE_OPTION_INFO				(ADDRESS_PROFILE_INFO_BASE + ADDRESS_EACH_BLOCK_SIZE*5)

/****************************用户信息地址定义***************************/
#define ADDRESS_USER_INFO					(ADDRESS_USER_INFO_BASE)

typedef struct abnormal_log_info_t_
{
    uint32_t timer; 		// 时间戳
    uint8_t log_type;       //用户操作日志类型
}abnormal_log_info_t;

typedef struct user_log_info_t_
{
    uint32_t id;			// 用户ID
    uint32_t timer; 		// 时间戳
    uint8_t log_type;       //用户操作日志类型
}user_log_info_t;
          
typedef enum user_type_e_
{
	OPERATION_USER = 0,
	APPLICATION_USER,
} user_type_e;

typedef struct operation_info_t_
{
	uint32_t lock_status;	// 锁状态
	uint32_t id;			// 用户ID
	uint32_t timer; 		// 时间戳
	uint8_t access; 		// 使用权
} operation_info_t;

typedef struct cargo_info_t_
{
	uint32_t len;	// 货物信息长度
	uint8_t *info;	// 货物信息
} cargo_info_t;


typedef enum
{
    SEN_TRAFFIC_FIELD = 0x01,//感知交通领域
    SEN_SECURITY_FIELD,//感知安防领域
    SEN_LOGISTICS_FIELD,//感知物流领域   
} profile_id_info_e;

typedef enum
{
    ESCORT_APPLICATION = 0x01,//车押应用
    WAREHOUCE_APPLICATION,//仓押应用
    BOX_APPLICATION,//感知箱应用 
} profile_id_specific_info_e;
// 有待完善
typedef struct profile_option_t_
{
//	uint32_t ssn_center_frequency;		// ssn中心频率
//	uint32_t lsn_center_frequency;		// lsn中心频率
    uint32_t heartbeat_interval_in_room; //在仓库中的心跳间隔
    uint32_t heartbeat_interval_out_room; //用户使用时心跳间隔
    uint32_t location_info_interval_in_room;//仓库中的位置信息上报间隔
    uint32_t location_info_interval_out_room;//用户使用时位置信息上报间隔
    uint8_t  temperature_threshold;     //温度阈值
    uint8_t  humidity_threshold;        //湿度阈值
	uint16_t acceleration_threshold;	// 加速度阀值
	uint16_t illumination_threshold;	// 光照阀值	
} profile_option_t;

typedef struct profile_info_t_
{
	uint8_t id[4];//profile ID
	uint16_t length;
	uint16_t num;
	profile_option_t profile_option;
} profile_info_t;

typedef struct application_info_t_
{
	uint32_t activation;	// 激活
	uint32_t id;			// 用户ID
	uint32_t timer; 		// 时间戳
	profile_info_t profile_info;
} application_info_t;

typedef struct rfid_modify_flag_t_
{
	uint32_t operation_modify_flag;
	uint32_t application_modify_flag;
} rfid_modify_flag_t;

/**
 * @brief rfid初始化
 * @param[in] 空
 * @return TRUE or FALSE
 */
bool_t hal_rfid_init(void);

/**
* @brief 写入rfid数据
*
* @param addr:     需要写入数据的地址
* @param len:      需要写入数据的长度
* @param buff:     需要写入数据的内容
*
* @return TRUE or FALSE
*/
bool_t hal_rfid_write_info(uint16_t addr, uint8_t len, uint8_t *buff);


/**
* @brief 读取rfid数据
*
* @param addr:     需要读取数据的地址
* @param len:      需要读取数据的长度
* @param buff:     需要读取数据的内容
*
* @return TRUE or FALSE
*/
bool_t hal_rfid_read_info(uint16_t addr, uint8_t len, uint8_t *buff);

/**
* @brief 读取rfid更改索引
*
* @param rfid_modify_flag: rfid更改索引结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_read_modify_flag(rfid_modify_flag_t* rfid_modify_flag);

/**
* @brief 清除rfid更改索引
*
* @param rfid_modify_flag: rfid更改索引结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_clear_modify_flag(void);

/**
* @brief 更新操作信息
*
* @param operation_info_t: rfid操作信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_operation_info(operation_info_t* operation_info);

/**
* @brief 更新profile信息
*
* @param profile_info_t: profile信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_profile_info(profile_info_t* profile_info);

/**
* @brief 更新应用信息
*
* @param profile_info_t: 应用信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_application_info(application_info_t* application_info);

/**
* @brief 读取rfid读取操作用户ID
*
* @param type: 读取类型
* @param id: 用户ID
*/
bool_t hal_rfid_read_user_id(user_type_e type, uint32_t* id);

/**
* @brief 查看感知箱是否被激活
*
* @return TRUE:已经被激活 FALSE:未被激活
*/
bool_t hal_rfid_access_status(void);
bool_t hal_rfid_updata_cargo_info(cargo_info_t* cargo_info);
#endif
