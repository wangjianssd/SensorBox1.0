/***************************************************************************
* @brief        : this
* @file         : nwk_frames.h
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-08-06
* @change Logs  :
* Date        Version      Author      Notes
* 2015-08-06      v0.1      gang.cheng    first version
***************************************************************************/
#ifndef __NWK_FRAMES_H__
#define __NWK_FRAMES_H__


/** NWK头占用字节数 */
#define NWK_HEAD_SIZE               1

/** NWK qos 占用字节数 */
#define NWK_QOS_SIZE                1

/** NWK seq 占用字节数 */
#define NWK_SEQ_SIZE                1

/** NWK短地址占用字节数 */
#define NWK_ADDR_SHORT_SIZE         2

/** NWK长地址占用字节数 */
#define NWK_ADDR_LONG_SIZE          8

/** NWK短地址 */
#define NWK_SINK_ADDR               0xFFFE

/** NWK广播地址 */
#define NWK_BROADCAST_ADDR          0xFFFF

//*< 发送正常数据的长度
#define NWK_OFFSET_SIZE             (MAC_OFFSET_SIZE + NWK_HEAD_SIZE + NWK_QOS_SIZE + \
                                     NWK_SEQ_SIZE + NWK_ADDR_SHORT_SIZE * 2)   

typedef struct
{
    uint8_t mode;               //*< 地址模式
    union
    {
        uint64_t   long_addr;   //*< 8字节长地址
        uint16_t  short_addr;   //*< 2字节短地址
    };
} mac_node_addr_t;

/**
* NWK帧类型枚举
*/
typedef enum {
    NWK_FRM_TYPE_DATA       = 0,            //*< NWK数据类型
    NWK_FRM_TYPE_JOIN_REQ   = 1,            //*< NWK入网请求类型
    NWK_FRM_TYPE_JOIN_RESP  = 2,            //*< NWK入网应答类型
    NWK_FRM_TYPE_HEART_BEAT = 3,            //*< NWK心跳类型
    NWK_FRM_TYPE_ROUTE_REQ  = 4,            //*< NWK路由请求帧
    NWK_FRM_TYPE_ROUTE_RESP = 5,            //*< NWK路由应答帧
    NWK_FRM_TYPE_DATA_ACK   = 6,            //*< NWK数据应答帧
    NWK_FRM_TYPE_RESERVED   = 7,            //*< NWK保留类型
} nwk_frm_type_enum;

/**
* NWK地址模式枚举
*/
typedef enum {
    NWK_ADDR_MODE_NONE      = 0,            //*< 没有地址
    NWK_ADDR_MODE_SHORT     = 2,            //*< 短地址模式
    NWK_ADDR_MODE_LONG      = 3,            //*< 长地址模式
} nwk_addr_mode_enum_t;

/**
 * QOS LEVEL
 */
typedef enum
{
    QOS_LEVEL_TIME = 0x00,                  //*< 数据时延优先
    QOS_LEVEL_DATA = 0x01,                  //*< 数据带宽优先
} qos_level_t;

/**
 * 入网应答结果类型定义
 */
typedef enum
{
    NET_JOIN_OK             = 0x00,
    NET_JOIN_ADDR_FAIL      = 0x01,
    NET_JOIN_LICENSE        = 0x02,
    NET_JOIN_LIC_ADDR       = 0x03,
} net_join_result_enum_t;

/**
* NWK心跳设备类型枚举
*/
//typedef enum {
//    NWK_HEART_DEV_TYPE_TAG  = 0,            //*< 终端类型
//    NWK_HEART_DEV_TYPE_ROUT = 1,            //*< 中继类型
//    NWK_HEART_DEV_TYPE_GAT  = 2,            //*< 网关类型
//} nwk_heart_dev_type_enum_t;

/**
* NWK心跳报警类型枚举
*/
typedef enum {
    NWK_HEART_ALARM_NONE    = 0,            //*< 普通心跳类型
    NWK_HEART_ALARM_RESTART = 1,            //*< 重启心跳类型
    NWK_HEART_ALARM_REJOIN  = 2,            //*< 重入网心跳类型
} nwk_heart_alarm_enum_t;

/**
* NWK心跳能量类型枚举
*/
typedef enum {
    NWK_HEART_ENERGY_NONE    = 0,           //*< 能量指示不明
    NWK_HEART_ENERGY_ACTIVE  = 1,           //*< 有源供电
    NWK_HEART_ENERGY_BATTERY = 2,           //*< 电池供电
} nwk_heart_energy_enum_t;

#pragma pack(1)

/**
* nwk头控制域结构体
*/
typedef struct
{
    uint8_t  frm_ctrl        : 3,           //*< 帧类型
             dst_mode        : 2,           //*< 目的地址帧类型
             src_mode        : 2,           //*< 源地址帧类型
             reserved        : 1;           //*< 保留
} nwk_hd_ctl_t;

/**
* NWK头域结构体，包含控制域、目的地址、源地址；
*/
typedef struct
{
    nwk_hd_ctl_t nwk_hd_ctl;                //*< nwk帧头控制域
    qos_level_t qos_level;                  //*< QOS分级
    uint8_t seq_num;                        //*< nwk的帧序列号
    uint64_t dst_addr;                      //*< 目的地址（根据控制域决定是8字节还是2字节）
    uint64_t src_addr;                      //*< 源地址（根据控制域决定是8字节还是2字节）
} nwk_hd_addr_t;


/**
* NWK入网请求帧载荷结构体
*/
typedef struct
{
    uint8_t device_type     : 2,            //*< 入网申请设备类型
            hop_num         : 4,            //*< 节点到网关的跳数，转发增加1
            reserved        : 2;            //*< 保留
    uint16_t father_id;                     //*< 父节点信息
    uint64_t nui;                           //*< 设备唯一NUI
    uint32_t join_apply_time;               //*< 入网请求时间
} nwk_join_req_t;

/**
* NWK入网应答帧载荷结构体
*/
typedef struct
{
    uint8_t join_res        : 2,            //*< 入网应答结果
            hop_num         : 4,            //*< 节点到网关的跳数，转发增加1
            reserved        : 2;            //*< 保留
    uint16_t father_id;
    uint64_t nui;
    uint16_t nwk_id;                        //*< 分配的网络地址
} nwk_join_resp_t;

/**
* NWK心跳状态指示域结构体
*/
typedef struct
{
    uint8_t device_type       : 2,          //*< 设备类型
            energy_support    : 2,          //*< 能量类型
            transmission_en   : 1,          //*< 传输成功率使能
            alarm_info        : 2,          //*< 心跳类型
            localization      : 1;          //*< 定位支持
} nwk_hb_status_t;


/**
* NWK心跳的载荷实体结构体
*/
typedef struct
{
    nwk_hb_status_t device_status;          //*<帧结构中的设备状态位
    uint16_t        father_id;              //*< 父节点
    uint8_t         residual_energy;        //*< 能量剩余
    uint8_t         transmission;           //*< 传输成功率
    uint16_t        alarm_info;             //*< 报警行号
    void *          localization_info;      //*< 定位信息
} nwk_heartbeat_t;

/**
 * NWK路由请求帧
 */
typedef struct
{
    uint64_t dst_id;                        //*< 路由请求到达的目的地址
} nwk_route_req_t;

/**
 * NWK路由应答帧
 */
typedef struct
{
    uint64_t dst_id;                        //*< 路由请求到达的目的地址
    uint16_t dst_nwk_id;                    //*< 查找到的目标网络地址
} nwk_route_resp_t;


#pragma pack()

/**
 * @brief 从pbuf数据段获取nwk层控制域、地址域的结构体，data_p指针偏移到获取以后的地址
 * @param[in]  pbuf     填写数据区的pbuf指针
 * @param[out]  nwk_addr nwk层的控制域结构体
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_hd_addr_get(pbuf_t *pbuf, nwk_hd_addr_t *nwk_addr);

/**
 * @brief 把nwk层控制域、地址域的结构体填充到pbuf数据段，data_p指针偏移到填充以后的地址
 * @see  nwk_frames_join_req_fill(),
 * @param[in]  pbuf     填写数据区的pbuf指针
 * @param[out]  nwk_addr nwk层的控制域结构体
 * @return 填充多少字节，如果为0表示失败
 */
uint8_t nwk_frames_hd_addr_fill(pbuf_t *pbuf, nwk_hd_addr_t *nwk_addr);

/**
 * @brief 修改网络目的地址,默认pbuf里已经存在了完整的一条待发数据，且目的地址是短
 *		  地址
 * 
 * @param[in] pbuf 	填写数据区的pbuf指针
 * @param[in] addr	目的网络短地址
 */
void nwk_frames_change_dst_addr(pbuf_t *pbuf, uint16_t addr);

/**
 * @brief 把入网请求载荷从pbuf数据段获取，data_p指针偏移到填充以后的地址
 * @param[in]  pbuf   数据区的pbuf指针
 * @param[out]  req    nwk层的入网请求载荷
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_join_req_get(pbuf_t *pbuf, nwk_join_req_t *req);

/**
 * @brief 把入网请求载荷填充到pbuf数据段，data_p指针偏移到填充以后的地址
 * @param[in]  pbuf   数据区的pbuf指针
 * @param[out]  req    nwk层的入网请求载荷
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_join_req_fill(pbuf_t *pbuf, nwk_join_req_t *req);

/**
 * @brief 把路由应答载荷填充到pbuf数据段，data_p指针偏移到填充以后的地址
 * @param[in]  pbuf   数据区的pbuf指针
 * @param[out]  req    nwk层的入网应答载荷
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_route_resp_fill(pbuf_t *pbuf, nwk_route_resp_t *resp);

/**
 * @brief 从pbuf数据段获取入网应答载荷
 * @param[in]  pbuf   数据区的pbuf指针
 * @param[out]  req    nwk层的入网应答载荷
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_join_resp_get(pbuf_t *pbuf, nwk_join_resp_t *resp);


/**
 * @brief 把入网应答载荷填充到pbuf数据段
 * @param[in]  pbuf   数据区的pbuf指针
 * @param[out]  req    nwk层的入网应答载荷
 * @return 填充多少字节，如果为0表示失败
 */
uint8_t nwk_frames_join_resp_fill(pbuf_t *pbuf, nwk_join_resp_t *resp);

/**
* @brief 把心跳信息填充到pbuf数据段
* @param[in] pbuf 填充数据区的pbuf指针
* @param[out] hb 心跳信息的载荷结构体指针
* @return 填充多少字节，如果为0表示失败
*/
uint8_t nwk_frames_heartbeat_fill(pbuf_t *pbuf, nwk_heartbeat_t *hb);


/**
* @brief 把网络路由请求载荷填充到pbuf数据缓冲区
* @param[in] pbuf 数据区的pbuf指针
* @param[out] req 路由请求的载荷结构体指针
* @return 填充到多少字节，如果为0表示失败
*/
uint8_t nwk_frames_route_req_fill(pbuf_t *pbuf, nwk_route_req_t *req);

/**
* @brief 从pbuf数据缓冲区里面拷贝出网络路由请求载荷
*
* @param[in] pbuf 数据缓冲区的指针
* @param[out] req 保存路由请求载荷的数据指针 
*
* @return 获取到了多少字节，如果为0，表示失败
*/
uint8_t nwk_frames_route_req_get(pbuf_t *pbuf, nwk_route_req_t *req);

/**
* @brief 从pbuf数据缓冲区获取网络路由应答载荷
* @param[in] pbuf 数据区的pbuf指针
* @param[out] req 路由请求的载荷结构体指针
* @return 获取到多少字节，如果为0表示失败
*/
uint8_t nwk_frames_route_resp_get(pbuf_t *pbuf, nwk_route_resp_t *resp);


/**
 * @brief 从pbuf拷贝应用层数据段到pbuf数据
 * @param[in] pbuf 指向数据区的pbuf指针
 * @param[in] datap 缓冲的数据缓冲区指针
 * @param[in] len 需要获取的数据的长度
 * @return 获取到多少字节，如果为0表示失败
 */
uint8_t nwk_frames_data_get(pbuf_t *pbuf, void *datap, uint8_t len);

/**
 * @brief 把数据拷贝到pbuf缓冲区
 * @param[in] pbuf 指向数据区的pbuf指针
 * @param[in] datap 缓冲的数据缓冲区指针
 * @param[in] len 需要获取的数据的长度
 * @return 填充了多少字节，如果为0表示失败
 */
uint8_t nwk_frames_data_fill(pbuf_t *pbuf, void *datap, uint8_t len);


#endif
