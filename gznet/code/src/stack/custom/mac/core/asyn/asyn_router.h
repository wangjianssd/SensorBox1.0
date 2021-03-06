#pragma once
#include "asyn_define.h"
typedef struct
{
    void (*recv_cb)(sbuf_t *const sbuf);
    void (*send_cb)(sbuf_t *const sbuf, bool_t state);
    void (*mac_assoc_cb)(bool_t state);
    void (*mac_restart)(net_mode_e mode);
} asyn_router_cb_t;

extern asyn_router_cb_t asyn_router_cb;

struct asyn_router_t
{
    bool_t (*run)(void);
    bool_t (*stop)(void);
    bool_t (*send)(sbuf_t *sbuf);
    asyn_info_t* (*get)(void);
};

extern const struct asyn_router_t asyn_router;
void router_init(void);