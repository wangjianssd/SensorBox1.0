/*
 * main.c
 *
 *  Created on: 2015年7月10日
 *      Author: uto
 */

#include <gznet.h>

#include "stdio.h"

static uint8_t osel_heap_buf[OSEL_HEAP_SIZE];


int main(void)
{
	osel_env_init(osel_heap_buf, OSEL_HEAP_SIZE, OSEL_MAX_PRIO);

	board_init();

	stack_init();
	printf("demo4\n");

	osel_run();

	return 0;
}

