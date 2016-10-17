#ifndef _APP_TASK_
#define _APP_TASK_

#define APP_TASK_PRIO       (2u)

typedef enum
{
    BOX_ACC_DATA_TIMER_EVENT = ((APP_TASK_PRIO<<8) | 0x01),
    BOX_GPS_DATA_EVENT,
    BOX_HUMITURE_TIMER_EVENT,     
    BOX_LED_TIMER_EVENT,
    BOX_HEARTBEAT_TIMER_EVENT,
	BOX_NFC_INT_EVENT,
    BOX_POWER_TIMER_1_EVENT,
    BOX_LIGHT_T1_TIMER_EVENT,
    BOX_LIGHT_T2_TIMER_EVENT,
    BOX_DATA_SENT_EVENT,
    BOX_WAIT_ACK_TIMER_EVENT,
    BOX_DATA_RECEIVED_EVENT,
    BOX_SSN_CONTROL_EVENT,
    BOX_GPS_TIMER_EVENT,
    BOX_GPRS_STOP_EVENT,//测试用
    BOX_FRIGNER_INT_EVENT, //ָ�ƿ����ϱ���Ϣ
    //BOX_BLU_DATA_SENT_EVENT,
	//BLU_NO_DATA_EVENT,
	BUZZER_TIMER_EVENT,
    NFC_NO_LOCK_EVENT,
    LOCK_NO_PASSWORD_EVENT,
    EXTERN_LOCK_LOCK_EVENT,
    EXTERN_LOCK_FROZEN_EVENT,
} app_task_sig_enum_t;

PROCESS_NAME(app_task_thread_process);
void app_task_init(void);

#endif