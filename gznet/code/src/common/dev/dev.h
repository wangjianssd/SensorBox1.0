/**
 * @brief       : 
 *
 * @file        : dev.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/10/20
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/20    v0.0.1      gang.cheng    first version
 */
#ifndef __DEV_H__
#define __DEV_H__

#include "common/lib/lib.h"

#ifdef SILABS_RADIO_SI443X
#include "si4432/radio.h"
#elif SILABS_RADIO_SI446X
#include "si4463/radio.h"
#elif SIM_RADIO
#include "sim_radio/radio.h"
#else
#error "must define radio_type in node_cfg.h"
#endif

#include "radio_defs.h"
#include "gprs_defs.h"
#endif

