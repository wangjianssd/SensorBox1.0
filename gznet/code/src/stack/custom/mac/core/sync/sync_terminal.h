#pragma once
#include "sync_define.h"
#include "sync_ctrl.h"

#define SYNC_LOST_CNT           (10)    //*< 丢失多少个信标以后算同步丢失

typedef struct
{
    void (*recv_cb)(sbuf_t *const sbuf);
    void (*send_cb)(sbuf_t *const sbuf, bool_t state);
    void (*mac_assoc_cb)(bool_t state);
	void (*mac_restart)(net_mode_e mode);
} terminal_cb_t;

extern terminal_cb_t terminal_cb;

struct terminal_t
{
    bool_t (*run)(void);
    bool_t (*stop)(void);
    bool_t (*send)(sbuf_t *sbuf);
    sync_info_t* (*get)(void);
};

extern const struct terminal_t terminal;
