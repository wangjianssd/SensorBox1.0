#pragma once
#include "common/lib/data_type_def.h"
#include "stack/common/sbuf.h"
#include "../../general/general.h"
#include "sync_define.h"


typedef struct
{
    void (*recv_cb)(sbuf_t *const sbuf);
    void (*send_cb)(sbuf_t *const sbuf, bool_t state);
    void (*mac_assoc_cb)(bool_t state);
	void (*mac_restart)(net_mode_e mode);
} sync_cb_t;

extern sync_cb_t sync_cb;

struct sync_t
{
    bool_t (*run)(void);
    bool_t (*stop)(void);
    bool_t (*send)(sbuf_t *sbuf);
    bool_t (*find_sys_coord)(void);         /**< 寻找上级设备 */
    void   (*assoc_switch)(void);			/**< 关联状态机 */
    sync_info_t* (*get)(void);
};

extern const struct sync_t m_sync;
extern osel_etimer_t assoc_timer;
