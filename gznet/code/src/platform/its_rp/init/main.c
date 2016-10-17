/**
 * @brief       : 
 *
 * @file        : main.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/hal/hal.h"
#include "common/lib/lib.h"
#include "sys_arch/wsnos/wsnos.h"
#include "stack/stack.h"

static uint8_t osel_heap_buf[OSEL_HEAP_SIZE];

int16_t main(void)
{
	/*开启了看门狗52s后复位*/
#ifdef NDEBUG
    bootloader_init();
#endif

	osel_env_init(osel_heap_buf, OSEL_HEAP_SIZE, OSEL_MAX_PRIO);

	hal_board_init();

	stack_init();

	osel_run();
    
	return 0;
}
