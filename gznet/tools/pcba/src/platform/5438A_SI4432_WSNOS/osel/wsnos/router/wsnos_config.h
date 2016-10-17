/**
 * @brief       : 这个文件是OS的一些配置
 *
 * @file        : wsnos_config.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 * 2015-10-10  v0.0.2      gang.cheng    delete some configs
 */

#ifndef __WSNOS_CONFIG_H
#define __WSNOS_CONFIG_H

#define OSEL_DBG_EN                     (1u)    // 断言宏开关

#define OSEL_MEM_ALIGNMENT				(4u)    // CPU默认字节对齐可选1、2、4

#define OSEL_MAX_PRIO                   (8u) 	// 最大优先级

#define OSEL_HEAP_SIZE					(7000u)  // 静态堆栈大小

#define OSEL_NO_INIT

#if ((OSEL_MAX_PRIO == 0) || (OSEL_MAX_PRIO > 64))
#error "OSEL_MAX_PRIO 必须在1-64之间选择。"
#endif

#endif