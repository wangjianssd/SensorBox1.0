#pragma once
#include "common/lib/data_type_def.h"
#include "stack/common/pbuf.h"
/**< define */
#define MAC_BROADCAST_ADDR                      0xffff
#define	CH_NUM              (4u)            /**< 信道最大数 */
#define	ASSOC_TABLE			(1u)			/**< 关联表功能开启状态 */
#define CLUSTER_NUMBER		(16-1)			/**< 簇间个数 */

#if ASSOC_TABLE == 1
#define ASSOC_NUM	(30)
#else
#define ASSOC_NUM	(1)
#endif

#define MAC_FCS_SIZE                            0u
#define MAC_HEAD_CTRL_SIZE                      2u
#define MAC_HEAD_SEQ_SIZE                       1u
#define MAC_ADDR_SHORT_SIZE                     2u
#define MAC_ADDR_LONG_SIZE                      8u

/**< 枚举定义 */

typedef enum
{
    SLOT_READY,
    ASSOC_REQUEST,				//关联请求
	ASSOC_RESPONSE,
    WORK_ON,
    WORK_DOWN,
    SYNC_STATE_END,

    READY_IDLE,
    COORD_READY,
} mac_state_e;

typedef enum
{
    NON_S,                     /**< 未知的 */
    ASYN_S,                    /**< 异步网络 */
    SYNC_S,                    /**< 同步网络 */
    RANDOM_S,                  /**< 随机 */
} net_mode_e;

typedef enum
{
    MAC_CMD_ASSOC_REQ = 1,
    MAC_CMD_ASSOC_RESP,
	MAC_CMD_QUERY_REQ,
	MAC_CMD_QUERY_RESP,
	MAC_CMD_WAKEUP,
	MAC_CMD_WAKEUP_RESP,
	MAC_CMD_DATA,
} mac_frame_type_e;

typedef enum
{
    MAC_MHR_ADDR_MODE_FALSE     = 0u,           //*< 地址模式无效
    MAC_MHR_ADDR_MODE_RESERVED  = 1u,           //*< 地址模式保留
    MAC_MHR_ADDR_MODE_SHORT     = 2u,           //*< 短地址模式
    MAC_MHR_ADDR_MODE_LONG      = 3u,           //*< 长地址模式
} mac_addr_mode_e;

typedef enum
{
    ASSOC_STATUS_SUCCESS     = 0u,
    ASSOC_STATUS_FULL  = 1u,
    ASSOC_STATUS_REFUSE     = 2u,
    ASSOC_STATUS_RESERVED      = 3u,
} mac_assoc_state_e;

typedef enum
{
    MAC_FRAME_TYPE_BEACON = 0,
    MAC_FRAME_TYPE_DATA,
    MAC_FRAME_TYPE_ACK,
    MAC_FRAME_TYPE_COMMAND,
    MAC_FRAME_TYPE_RESERVED,
} mac_frame_switch_e;

#pragma pack(1)
/**< 结构体定义 */
typedef struct
{
    uint16_t device_type    : 2,
             sec_cap        : 1,
             beacon_bitmap  : 13;
    uint32_t assoc_apply_time;
} mac_assoc_req_arg_t;

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
    uint8_t  index   : 4,
             hops    : 4;
    uint32_t time_stamp;
} bcn_payload_t;

typedef struct
{
    uint8_t mac_seq_num; 
    uint8_t hops;
    int8_t down_rssi;                   //父节点下来的rssi值
    uint16_t coord_saddr;               //父节点短地址
    uint64_t mac_addr;
    uint16_t self_saddr;
	uint8_t license[22];
} mac_pib_t;

#pragma pack()


extern mac_head_t mac_head_info;

uint16_t mac_short_addr_get(uint64_t mac_addr);

uint8_t get_addr(pbuf_t *pbuf, mac_addr_mode_e mode, uint64_t *addr);
