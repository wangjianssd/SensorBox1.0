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

#define OSEL_DBG_EN                     (0u)    // 断言宏开关
#define OSEL_EPOOL_DBG_EN               (0u)    // 消息池调试宏开关
#define OSEL_EBLOCK_DBG_EN              (0u)    // 事件控制块调试宏开关
#define OSEL_USE_MULTIPLES              (1u)    // 是否支持一个消息对应多个任务

#define OSEL_STATIC_MEM_EN              (1u)    // 静态内存管理宏开关
#define OSEL_MEM_ALIGNMENT				(4u)    // CPU默认字节对齐可选1、2、4
#define OSEL_HEAP_SIZE					(6000u)  // 静态堆栈大小

#define OSEL_EPOOL_NUM					(5u)    // 消息总数
#define OSEL_EBLOCK_NUM					(20u)   // 事件块总数

#define OSEL_MAX_PRIO                   (8u) 	// 最大优先级

#define OSEL_NO_INIT

#if ((OSEL_MAX_PRIO == 0) || (OSEL_MAX_PRIO > 64))
#error "OSEL_MAX_PRIO 必须在1-64之间选择。"
#endif

#if OSEL_DBG_EN > 0
#define OSEL_ASSERT(test_)		if (!(test_))               \
{                                                           \
    while(1);                                               \
}
#else
#define OSEL_ASSERT(test_)      if (!(test_))               \
{                                                           \
   	;                                       				\
}
#endif

#endif

