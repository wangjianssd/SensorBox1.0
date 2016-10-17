/**
 * @brief       : 
 *
 * @file        : wsnos.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/30
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/30    v0.0.1      gang.cheng    first version
 */

#ifndef __WSNOS_H__
#define __WSNOS_H__

#include "wsnos_config.h"
#include "wsnos_port.h"

#include "wsnos_default.h"


#include "wsnos_equeue.h"
#include "wsnos_mem.h"
#include "wsnos_task.h"
#include "wsnos_pthread.h"
#include "wsnos_etimer.h"
#include "wsnos_sched.h"


void osel_env_init(osel_uint8_t *buf, 
                   osel_uint16_t size,
                   osel_uint8_t max_prio);
 
#endif

