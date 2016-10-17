/**
* @brief       : 
*
* @file        : stack.c
* @author      : gang.cheng
* @version     : v0.0.1
* @date        : 2015/5/7
*
* Change Logs  :
*
* Date        Version      Author      Notes
* 2015/5/7    v0.0.1      gang.cheng    first version
*/

#include <lib.h>
#include <prim.h>
#include <pbuf.h>
#include <sbuf.h>
#include <stack.h>
#include <hal_board.h>
#include <app.h>
#include <dev.h>

void stack_init(void)
{
    pbuf_init();
    bool_t res = sbuf_init();
    DBG_ASSERT(res != FALSE __DBG_LINE);
    
    SSN_RADIO.init();
    
	app_init();
}
