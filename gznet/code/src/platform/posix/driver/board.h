/**
 * @brief       : this
 * @file        : board.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-05-05
 * change logs  :
 * Date       Version     Author        Note
 * 2015-05-05  v0.0.1  gang.cheng    first version
 */
#ifndef __BOARD_H__
#define __BOARD_H__

#include "pthread.h"
#include "termios.h"	// termios

#include "limits.h"		// PTHREAD_STACK_MIN
#include "stdio.h"

#include "semaphore.h"	// sem_t

#include "sys/time.h"	// struct timeval
#include "sys/select.h"
#include "unistd.h"

#define TICK_PER_SECOND          100ul
#define MS_FOR_ONETICK			 (1000/TICK_PER_SECOND)

#define BLUE              (1u)
#define RED               (2u)
#define GREEN             (3u)

#define LED_OPEN(color)   
#define LED_CLOSE(color)   
#define LED_TOGGLE(color)  

extern sem_t sem_id;

typedef void * (*thread_routine_t)(void *arg);

int hardware_int_init(	pthread_t *thread,
						thread_routine_t routine,
						void *arg);

void board_init(void);

void led_init(void);

void board_reset(void);

void srand_arch(void);

void lpm_arch(void);

#endif
