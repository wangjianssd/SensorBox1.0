#pragma once

#include "common/hal/hal.h"
#include "common/dev/dev.h"
#include "stack/common/pbuf.h"
#include "sys_arch/osel_arch.h"

#define APP_TASK_PRIO       (3u)

typedef enum
{
    APP_AT_RECV =  ((APP_TASK_PRIO<<8) | 0x01),
	APP_AT_SEND,
    APP_UART_EVENT,
    APP_CYCLE_TIMER_EVENT,
    APP_GPRS_DATA_SENT_START_EVENT,
    APP_GPRS_RESTART_EVENT,
    APP_RF_DATA_EVENT,
} app_task_sig_enum_t;

PROCESS_NAME(app_process);

void app_init(void);

