#ifndef __HAL_RTC_H
#define __HAL_RTC_H

typedef struct
{
	uint8_t year;   //��
	uint8_t month;  //��
	uint8_t day;    //��
	uint8_t hour;   //ʱ
	uint8_t minute; //��
	uint8_t second; //��
}hal_rtc_t;

void hal_rtc_init();
void hal_rtc_get_time(hal_rtc_t *p_rtc);
bool_t hal_rtc_set_time(const hal_rtc_t *p_rtc);

#endif
