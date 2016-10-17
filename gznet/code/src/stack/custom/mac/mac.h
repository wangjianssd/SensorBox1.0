#pragma once
#include "common/lib/data_type_def.h"
#include "stack/common/sbuf.h"
#include "general/general.h"
#include "core/sync/sync_ctrl.h"
#include "core/asyn/asyn_ctrl.h"
#include "general/assos_table.h"

#pragma pack(1)

typedef struct
{
	//public:
	uint8_t             ch[CH_NUM];
	net_mode_e          mode;
	uint8_t       drivce_type;
	mac_pib_t           mac_pib;
	struct 
	{
		asyn_attribute_t    asyn_attribute;
		sync_attribute_t	sync_attribute;
	} agreement;
	//private:
	net_mode_e          current_mode;
	mac_state_e       	state;
} mac_info_t;
#pragma pack()

typedef struct
{
	void (*recv_cb)(sbuf_t *const sbuf);
	void (*send_cb)(sbuf_t *const sbuf, bool_t state);
	void (*mac_assoc_cb)(bool_t state);
} mac_dependent_t;

bool_t mac_init(void);	//执行一次
bool_t mac_run(void);
bool_t mac_stop(void);
bool_t mac_send(sbuf_t *sbuf);
mac_info_t* mac_get(void);



/**<  提供给NKW的接口*/
extern mac_dependent_t mac_cb;
#define MAC_OFFSET_SIZE	(PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE + MAC_ADDR_SHORT_SIZE*2 +MAC_FCS_SIZE)
static inline void mac_dependent_cfg(mac_dependent_t *mac_cfg)/**< 注册回调函数 */
{
	mac_cb.recv_cb = mac_cfg->recv_cb;
	mac_cb.send_cb = mac_cfg->send_cb;
	mac_cb.mac_assoc_cb = mac_cfg->mac_assoc_cb;	
}
static inline void mac_beacon_sent_set(bool_t state)/**< 启动mac发送beacon */
{
	mac_state_e s = (state==TRUE) ? WORK_ON:WORK_DOWN;
	mac_info_t *info = mac_get();
	info->state = s;
}
static inline uint8_t mac_drivce_type(void)/**< 获取设备类型 */
{
	mac_info_t *info = mac_get();
	return (uint8_t)info->drivce_type;
}
static inline uint64_t mac_self_addr(void)/**< 获取设备长地址 */
{
	mac_info_t *info = mac_get();
	return info->mac_pib.mac_addr;
}
static inline uint16_t mac_self_saddr(void)/**< 获取设备短地址 */
{
	mac_info_t *info = mac_get();
	return mac_short_addr_get(info->mac_pib.mac_addr);
}
static inline uint16_t mac_pib_coord_short_addr_get(void)/**< 获取上级设备短地址 */
{
	mac_info_t *info = mac_get();
	return info->mac_pib.coord_saddr;
}
static inline uint16_t mac_pib_hops_get(void)/**< 获取网络跳数 */
{
	mac_info_t *info = mac_get();
	return info->mac_pib.hops;
}
static inline bool_t mac_assoc_device_find(uint64_t mac_addr)/**< 获取设备网络连接类型(直连OR中转) */
{
	return assoc_driver_find(mac_addr);
}

static inline bool_t mac_assoc_device_short_find(uint16_t mac_addr)
{
	return assos_sid_find(mac_addr);
}

static inline void mac_assoc_driver_remove(uint64_t mac_addr,mac_addr_mode_e mode)
{
	assos_table_remove(mac_addr,mode);
}
static inline void mac_assoc_again(uint16_t addr)/**< 重新关联 */
{
}
/**<  END*/



