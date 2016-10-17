/**
 *	这个文件定义了OS与硬件之间的一些配置
 *
 * file	: wsnos_port.c
 *  
 * Date	: 2011--8--04
 *  
**/

#include <wsnos_port.h>
volatile osel_uint32_t osel_int_nest;
void osel_start(void)
{
    OSEL_INT_UNLOCK();
}

void osel_exit(void)
{
    ;
}

void osel_eoi(void)
{
    ;
}

