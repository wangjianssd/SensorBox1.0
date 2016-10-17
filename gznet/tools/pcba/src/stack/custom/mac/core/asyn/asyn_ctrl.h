#pragma once
#include "../../general/general.h"
#include "asyn_frame.h"
#include "asyn_define.h"
#include <sbuf.h>

typedef struct
{
    void (*recv_cb)(sbuf_t *const sbuf);
    void (*send_cb)(sbuf_t *const sbuf, bool_t state);
    void (*mac_assoc_cb)(bool_t state);
    void (*mac_restart)(net_mode_e mode);
} asyn_cb_t;

extern asyn_cb_t asyn_cb;

struct asyn_t
{
    bool_t (*run)(void);
    bool_t (*stop)(void);
    bool_t (*send)(sbuf_t *sbuf);
    bool_t (*find_sys_coord)(void);         /**< 寻找上级设备 */
    void (*mac_task_register)(osel_task_t *task);
    asyn_info_t* (*get)(void);
};

extern const struct asyn_t asyn;
