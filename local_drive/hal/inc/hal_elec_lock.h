 /**
 * @brief       : 感知箱锁的控制
 *
 * @file        : hal_elec_lock.h
 * @author      : xxx
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */

#ifndef _HAL_ELEC_LOCK_H_
#define _HAL_ELEC_LOCK_H_
#include <gznet.h>
#include "elec_lock.h"
//#include <data_type_def.h>

void hal_elec_lock_init(void);
bool_t hal_elec_lock_open(void);
void hal_elec_lock_close(void);
bool_t hal_elec_lock_state(void);
#endif