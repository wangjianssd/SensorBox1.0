/**
 * @brief       : 
 *
 * @file        : platform.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __PLATFORM_H
#define __PLATFORM_H

/** 需要定义平台
	PF_CORE_M3
	PF_CORE_430
	PF_ITS_RP
	PF_SEN_COMM
**/

#ifdef PF_CORE_M3
#include "platform/core_stm32l/drivers/driver.h"
#elif PF_CORE_430
#include "platform/core430/driver/driver.h"
#elif PF_ITS_RP
#include "platform/its_rp/driver/driver.h"
#elif PF_SEN_COMM
#include "platform/sen_comm/driver/driver.h"
#elif PF_CORE_POSIX
#include "platform/posix/driver/driver.h"
#else
#error "need define NODE_PF in proj defined symbols"
#endif



#endif
