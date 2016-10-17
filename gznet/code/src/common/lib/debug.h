/**
 * @brief       : 
 *
 * @file        : debug.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __DEBUG_H
#define __DEBUG_H

#include <assert.h>

#include "common/lib/printf.h"
#include <string.h>
#include <stdio.h>

#include "common/lib/data_type_def.h"

//#define DEBUG_INFO_PRINT_EN                 (1)

#define DBG_LEVEL_NONE		                0x00
#define DBG_LEVEL_TRACE		                (0x01<<0)
#define DBG_LEVEL_INFO		                (0x01<<1)
#define DBG_LEVEL_WARNING	                (0x01<<2)
#define DBG_LEVEL_ERROR		                (0x01<<3)
#define DBG_LEVEL_CRITICAL	                (0x01<<4)
#define DBG_LEVEL_ORIGIN                    (0x01<<5)
/**
 * 调试等级设置,只有消息与更高水平
 */
extern uint8_t global_debug_level;

bool_t debug_enter_queue(uint8_t *string, uint8_t len);
void debug_sqqueue_init();

#include "node_cfg.h"

#if DEBUG_INFO_PRINT_EN > 0

#define _DBG_LINE_         
#define __DBG_LINE

/** define file name */
#define DBG_THIS_FILE                                               \
static char const l_this_file[] = __FILE__;

#define DBG_THIS_MODULE(name_)                                      \
static char const l_this_file[] = name_;

void debug_log(uint8_t dbg_lev, const char *fn, uint16_t line, ...);

#define __FILENAME__            (l_this_file)

#define DBG_LOG(level, ...)                                         \
    do                                                              \
    {                                                               \
        debug_log(level, __FILENAME__, __LINE__, __VA_ARGS__);      \
    }while(__LINE__ == -1)
     
#else
/*形参*/
#define _DBG_LINE_  	, uint16_t line
/*实参*/
#define __DBG_LINE  	, __LINE__

#define DBG_THIS_FILE

#define DBG_THIS_MODULE(name_)
        
#define DBG_LOG(level, ...)

#endif

void DBG_ASSERT(bool_t cond _DBG_LINE_);

void debug_init(uint8_t debug_lev);

void debug_info_printf(void);

uint16_t debug_info_get(void);

void debug_info_clr(void);

void debug_info_set(uint16_t debug_line);
#endif


