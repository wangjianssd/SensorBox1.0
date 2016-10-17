/**
 * @brief       : this
 * @file        : board.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-05-05
 * change logs  :
 * Date       Version     Author        Note
 * 2015-05-05  v0.0.1  gang.cheng    first version
 */	

#include "sys_arch/osel_arch.h"
#include "common/lib/lib.h"
#include "driver.h"

#include <stdlib.h>

#ifndef PTHREAD_STACK_MIN
#define	PTHREAD_STACK_MIN	9012
#endif

static struct termios l_tsav;
sem_t sem_id;

static pthread_t tick_thread;
static bool_t tick_is_trigger = FALSE;

int hardware_int_init(	pthread_t *thread,
						thread_routine_t routine,
						void *arg)
{
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, (size_t)PTHREAD_STACK_MIN);

	if (pthread_create(thread, &attr, routine, arg) != 0)
	{
		pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
		param.sched_priority = 0;
		pthread_attr_setschedparam(&attr, &param);
		if (pthread_create(thread, &attr, routine, arg) != 0)
		{
			printf("software int simulator init failed\r\n");
			return -1;
		}
	}

	return 0;
}

static void* tick_thread_routine(void *arg)
{
	while(1)
	{
		struct timeval timeout = {0};
		timeout.tv_usec = MS_FOR_ONETICK*1000;	// 10MS
		select(0, 0, 0, 0, &timeout);
		tick_is_trigger= TRUE;
		sem_post(&sem_id);
	}

	return NULL;
}

static void board_idle_handle(void *p)
{
	sem_wait(&sem_id);
	if(tick_is_trigger)
	{
		tick_is_trigger = FALSE;
		wsnos_ticks();
	}

	//*< 这里是所有外部中断的处理函数
	//linux_terminal_irq_handle();
	//linux_rfid_irq_handle();

	osel_schedule();
}

void board_init(void)
{
	struct termios tio;
	tcgetattr(0, &l_tsav);
	tcgetattr(0, &tio);

	tio.c_cflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &tio);

	sem_init(&sem_id, 0, 0);
	osel_idle_hook(board_idle_handle);
	hardware_int_init(&tick_thread, tick_thread_routine, NULL);

	debug_init(DBG_LEVEL_INFO | DBG_LEVEL_WARNING);
}

void led_init(void)
{
	;
}

void board_reset(void)
{
	DBG_LOG(DBG_LEVEL_INFO, "board reset\r\n");
	exit(0);
}

void srand_arch(void)
{
	
}

void lpm_arch(void)
{

}
