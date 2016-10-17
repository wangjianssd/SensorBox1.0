/**
 *	这个文件是OS的一些配置
 *
 * file	: wsnos_config.h
 *
 * Date	: 2011--8--04
 *
**/
#ifndef __WSNOS_CONFIG_H
#define __WSNOS_CONFIG_H

#define OSEL_DBG_EN                     (1u)    // 断言宏开关

#define OSEL_MEM_ALIGNMENT				(4u)    // CPU默认字节对齐可选1、2、4

#define OSEL_MAX_PRIO                   (8u) 	// 最大优先级

#define OSEL_HEAP_SIZE					(7000u)  // 静态堆栈大小

#define OSEL_TICK_RATE_HZ               (100u)
#define OSEL_TICK_PER_MS                (1000/OSEL_TICK_RATE_HZ)


#if ((OSEL_MAX_PRIO == 0) || (OSEL_MAX_PRIO > 64))
#error "OSEL_MAX_PRIO 必须在1-64之间选择。"
#endif

#endif

