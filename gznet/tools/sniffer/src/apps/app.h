#pragma once


#define         PRINTF_TASK_PRIO        (2u)
#define         APP_TASK_PRIO           (3u)


typedef enum
{
    APP_SERIAL_EVENT =  ((APP_TASK_PRIO<<8) | 0x01),
    APP_SERIAL_DATA_EVENT,
    APP_SOUTH_DATA_EVENT,
} app_task_sig_enum_t;

typedef enum
{
    PRINTF_SNFFIER_EVENT =  ((PRINTF_TASK_PRIO<<8) | 0x01),
} printf_task_sig_enum_t;

void app_init(void);