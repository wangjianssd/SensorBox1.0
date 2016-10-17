/**
 * @brief       : 
 *
 * @file        : srandom.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "platform/platform.h"
#include "common/lib/debug.h"
#include <stdlib.h>

uint16_t srandom(uint16_t min, uint16_t max)         //添加stdlib.h 头文件
{
    DBG_ASSERT(max >= min __DBG_LINE);
    srand_arch();
    return (min + rand()%(max-min+1));
}
