#ifndef __RTC_H
#define __RTC_H

typedef struct
{
	uint8_t year;   //��
	uint8_t month;  //��
	uint8_t day;    //��
	uint8_t hour;   //ʱ
	uint8_t minute; //��
	uint8_t second; //��
}rtc_t;

void rtc_init();
void rtc_get_time(rtc_t *p_rtc);
bool_t rtc_set_time(const rtc_t *p_rtc);

#endif