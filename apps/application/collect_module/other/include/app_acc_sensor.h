#ifndef _APP_ACC_SENSOR_H_
#define _APP_ACC_SENSOR_H_


void box_enable_acc_alg(void);
void box_acc_data_timer_event_handle(void *arg);
void box_disable_acc_alg(void);

void buzzer_cycle_timer_start(uint32_t args);
void buzzer_cycle_timer_out_event_handle(void);

#endif