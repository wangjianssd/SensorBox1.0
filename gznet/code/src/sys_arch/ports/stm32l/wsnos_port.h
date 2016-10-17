/**
 *  这个文件定义了OS与硬件之间的一些配置以及OS需要的数据类型结构
 *
 * file : wsnos_port.h
 *
 * Date : 2011--8--04
 *
**/

#ifndef __WSNOS_PORT_H
#define __WSNOS_PORT_H

#include "platform/core_stm32l/libs/cmsis/stm32l1xx.h"
typedef unsigned char osel_int_status_t;
static volatile int os_line = 0;

/// Cortex M3
#define OSEL_INT_LOCK()                         __set_BASEPRI(WSNOS_BASEPRI)
#define OSEL_INT_UNLOCK()                       __set_BASEPRI(0U)

/* NOTE: keep in synch with the value defined in "wsnos_port.s" */
#define WSNOS_BASEPRI                           (0xFF >> 2)

#define OSEL_ENTER_CRITICAL(s)                  s = interrupt_disable()    
#define OSEL_EXIT_CRITICAL(s)                   interrupt_enable(s)

#define CODE
#define NOP()                                   _NOP()

#define OSEL_ISR_ENTRY()                        

#define NVIC_INT_CTRL   0xE000ED04              // interrupt control state register
#define NVIC_PENDSVSET  0x10000000              // value to trigger PendSV exception
#define OSEL_ISR_EXIT()                         \
do                                              \
{                                               \
    osel_int_status_t status = 0;               \
    OSEL_ENTER_CRITICAL(status);                \
    *(uint32_t *)(NVIC_INT_CTRL) = NVIC_PENDSVSET;\
    OSEL_EXIT_CRITICAL(status);                 \
} while (__LINE__ == -1)

#define OSEL_POST_SIGNAL(this)                  \
do                                              \
{                                               \
    osel_int_status_t status = 0;               \
    OSEL_ENTER_CRITICAL(status);                \
    *(uint32_t *)(NVIC_INT_CTRL) = NVIC_PENDSVSET;\
    OSEL_EXIT_CRITICAL(status);                 \
} while (__LINE__ == -1)

#define OSEL_POST_WAIT(this)

#define OSEL_NO_INIT                            __no_init


typedef unsigned char                           osel_bool_t;
typedef unsigned char                           osel_uint8_t;
typedef signed   char                           osel_int8_t;
typedef unsigned int                            osel_uint16_t;
typedef signed   int                            osel_int16_t;
typedef unsigned long                           osel_uint32_t;
typedef signed long                             osel_int32_t;

typedef char                                    char_t;   /* char type */
typedef unsigned char                           bool_t;   /* Boolean type */
typedef unsigned int                            uint;
typedef float                                   fp32_t;
typedef double                                  fp64_t;

enum
{
    OSEL_FALSE,
    OSEL_TRUE
};

uint32_t interrupt_disable();
void interrupt_enable(uint32_t level);

void osel_start(void);

void osel_exit(void);

void osel_eoi(void);

#endif
