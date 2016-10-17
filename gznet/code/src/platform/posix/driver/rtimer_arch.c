/**
 * @brief       : this
 * @file        : rtimer_arch.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-20
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-20  v0.0.1  gang.cheng    first version
 */
#include "sys_arch/osel_arch.h"
#include "common/lib/lib.h"
#include "driver.h"

#include <stdlib.h>

static pthread_t rtimer_thread;
uint16_t hardware_cnt = 0;	//*< Simulator hardware register
uint16_t compare_cnt = 0;   //*< simulator compare register
bool_t compare_flag = FALSE;
static void *rtimer_thread_routine(void *arg)
{
	while(1)
	{
		struct timeval timeout = {0};
		timeout.tv_usec = TICK_TO_US(1);	// 31us
		select(0, 0, 0, 0, &timeout);
		hardware_cnt++;
	}
}

void rtimer_start(void)
{
	hardware_int_init(&rtimer_thread, rtimer_thread_routine, NULL);
}
