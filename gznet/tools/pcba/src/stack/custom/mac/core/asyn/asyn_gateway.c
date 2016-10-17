#include "asyn_gateway.h"
#include "asyn_general.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
#include <hal_board.h>
asyn_gateway_cb_t asyn_gateway_cb;
static asyn_info_t info;
static mac_state_e get_state(void)
{
    return *info.state;
}
static void set_state(mac_state_e state)
{
    *info.state = state;
}

static void recv_slot_cb(void *seq_p)
{
	hal_led_open(HAL_LED_GREEN);
    m_tran_recv();
}

static void sleep_slot_cb(void *seq_p)
{
	hal_led_close(HAL_LED_GREEN);
    m_tran_sleep();
}

static void idle_slot_cb(void *seq_p)
{
}

static void slot_cb_init(void)
{
    info.asyn_pib->time_slot[ASYN_RECV] = recv_slot_cb;
    info.asyn_pib->time_slot[ASYN_SLEEP] = sleep_slot_cb;
    info.asyn_pib->time_slot[ASYN_IDLE] = idle_slot_cb;
}

static void mac_schedule_init(void)
{
    asyn_config(0x00);
    asyn_cfg(info.asyn_pib, SLOT_LOCAL_TIME);
    hal_time_t now;
    now = hal_timer_now();
    now.w += MS_TO_TICK(1000);
    m_slot_run(&now);
}

static bool_t run(void)
{
    set_state(WORK_ON);
    //mac_frames_cb_init();
    //frame_switch_init(mac_beacon,mac_data,mac_ack,mac_command);
    slot_cb_init();
    mac_schedule_init();
    return TRUE;
}
static bool_t stop(void)
{
    set_state(WORK_DOWN);
    return TRUE;
}
static bool_t gateway_send(sbuf_t *sbuf)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    switch (sbuf->primtype)
    {
    case N2M_DATA_REQUEST:
    {
#if NODE_TYPE == NODE_TYPE_COLLECTOR
        south_sbuf_fill(sbuf);
#else
        mac_data_fill_package(sbuf);
#endif
    }
    break;
    default:
        return FALSE;
    }
    return TRUE;
}

static asyn_info_t* get(void)
{
    return &info;
}

const struct asyn_gateway_t asyn_gateway =
{
    run,
    stop,
    gateway_send,
    get,
};
