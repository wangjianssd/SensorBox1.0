/**
 * @brief       : 这个文件定义了OS与硬件之间的一些配置
 *
 * @file        : wsnos_port.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <wsnos_port.h>

volatile osel_uint32_t osel_int_nest = 0;

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

