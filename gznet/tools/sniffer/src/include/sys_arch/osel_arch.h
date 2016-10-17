/**
 * @brief       :
 *
 * @file        : osel_arch.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __OSEL_ARCH_H
#define __OSEL_ARCH_H

#ifndef NO_SYS
#define NO_SYS                          1
#endif

/* you should implement this API by RTOS, if it has made up, do nothing */
#if (NO_SYS == 0)
// ...
void osel_post(osel_task_t *task, osel_pthread_t *p, osel_event_t *event);

#else

#include <wsnos.h>

#endif

#endif
