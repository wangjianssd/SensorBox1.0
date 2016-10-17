#ifndef __GPRS_H
#define __GPRS_H

#include "stdio.h"
#include "common/lib/lib.h"
#include "common/lib/printf.h"
#include "platform/platform.h"
#include "sys_arch/osel_arch.h"
#include "common/dev/sim928a/gprs_tx.h"
#include "common/dev/sim928a/gprs_rx.h"
#include "common/dev/sim928a/gprs_str.h"

#define GPRS_TASK_PRIO       (4u)


typedef enum
{
/** USER */
    GPRS_SEND_EVENT,
	GPRS_STATE_TRANS_EVENT,
	GPRS_UART_EVENT,
    GPRS_TIMEOUT_EVENT,
    GPRS_NO_DATA_EVENT,
    
} gprs_task_sig_enum_t;

PROCESS_NAME(gprs_event_process);
#endif