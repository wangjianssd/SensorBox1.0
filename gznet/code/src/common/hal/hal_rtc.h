#ifndef __HAL_RTC_H
#define __HAL_RTC_H

typedef struct
{
	uint8_t year;   //年
	uint8_t month;  //月
	uint8_t day;    //日
	uint8_t hour;   //时
	uint8_t minute; //分
	uint8_t second; //秒
}hal_rtc_t;

void hal_rtc_init();
void hal_rtc_get_time(hal_rtc_t *p_rtc);
bool_t hal_rtc_set_time(const hal_rtc_t *p_rtc);

#endif
