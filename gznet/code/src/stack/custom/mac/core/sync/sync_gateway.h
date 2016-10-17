#pragma once
#include "sync_define.h"
#include "sync_ctrl.h"
typedef struct
{
    void (*recv_cb)(sbuf_t *const sbuf);
    void (*send_cb)(sbuf_t *const sbuf, bool_t state);
    void (*mac_assoc_cb)(bool_t state);
} gateway_cb_t;

extern gateway_cb_t gateway_cb;

struct gateway_t
{
    bool_t (*run)(void);
    bool_t (*stop)(void);
    bool_t (*send)(sbuf_t *sbuf);
    sync_info_t* (*get)(void);
};

extern const struct gateway_t gateway;
