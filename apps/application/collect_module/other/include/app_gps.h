#ifndef _APP_GPS_H_
#define _APP_GPS_H_
void box_gps_data_event_handle(void *arg);
void box_gps_init(uint32_t period);
void box_gps_vcc_open(void);
void box_gps_vcc_close(void);
void box_gps_timer_start(void);//周期性上报位置信息所需要的定时器
void box_gps_timer_end(void);
void box_gps_timer_out_event_handle(void *arg);
#endif