#ifndef _APP_LIGHT_SENSOR_H_
#define _APP_LIGHT_SENSOR_H_

#define LIGHT_SENSOR_T1_TIMER   500
#define LIGHT_SENSOR_T2_TIMER   200
#define LIGHT_SENSOR_STATUE_THRESH_L1   20
#define LIGHT_SENSOR_STATUE_THRESH_L2   20
#define LIGHT_SENSOR_COUNT_THRESH_C1   5

void box_light_sensor_t1_timer_handle(void *arg);
void box_light_sensor_t2_timer_handle(void *arg);


void box_light_sensor_init();
void box_light_sensor_open();
void box_light_sensor_close();
#endif