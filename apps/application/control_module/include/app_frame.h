 /**
 * @brief       : 前端应用数据帧构造
 *
 * @file        : app_frame.h
 * @author      : cuihongpeng
 * @version     : v0.0.2
 * @date        : 2015/09/22
 *
 * Change Logs  : 修改注释及代码格式
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 * 2015/09/23  v0.0.2       xukai       修改部分内容
 */
#ifndef _APP_FRAME_H_
#define _APP_FRAME_H_

#include <gznet.h>
//#include <data_type_def.h>
//#include <coap_client.h>
//#include <tfp_printf.h>
#include <stdlib.h>


#define MAX_BOX_FRAME_LENGTH 128
/**
 * @brief CoAP请求路由
 */   
#define ROUTE_SENSOR        "D"         /**< 传感器路由 */
#define ROUTE_ALARM         "A"         /**< 报警路由 */
#define ROUTE_LOCATION      "L"         /**< 位置路由 */
#define ROUTE_HEART         "H"         /**< 心跳路由 */     
#define ROUTE_DEVICE        "B"         /**< 设备路由 启动*/
     
#define COAP_TYPE_SENSOR        CT_CON      /**< CON类型 */
#define COAP_TYPE_ALARM         CT_CON      /**< CON类型 */
#define COAP_TYPE_LOCATION      CT_CON      /**< CON类型 */
#define COAP_TYPE_HEART         CT_NON      /**< NON类型 */ 
#define COAP_TYPE_DEVICE        CT_CON      /**< CON类型 */

/**
* @brief 传感器信息数据类型
*/
typedef enum
{
    LITTLE_ENDIAN = 0,
    BIG_ENDIAN
} endian_t;

/**
* @brief 设备启动路由
*/
#define DEVICE_USER_ID_INDEX            0
#define DEVICE_TIMESTAMP_INDEX          4
#define DEVICE_PROFILE_ID_INDEX         8
#define DEVICE_PROFILE_MD5_INDEX        12

#define PROFILE_ID_LEN                  4          /**< 配置文件长度 */
#define PROFILE_MD5_LEN                 16

typedef struct
{
    uint32_t user_id;                               /**< 用户编号 */
    uint32_t timestamp;                             /**< 用户操作时间 */
    uint8_t profile_id[PROFILE_ID_LEN];             /**< profile id */
    uint8_t profile_md5[PROFILE_MD5_LEN];            /**< profile MD5 */
} box_device_info_t;


/**
* @brief 传感器信息数据结构
*/
#define     SENSOR_TYPE_INDEX               0
#define     SENSOR_LEN_INDEX                1
#define     SENSOR_USER_ID_INDEX            2
#define     SENSOR_TIMESTAMP_INDEX          6
#define     SENSOR_CARGO_MD5_INDEX          10
#define     SENSOR_TMP_HUM_INDEX			10


#define     SENSOR_UNLOCK                   0x01    /**< 开锁操作 */
#define     SENSOR_LOCK                     0x02    /**< 关锁操作 */
#define     SENSOR_LOSE_RIGHTS              0x03    /**< 用户放弃感知箱权限 */
//#define     SENSOR_ACQQUIRE_RIGHTS          0x04    /**< 用户获取感知箱权限 */
#define     SENSOR_CARGO                    0x05    /**< 货物变更操作 */
#define     SENSOR_TMP_HUM                  0x04    /**< ��ȡ��ʪ�ȴ�������Ϣ*/


typedef struct
{
    uint8_t type;                           /**< 数据类型 */
    uint8_t len;                            /**< 数据长度 */
    uint32_t user_id;                       /**< 用户ID */
    uint32_t timestamp;                     /**< 告警时间 */
    uint8_t content[16];                    /**< 若type=0x03, 存放货物信息的MD5结果，货物信息如何存在 */
    fp32_t  temperature_value;              /**< �¶�ֵ*/
    fp32_t  humidity_value;					/**< ʪ��ֵ*/

} sensor_info_t;

/**
* @brief 报警信息数据结构
*/
#define ALARM_TYPE_INDEX                    0

#define ALARM_T_OVERRUN                     0x01    /**< 温度超限 */
#define ALARM_H_OVERRUN                     0x02    /**< 湿度超限 */
#define ALARM_ACC_OVERRUN                   0x03    /**< 箱子被搬动 */
#define ALARM_ACC_MOVE                      0x04    /**<箱子异常震动>*/
#define ALARM_NO_PASSWORD                   0x05    /**< 非授权操作报警 */  
#define ALARM_LOCK_LOWPOWER                 0x06    /**< 电子电量低报警 */ 
typedef struct
{
    uint8_t type;                               /**< 类型信息 */

	uint16_t sn;
} alarm_info_t;

/**
* @brief 位置信息数据结构
*/
#define     LOCATION_TYPE_INDEX             0
#define     LOCATION_LEN_INDEX              1
#define     LOCATION_GPS_LONG_INDEX         2
#define     LOCATION_GPS_LAT_INDEX          6

typedef struct
{
    double longitude;                           /**< 经度结果 */
    double latitude;                            /**< 纬度结果 */        
} gps_info_t;

typedef struct
{
    uint8_t type;                               /**< 数据类型 */
    uint8_t len;                                /**< 数据长度 */
    gps_info_t gps_info;                        /**< gps信息 */
} location_info_t;

/**
* @brief 心跳信息数据结构
*/

#define HEART_SSN_ENERGY_INDEX            0
#define HEART_LOCK_ENERGY_INDEX           0
#define HEART_BOX_OPERATION_STATUS_INDEX  2

typedef struct
{
    uint16_t ssn_remain_energy;                 // ssn剩余电量
    uint16_t lock_remain_energy;                // lock剩余电量 
    uint8_t box_operation_status;              // box operation status
} heart_info_t;

/**
* @brief 数据帧payload部分
*/
typedef enum
{
    BOX_REG_DEVICE_FRAME = 0x00,
    BOX_SENSOR_FRAME,
    BOX_ALARM_FRAME,
    BOX_LOCATION_FRAME,
    BOX_HEART_FRAME,
}box_frame_type_e;

typedef struct
{
    uint8_t frame_type;
    union
    {
		box_device_info_t   box_device_info;//注册设备信息
		sensor_info_t       sensor_info;//传感器信息
		alarm_info_t        alarm_info;//报警
		location_info_t     location_info;//位置
		heart_info_t        heart_info;//心跳        
    }box_type_frame_u;    
}box_frame_t;


/**
 *
 * @brief 填充应用层设备启动CoAP请求负载
 *
 * @param[in]  info 位置信息负载
 * @param[out] payload  被填充的缓冲区
 * @param[out] max_len  被填充的缓冲区可被利用的最大长度
 *
 * @return 填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t make_device_payload(box_device_info_t info, 
                            uint8_t *payload, uint8_t max_len);
/**
 *
 * @brief 填充应用层设备启动信息CoAP请求
 *
 * @param[in] pdu coap请求完整内容
 * @param[in] uid 感知箱uid
 * @param[out] payload  coap负载具体内容
 * @param[out] payload_len  coap负载长度
 *
 * @return pdu填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t coap_post_device_request(coap_pdu_t *pdu, uint64_t uid, 
                                 uint8_t *payload, uint8_t payload_len,uint16_t frame_sn);

/**
 *
 * @brief 填充应用层位置信息CoAP请求负载
 *
 * @param[in]  info 位置信息负载
 * @param[out] payload  被填充的缓冲区
 * @param[out] max_len  被填充的缓冲区可被利用的最大长度
 *
 * @return 填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t make_location_payload(location_info_t info, 
                              uint8_t *payload, uint8_t max_len);

/**
 *
 * @brief 填充应用层位置信息CoAP请求
 *
 * @param[in] pdu coap请求完整内容
 * @param[in] uid 感知箱uid
 * @param[out] payload  coap负载具体内容
 * @param[out] payload_len  coap负载长度
 *
 * @return pdu填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t coap_post_location_request(coap_pdu_t *pdu, uint64_t uid, 
                                   uint8_t *payload, uint8_t payload_len,uint16_t frame_sn);
                                   
/**
 *
 * @brief 填充应用层传感信息CoAP请求负载
 *
 * @param[in]  info 传感器信息
 * @param[out] payload  被填充的缓冲区
 * @param[out] max_len  被填充的缓冲区可被利用的最大长度
 *
 * @return 填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t make_sensor_payload(sensor_info_t info, 
                            uint8_t *payload, uint8_t max_len);
 
/**
 *
 * @brief 填充应用层传感器信息CoAP请求
 *
 * @param[in] pdu coap请求完整内容
 * @param[in] uid 感知箱uid
 * @param[out] payload  coap负载具体内容
 * @param[out] payload_len  coap负载长度
 *
 * @return pdu填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t coap_post_sensor_request(coap_pdu_t *pdu, uint64_t uid, 
                                   uint8_t *payload, uint8_t payload_len,uint16_t frame_sn);

/**
 *
 * @brief 填充应用层报警信息CoAP请求负载
 *
 * @param[in]  info 位置信息负载
 * @param[out] payload  被填充的缓冲区
 * @param[out] max_len  被填充的缓冲区可被利用的最大长度
 *
 * @return 填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t make_alarm_payload(alarm_info_t info, 
                              uint8_t *payload, uint8_t max_len);

/**
 *
 * @brief 填充应用层报警信息CoAP请求
 *
 * @param[in] pdu coap请求完整内容
 * @param[in] uid 感知箱uid
 * @param[out] payload  coap负载具体内容
 * @param[out] payload_len  coap负载长度
 *
 * @return pdu填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t coap_post_alarm_request(coap_pdu_t *pdu, uint64_t uid, 
                                uint8_t *payload, uint8_t payload_len,uint16_t frame_sn);
 
/**
 *
 * @brief 填充应用层心跳信息CoAP请求负载
 *
 * @param[in]  info 位置信息负载
 * @param[out] payload  被填充的缓冲区
 * @param[out] max_len  被填充的缓冲区可被利用的最大长度
 *
 * @return 填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t make_heart_payload(heart_info_t info, 
                           uint8_t *payload, uint8_t max_len);

/**
 *
 * @brief 填充应用层心跳信息CoAP请求
 *
 * @param[in] pdu coap请求完整内容
 * @param[in] uid 感知箱uid
 * @param[out] payload  coap负载具体内容
 * @param[out] payload_len  coap负载长度
 *
 * @return pdu填充的字节数
 *   - 0 填充失败
 *   - 非0 填充成功的字节数
 */
uint8_t coap_post_heart_request(coap_pdu_t *pdu, uint64_t uid, 
                                uint8_t *payload, uint8_t payload_len,uint16_t frame_sn);

#endif