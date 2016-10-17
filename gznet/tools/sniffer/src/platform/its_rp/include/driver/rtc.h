#ifndef __RTC_H
#define __RTC_H

typedef struct
{
	uint8_t year;   //年
	uint8_t month;  //月
	uint8_t day;    //日
	uint8_t hour;   //时
	uint8_t minute; //分
	uint8_t second; //秒
}rtc_t;

void rtc_init();
void rtc_get_time(rtc_t *p_rtc);
bool_t rtc_set_time(const rtc_t *p_rtc);

#endif