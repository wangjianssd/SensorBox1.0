/*
 * main.c
 *
 *  Created on: 2015年7月10日
 *      Author: uto
 */

#include <lib.h>
#include <driver.h>
#include <wsnos.h>

#include "stdio.h"

extern void test_init(void);

int main(void)
{
	osel_init();

	board_init();

	test_init();

	osel_run();

	return 0;
}

