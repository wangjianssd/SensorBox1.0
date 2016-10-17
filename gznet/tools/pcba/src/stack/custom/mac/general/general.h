#pragma once
#include <data_type_def.h>
#include <pbuf.h>
/**< define */
#define MAC_BROADCAST_ADDR                      0xffff
#define	CH_NUM              (1u)            /**< 信道最大数 */
#define	ASSOC_TABLE			(1u)			/**< 关联表功能开启状态 */
#define CLUSTER_NUMBER		(4u)			/**< 簇间个数 */

#if ASSOC_TABLE == 1
#define ASSOC_NUM	(10)
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
    TERMINAL = 0,   //终端
    ROUTER,     //中继器
    GATEWAY,    //网关
	UNDEFINE,
} drivce_type_e;

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

/**< 结构体定义 */
typedef struct
{
    uint16_t device_type    : 2,
             sec_cap        : 1,
             beacon_bitmap  : 13;
    uint32_t assoc_apply_time;
} mac_assoc_req_arg_t;

extern  uint8_t license[22];

uint16_t mac_short_addr_get(uint64_t mac_addr);

uint8_t get_addr(pbuf_t *pbuf, mac_addr_mode_e mode, uint64_t *addr);
