#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "driver.h"
#include "rtc.h"

int SetRTCYEAR(int year);
int SetRTCMON(int mon);
int SetRTCDAY(int day);
int SetRTCHOUR(int hour);
int SetRTCMIN(int min);
int SetRTCSEC(int sec);

void rtc_get_time(rtc_t *p_rtc)
{
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
    while(!(RTCCTL1 & RTCRDY_H));
    p_rtc->second = RTCSEC;
    p_rtc->minute = RTCMIN;
    p_rtc->hour = RTCHOUR;
    p_rtc->day = RTCDAY;
    p_rtc->month = RTCMON;
    p_rtc->year = RTCYEAR % 100;
    OSEL_EXIT_CRITICAL(s);
}

bool_t rtc_set_time(const rtc_t *p_rtc)
{
    if(   (p_rtc->second > 59)
       || (p_rtc->minute > 59)
       || (p_rtc->hour > 23)
       || (p_rtc->day > 31)
       || (p_rtc->month > 12)
       || (p_rtc->year > 99)
          )
    {
        return FALSE;
    }
    osel_int_status_t s;
    
	OSEL_ENTER_CRITICAL(s);
    SetRTCSEC(p_rtc->second);
    SetRTCMIN(p_rtc->minute);
    SetRTCHOUR(p_rtc->hour);
    SetRTCDAY(p_rtc->day);
    SetRTCMON(p_rtc->month);
    SetRTCYEAR(p_rtc->year + 2000);
    OSEL_EXIT_CRITICAL(s);
    
    return TRUE;
}

void rtc_init()
{
//    RTCCTL1 = RTCMODE_H;
    RTCCTL1 = 0x0020;
    SetRTCSEC(0);
    SetRTCMIN(52);
    SetRTCHOUR(17);
    SetRTCDAY(19);
    SetRTCMON(12);
    SetRTCYEAR(13 + 2000);
}
