#include "common/hal/hal.h"
#include "platform/platform.h"
//#include <wsnos.h>
//#include <driver.h>
#include "platform/core430/driver/rtc.h"
#include "common/hal/hal_rtc.h"

void hal_rtc_init()
{
    rtc_init();
}

void hal_rtc_get_time(hal_rtc_t *p_rtc)
{
    rtc_t rtc;
    
    rtc_get_time(&rtc);
    p_rtc->year = rtc.year;
    p_rtc->month = rtc.month;
    p_rtc->day = rtc.day;
    p_rtc->hour = rtc.hour;
    p_rtc->minute = rtc.minute;
    p_rtc->second = rtc.second;
}
bool_t hal_rtc_set_time(const hal_rtc_t *p_rtc)
{
    rtc_t rtc;
    
    rtc.year = p_rtc->year;
    rtc.month = p_rtc->month;
    rtc.day = p_rtc->day;
    rtc.hour = p_rtc->hour;
    rtc.minute = p_rtc->minute;
    rtc.second = p_rtc->second;
    
    return rtc_set_time(&rtc);
}
