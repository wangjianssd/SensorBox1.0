#pragma once
#include <data_type_def.h>
#include <sbuf.h>
#include <node_cfg.h>
#include <phy_packet.h>
#include <phy_state.h>
#include <mac_module.h>
#include "../../general/general.h"

#define SCAN_SYNC_TIME                       (3000u)
#define DELAY_TIME							 (600u)
typedef void (*time_slot_cb)(void *seq_p);
#define MAX_HOP_NUM                             (7u)           /**< 中继器最大跳数 */
#define MAX_DOWN_GTS_NUM                        (7u)           /**< 下行最大帧个数 */
#define GTS_NUM                                 (7u)


typedef enum
{
    MAC_FRAME_TYPE_BEACON = 0,
    MAC_FRAME_TYPE_DATA,
    MAC_FRAME_TYPE_ACK,
    MAC_FRAME_TYPE_COMMAND,
    MAC_FRAME_TYPE_RESERVED,
} mac_frame_switch_e;

typedef enum
{
    BEACON_SLOT = 0,
    INTRA_GTS_SLOT,
    SUPER_FRAME_SLOT,
    INTRACOM_SLOT,
    INTER_GTS_SLOT,
    INTERCOM_SUB_SLOT,
    INTERCOM_SLOT,
    SUB_CAP_SLOT,
    CAPCOM_SLOT,
    SLEEP_SLOT,
    BEACON_INTERVAL,
    SLOT_END,
} slot_cb_e;

#pragma pack(1)
typedef struct
{
    uint16_t beacon_interv_order;               /**< 大超帧时间指数       （可外部配置） */
    uint8_t  beacon_duration_order;             /**< beacon时隙时间指数   （可外部配置） */
    uint32_t
    down_link_slot_edge         :   1,          /**< 下行数据是从簇内顺序下发，还是逆顺序下发 */
                                down_link_slot_length       :   3,          /**< 有pengdding时下发时隙的长度 */
                                gts_duration                :   5,          /**< GTS时隙长度          (可外部配置) */
                                intra_channel               :   4,          /**< 簇内信道             (可外部配置) */
                                cluster_number              :   4,          /**< 簇内时隙个数         (可外部配置) */
                                intra_gts_number            :   7,          /**< 簇内时隙GTS的个数    (可外部配置) */
                                inter_channel               :   4,          /**< 簇间信道             (可外部配置) */
                                inter_unit_number           :   3,          /**< 簇间交互单元个数     (可外部配置) */
                                reserved                    :   1;
    uint8_t intra_cap_number;
    uint8_t inter_gts_number[MAX_HOP_NUM];      /**< 簇间单元内的GTS时隙个数(可外部配置) */
} supf_spec_t;

typedef struct
{
    uint8_t mac_seq_num;
    uint8_t self_cluster_index;				//自己的簇号
    uint8_t self_gts_index;					//自己的时隙号
    uint16_t local_beacon_map;
    uint8_t hops;
    uint8_t coord_cluster_index;
    int8_t down_rssi;                   //父节点下来的rssi值
    uint16_t coord_saddr;               //父节点短地址
    uint8_t intra_channel;
    uint64_t mac_addr;
    uint16_t self_saddr;
    supf_spec_t supf_cfg_arg;
    time_slot_cb time_slot[SLOT_END];   //时隙中的所有回调函数集合
} mac_pib_t;

typedef struct
{
    uint16_t frm_type           :    3,             /**<  MAC帧类型 */
             sec_enable         :    1,             /**<  安全机制使能标志 */
             frm_pending        :    1,             /**<  连续发送数据帧标志位 */
             ack_req            :    1,             /**<  是否需要ACK应答 */
             des_addr_mode      :    2,             /**<  MAC帧目的地址模式 */
             src_addr_mode      :    2,             /**<  MAC帧源地址模式 */
             reseverd           :    6;             /**<  保留 */
} mac_frm_ctrl_t;

typedef struct
{
    mac_frm_ctrl_t ctrl;
    uint8_t        seq;
    struct
    {
        uint64_t    dst_addr;
        uint64_t    src_addr;
    } addr;
} mac_head_t;

typedef struct
{
    uint8_t short_addr_num      :   4,
            long_addr_num       :   4;
} pend_addr_spec_t;

typedef struct
{
    uint8_t  index   : 4,
             hops    : 4;
    uint32_t time_stamp;
} bcn_payload_t;


typedef struct
{
    uint8_t  status     : 4,
             index      : 4;
} mac_assoc_res_arg_t;
#pragma pack()


#pragma pack(1)
typedef struct
{
    drivce_type_e       *drivce_type;
    mac_state_e       	*state;
    mac_pib_t           *mac_pib;
} sync_info_t;
#pragma pack()
