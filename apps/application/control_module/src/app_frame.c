 /**
 * @brief       : 应用层前端数据协议实现
 *
 * @file        : app_frame.c
 * @author      : xukai
 * @version     : v0.0.1
 * @date        : 2015/09/22
 *
 * Change Logs  : 修改注释及代码格式
 *
 * Date        Version      Author      Notes
 * 2015/09/22  v0.0.1       xukai       初版
 */
#include <gznet.h>
#include <app_frame.h>

//#include <coap.h>
#include <string.h>

/**
 * @brief message id for coap header
 */
uint16_t msg_id = 1;

extern uint16_t sending_frame_sn;
/**
 *
 * @brief double类型变量变为4字节字符数组
 *
 * @param[out] buf 指向被填充的缓冲区
 * @param[in] value 被转换的double类型变量
 * @param[in] little 大端小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 无
 */
STATIC void double_to_buf(uint8_t *buf, double value, endian_t endian)
{
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        (buf)[0] = ((uint8_t *)(&value))[0];
        (buf)[1] = ((uint8_t *)(&value))[1];
        (buf)[2] = ((uint8_t *)(&value))[2];
        (buf)[3] = ((uint8_t *)(&value))[3];
    }
    else
    {
        // 大端
        (buf)[0] = ((uint8_t *)(&value))[3];
        (buf)[1] = ((uint8_t *)(&value))[2];
        (buf)[2] = ((uint8_t *)(&value))[1];
        (buf)[3] = ((uint8_t *)(&value))[0];
    }   
}

/**
 *
 * @brief uint16_t类型变量变为2字节字符数组
 *
 * @param[out] buf 指向被填充的缓冲区
 * @param[in] value 被转换的uint16_t类型变量
 * @param[in] little 大端小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 无
 */
STATIC void uint16_to_buf(uint8_t *buf, uint16_t value, endian_t endian)
{
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        (buf)[0] = ((uint8_t *)(&value))[0];
        (buf)[1] = ((uint8_t *)(&value))[1];
    }
    else
    {
        // 大端
        (buf)[0] = ((uint8_t *)(&value))[1];
        (buf)[1] = ((uint8_t *)(&value))[0];
    }   
}

/**
 *
 * @brief uint32_t类型变量变为4字节字符数组
 *
 * @param[out] buf 指向被填充的缓冲区
 * @param[in] value 被转换的uint32_t类型变量
 * @param[in] little 大端小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 无
 */
STATIC void uint32_to_buf(uint8_t *buf, uint32_t value, endian_t endian)
{
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        (buf)[0] = ((uint8_t *)(&value))[0];
        (buf)[1] = ((uint8_t *)(&value))[1];
        (buf)[2] = ((uint8_t *)(&value))[2];
        (buf)[3] = ((uint8_t *)(&value))[3];
    }
    else
    {
        // 大端
        (buf)[0] = ((uint8_t *)(&value))[3];
        (buf)[1] = ((uint8_t *)(&value))[2];
        (buf)[2] = ((uint8_t *)(&value))[1];
        (buf)[3] = ((uint8_t *)(&value))[0];
    }   
}
#if 0
/**
 *
 * @brief uint64_t类型变量变为8字节字符数组
 *
 * @param[out] buf 指向被填充的缓冲区
 * @param[in] value 被转换的uint64_t类型变量
 * @param[in] little 大端小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 无
 */
STATIC void uint64_to_buf(uint8_t *buf, uint64_t value, endian_t endian)
{
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        (buf)[0] = ((uint8_t *)(&value))[0];
        (buf)[1] = ((uint8_t *)(&value))[1];
        (buf)[2] = ((uint8_t *)(&value))[2];
        (buf)[3] = ((uint8_t *)(&value))[3];
        (buf)[4] = ((uint8_t *)(&value))[4];
        (buf)[5] = ((uint8_t *)(&value))[5];
        (buf)[6] = ((uint8_t *)(&value))[6];
        (buf)[7] = ((uint8_t *)(&value))[7];
    }
    else
    {
        // 大端
        (buf)[0] = ((uint8_t *)(&value))[7];
        (buf)[1] = ((uint8_t *)(&value))[6];
        (buf)[2] = ((uint8_t *)(&value))[5];
        (buf)[3] = ((uint8_t *)(&value))[4];
        (buf)[4] = ((uint8_t *)(&value))[3];
        (buf)[5] = ((uint8_t *)(&value))[2];
        (buf)[6] = ((uint8_t *)(&value))[1];
        (buf)[7] = ((uint8_t *)(&value))[0];
    }   
}     
#endif

#if 0
/**
 *
 * @brief 4字节字符数组转化为double类型变量
 *
 * @param[in] buf 4字节字符数组
 * @param[in] endian 大端或小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 转换结果
 */
STATIC double buf_to_double(uint8_t *buf, endian_t endian)
{
    double value = 0.00;
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        ((uint8_t *)(&(value)))[0] = (buf)[0];
        ((uint8_t *)(&(value)))[1] = (buf)[1];
        ((uint8_t *)(&(value)))[2] = (buf)[2];
        ((uint8_t *)(&(value)))[3] = (buf)[3];
    }
    else
    {
        // 大端
        ((uint8_t *)(&(value)))[0] = (buf)[3];
        ((uint8_t *)(&(value)))[1] = (buf)[2];
        ((uint8_t *)(&(value)))[2] = (buf)[1];
        ((uint8_t *)(&(value)))[3] = (buf)[0];
    }
    return value;
}
#endif

#if 0
/**
 *
 * @brief 2字节字符数组转化为uint16_t类型变量
 *
 * @param[in] buf 2字节字符数组
 * @param[in] endian 大端或小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 转换结果
 */
STATIC uint16_t buf_to_uint16(uint8_t *buf, endian_t endian)
{
    uint32_t value = 0x00;
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        ((uint8_t *)(&(value)))[0] = (buf)[0];
        ((uint8_t *)(&(value)))[1] = (buf)[1];
    }
    else
    {
        // 大端
        ((uint8_t *)(&(value)))[2] = (buf)[1];
        ((uint8_t *)(&(value)))[3] = (buf)[0];
    }
    return value;
}
#endif

#if 0
/**
 *
 * @brief 4字节字符数组转化为double类型变量
 *
 * @param[in] buf 4字节字符数组
 * @param[in] endian 大端或小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 转换结果
 */
STATIC uint32_t buf_to_uint32(uint8_t *buf, endian_t endian)
{
    uint32_t value = 0x00;
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        ((uint8_t *)(&(value)))[0] = (buf)[0];
        ((uint8_t *)(&(value)))[1] = (buf)[1];
        ((uint8_t *)(&(value)))[2] = (buf)[2];
        ((uint8_t *)(&(value)))[3] = (buf)[3];
    }
    else
    {
        // 大端
        ((uint8_t *)(&(value)))[0] = (buf)[3];
        ((uint8_t *)(&(value)))[1] = (buf)[2];
        ((uint8_t *)(&(value)))[2] = (buf)[1];
        ((uint8_t *)(&(value)))[3] = (buf)[0];
    }
    return value;
}
#endif

#if 0
/**
 *
 * @brief 8字节字符数组转化为uint64_t类型变量
 *
 * @param[in] buf 8字节字符数组
 * @param[in] endian 大端或小端指示
 *   - LITTLE_ENDIAN 小端
 *   - BIG_ENDIAN 大端
 *
 * @return 转换结果
 */
STATIC uint64_t buf_to_uint64(uint8_t *buf, endian_t endian)
{
    uint64_t value = 0x00;
    if (endian == LITTLE_ENDIAN)
    {
        //  小端
        ((uint8_t *)(&(value)))[0] = (buf)[0];
        ((uint8_t *)(&(value)))[1] = (buf)[1];
        ((uint8_t *)(&(value)))[2] = (buf)[2];
        ((uint8_t *)(&(value)))[3] = (buf)[3];
        ((uint8_t *)(&(value)))[4] = (buf)[4];
        ((uint8_t *)(&(value)))[5] = (buf)[5];
        ((uint8_t *)(&(value)))[6] = (buf)[6];
        ((uint8_t *)(&(value)))[7] = (buf)[7];
    }
    else
    {
        // 大端
        ((uint8_t *)(&(value)))[0] = (buf)[7];
        ((uint8_t *)(&(value)))[1] = (buf)[6];
        ((uint8_t *)(&(value)))[2] = (buf)[5];
        ((uint8_t *)(&(value)))[3] = (buf)[4];
        ((uint8_t *)(&(value)))[4] = (buf)[3];
        ((uint8_t *)(&(value)))[5] = (buf)[2];
        ((uint8_t *)(&(value)))[6] = (buf)[1];
        ((uint8_t *)(&(value)))[7] = (buf)[0];
    }
    return value;
}
#endif

/**
 *
 * @brief 字符数组型uid变量转换为字符串形式
 *
 * @param[in] uid 设备唯一编号
 * @param[out] str 指向被格式化的区域
 * @return 无
 */
STATIC void print_uid(uint64_t uid, uint8_t *str)
{
    uint8_t buf[8];
    memcpy(buf, (void*)&uid, 8);
    tfp_sprintf((char*)str, "%02X%02X%02X%2X%02X%02X%02X%02X", 
                            buf[7], buf[6], buf[5], buf[4],
                            buf[3], buf[2], buf[1], buf[0]);
}

uint8_t make_device_payload(box_device_info_t info, 
                            uint8_t *payload, uint8_t max_len)
{
    uint8_t info_size =  sizeof(box_device_info_t);
    uint8_t len = 0;
    if (max_len < info_size)
    {
        return 0;   // payload区域不足，导致填充失败，严重错误
    }

#if 0	
    uint32_to_buf(&payload[DEVICE_USER_ID_INDEX], info.user_id, BIG_ENDIAN);
    len += 4;
    
    uint32_to_buf(&payload[DEVICE_TIMESTAMP_INDEX], info.timestamp, BIG_ENDIAN);
    len += 4;
    
    memcpy(&payload[DEVICE_PROFILE_ID_INDEX], info.profile_id, PROFILE_ID_LEN);
    len += PROFILE_ID_LEN;

    memcpy(&payload[DEVICE_PROFILE_MD5_INDEX], info.profile_md5, PROFILE_MD5_LEN);
    len += PROFILE_MD5_LEN;

#else

	payload[0]= 0x00;
	payload[1]= 0x00;
	payload[2]= 0x01;
    len += 3;

#endif

    return len;    
}

uint8_t coap_post_device_request(coap_pdu_t *pdu, uint64_t uid, 
                                 uint8_t *payload, uint8_t payload_len,uint16_t frame_sn)
{
    coap_init_pdu(pdu);                 // 初始化数据单元
    
    // 设置版本
    coap_set_version(pdu, COAP_V1);
    // 设置类型
    coap_set_type(pdu, COAP_TYPE_DEVICE);                     

    // post方法
    coap_set_code(pdu, CC_POST);
    // 设置信息ID      
	coap_set_mid(pdu,frame_sn);
	
    // 设置URL第一部分，<sensor_id>
    uint8_t str_uid[32];
    print_uid(uid, &str_uid[0]);
    coap_add_option(pdu, CON_URI_PATH, str_uid, strlen((char const *)str_uid)); 
    // 设置URL第二部分
    coap_add_option(pdu, CON_URI_PATH, ROUTE_DEVICE, strlen((char const *)ROUTE_DEVICE)); 
    
    // 设置媒体格式
    uint8_t format = CCF_OCTECT_STREAM;
    coap_add_option(pdu, CON_CONTENT_FORMAT, (uint8_t*)&format, 1);

    // 最后加入载荷
    coap_set_payload(pdu, payload, payload_len); 
    
    // 返回pdu的长度
    return pdu->len;    
}

uint8_t make_location_payload(location_info_t info, uint8_t *payload, 
                              uint8_t max_len)
{
    uint8_t info_size =  sizeof(location_info_t);
    uint8_t len = 0;
    if (max_len < info_size)
    {
        return 0;   // payload区域不足，导致填充失败，严重错误
    }
    
    payload[LOCATION_TYPE_INDEX] = info.type;
    len++;
    payload[LOCATION_LEN_INDEX] = info.len;
    len++;
    
    if (info.len != 8)
    {
        return 0;    //  按照文档定义，负载长度为8
    }
    
    // GPS位置信息 存在大端格式存放
    double_to_buf(&payload[LOCATION_GPS_LONG_INDEX], 
                  info.gps_info.longitude, BIG_ENDIAN);
    len += 4;
    double_to_buf(&payload[LOCATION_GPS_LAT_INDEX],
                  info.gps_info.latitude, BIG_ENDIAN);
    len += 4;

    return len;
}

uint8_t coap_post_location_request(coap_pdu_t *pdu, uint64_t uid, 
                                   uint8_t *payload, uint8_t payload_len,uint16_t frame_sn)
{
    coap_init_pdu(pdu);                 // 初始化数据单元
    
    // 设置版本
    coap_set_version(pdu, COAP_V1);
    // 设置类型
    coap_set_type(pdu, COAP_TYPE_LOCATION);                     

    // post方法
    coap_set_code(pdu, CC_POST);
    // 设置信息ID
    //coap_set_mid(pdu, msg_id++);        
	coap_set_mid(pdu,frame_sn);

    // 设置URL第一部分，<sensor_id>
    uint8_t str_uid[32];
    print_uid(uid, &str_uid[0]);
    coap_add_option(pdu, CON_URI_PATH, str_uid, strlen((char const *)str_uid)); 
    // 设置URL第二部分
    coap_add_option(pdu, CON_URI_PATH, ROUTE_LOCATION, strlen((char const *)ROUTE_LOCATION)); 
    
    // 设置媒体格式
    uint8_t format = CCF_OCTECT_STREAM;
    coap_add_option(pdu, CON_CONTENT_FORMAT, (uint8_t*)&format, 1);

    // 最后加入载荷
    coap_set_payload(pdu, payload, payload_len); 
    
    // 返回pdu的长度
    return pdu->len;
}

uint8_t make_sensor_payload(sensor_info_t info, 
                            uint8_t *payload, uint8_t max_len)
{
    uint8_t info_size =  sizeof(sensor_info_t);
    uint8_t len = 0;    // 负载真实长度
    
    if (max_len < info_size)
    {
        return 0;       // payload区域不足，导致填充失败，严重错误
    }
    
    payload[SENSOR_TYPE_INDEX] = info.type;
    len++;
    payload[SENSOR_LEN_INDEX] = info.len;
    len++;
    
    // 填充用户ID
    uint32_to_buf(&payload[SENSOR_USER_ID_INDEX], info.user_id, BIG_ENDIAN);
    len += 4;
    
    // 填充操作时间
    uint32_to_buf(&payload[SENSOR_TIMESTAMP_INDEX], info.timestamp, BIG_ENDIAN);
    len += 4;
    
    if (info.type == SENSOR_CARGO) 
    {
        len += 16;
        // info.content应存放 货物信息的MD5值
        memcpy(&payload[SENSOR_CARGO_MD5_INDEX], info.content, 16);
    }
	else if(info.type == SENSOR_TMP_HUM)
	{
		len += 8;
		memcpy(&payload[SENSOR_TMP_HUM_INDEX], info.content, 8);
	}

    return len;    
}

uint8_t coap_post_sensor_request(coap_pdu_t *pdu, uint64_t uid, 
                                 uint8_t *payload, uint8_t payload_len,uint16_t frame_sn)
{
    coap_init_pdu(pdu);                 // 初始化数据单元
    
    // 设置版本
    coap_set_version(pdu, COAP_V1);
    // 设置类型
    coap_set_type(pdu, COAP_TYPE_SENSOR);                     

    // post方法
    coap_set_code(pdu, CC_POST);
    // 设置信息ID
    //coap_set_mid(pdu, msg_id++);        
	coap_set_mid(pdu,frame_sn);

    // 设置URL第一部分，<sensor_id>
    uint8_t str_uid[32];
    print_uid(uid, &str_uid[0]);
    coap_add_option(pdu, CON_URI_PATH, str_uid, strlen((char const *)str_uid)); 
    // 设置URL第二部分
    coap_add_option(pdu, CON_URI_PATH, ROUTE_SENSOR, strlen((char const *)ROUTE_SENSOR)); 
    
    // 设置媒体格式
    uint8_t format = CCF_OCTECT_STREAM;
    coap_add_option(pdu, CON_CONTENT_FORMAT, (uint8_t*)&format, 1);

    // 最后加入载荷
    coap_set_payload(pdu, payload, payload_len); 
    
    // 返回pdu的长度
    return pdu->len;
}

uint8_t make_alarm_payload(alarm_info_t info, 
                           uint8_t *payload, uint8_t max_len)
{
    uint8_t info_size =  sizeof(alarm_info_t);
    uint8_t len = 0;    // 负载真实长度
    
    if (max_len < info_size)
    {
        return 0;       // payload区域不足，导致填充失败，严重错误
    }
    
    payload[ALARM_TYPE_INDEX] = info.type;
    len++;

	payload[ALARM_TYPE_INDEX + 1] = (uint8_t)(info.sn>>8);
	len++;

	payload[ALARM_TYPE_INDEX + 2] = (uint8_t)(info.sn);
	len++;

    return len;     
}

uint8_t coap_post_alarm_request(coap_pdu_t *pdu, uint64_t uid, 
                                uint8_t *payload, uint8_t payload_len,uint16_t frame_sn)
{
    coap_init_pdu(pdu);                 // 初始化数据单元
    
    // 设置版本
    coap_set_version(pdu, COAP_V1);
    // 设置类型
    coap_set_type(pdu, COAP_TYPE_ALARM);                     

    // post方法
    coap_set_code(pdu, CC_POST);
    // 设置信息ID
    //coap_set_mid(pdu, msg_id++);        
	coap_set_mid(pdu,frame_sn);

    // 设置URL第一部分，<sensor_id>
    uint8_t str_uid[32];
    print_uid(uid, &str_uid[0]);
    coap_add_option(pdu, CON_URI_PATH, str_uid, strlen((char const *)str_uid)); 
    // 设置URL第二部分
    coap_add_option(pdu, CON_URI_PATH, ROUTE_ALARM, strlen((char const *)ROUTE_ALARM)); 
    
    // 设置媒体格式
    uint8_t format = CCF_OCTECT_STREAM;
    coap_add_option(pdu, CON_CONTENT_FORMAT, (uint8_t*)&format, 1);

    // 最后加入载荷
    coap_set_payload(pdu, payload, payload_len); 
    
    // 返回pdu的长度
    return pdu->len;    
}

uint8_t make_heart_payload(heart_info_t info, 
                           uint8_t *payload, uint8_t max_len)
{
    uint8_t info_size =  sizeof(heart_info_t);
    uint8_t len = 0;    // 负载真实长度
    
    if (max_len < info_size)
    {
        return 0;       // payload区域不足，导致填充失败，严重错误
    }
    
//    uint16_to_buf(&payload[HEART_SSN_ENERGY_INDEX], info.ssn_remain_energy, BIG_ENDIAN);
//    len += 2;
    
    uint16_to_buf(&payload[HEART_LOCK_ENERGY_INDEX], info.lock_remain_energy, BIG_ENDIAN);
    len += 2;

	//uint16_to_buf(&payload[HEART_BOX_OPERATION_STATUS_INDEX], info.box_operation_status, BIG_ENDIAN);
	payload[HEART_BOX_OPERATION_STATUS_INDEX] = (uint8_t)(info.box_operation_status);
    len += 1;

    return len;     
}

uint8_t coap_post_heart_request(coap_pdu_t *pdu, uint64_t uid, 
                                uint8_t *payload, uint8_t payload_len,uint16_t frame_sn)
{
    coap_init_pdu(pdu);                 // 初始化数据单元
    
    // 设置版本
    coap_set_version(pdu, COAP_V1);
    // 设置类型
    coap_set_type(pdu, COAP_TYPE_HEART);                     

    // post方法
    coap_set_code(pdu, CC_POST);
    // 设置信息ID
    //coap_set_mid(pdu, msg_id++);        
	coap_set_mid(pdu,frame_sn);

    // 设置URL第一部分，<sensor_id>
    uint8_t str_uid[32];
    print_uid(uid, &str_uid[0]);
    coap_add_option(pdu, CON_URI_PATH, str_uid, strlen((char const *)str_uid)); 
    // 设置URL第二部分
    coap_add_option(pdu, CON_URI_PATH, ROUTE_HEART, strlen((char const *)ROUTE_HEART)); 
    
    // 设置媒体格式
    uint8_t format = CCF_OCTECT_STREAM;
    coap_add_option(pdu, CON_CONTENT_FORMAT, (uint8_t*)&format, 1);

    // 最后加入载荷
    coap_set_payload(pdu, payload, payload_len); 
    
    // 返回pdu的长度
    return pdu->len;    
}

