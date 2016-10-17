/**
 *	这个文件定义了OS与硬件之间的一些配置以及OS需要的数据类型结构
 *
 * file	: wsnos_port.h
 *
 * Date	: 2011--8--04
 *
**/

#ifndef __WSNOS_PORT_H
#define __WSNOS_PORT_H
#include <msp430.h>
typedef unsigned char osel_int_status_t;
static volatile int os_line = 0;

#define OSEL_INT_LOCK()                         (_DINT())
#define OSEL_INT_UNLOCK()                       (_EINT())

#define OSEL_ENTER_CRITICAL(x)                  \
do                                              \
{                                               \
    if (__get_SR_register()&GIE)                \
    {                                           \
        x = 1;                                  \
        _DINT();                                \
    }                                           \
    else                                        \
    {                                           \
        x = 0;                                  \
    }                                           \
} while(__LINE__ == -1)

#define OSEL_EXIT_CRITICAL(x)    st(if ((x) == 1) _EINT();)

#define CODE

#define OSEL_ISR_ENTRY()                        st(++osel_int_nest;)

#define OSEL_ISR_EXIT()                         \
do                          	                \
{                                               \
    if(--osel_int_nest == 0)                    \
    {                                           \
        osel_int_status_t status = 0;           \
        OSEL_ENTER_CRITICAL(status);            \
        if(osel_int_nest == 0)                  \
        {                                       \
            osel_schedule();                    \
        }                                       \
        OSEL_EXIT_CRITICAL(status);             \
    }                                           \
} while (__LINE__ == -1)

#define OSEL_POST_SIGNAL(this)                  \
do                                              \
{                     	                        \
    osel_int_status_t status = 0;               \
    OSEL_ENTER_CRITICAL(status);                \
    if(osel_int_nest == 0)                      \
    {                                           \
        osel_schedule();                        \
    }                                           \
    OSEL_EXIT_CRITICAL(status);                 \
} while (__LINE__ == -1)

#define OSEL_POST_WAIT(this)


#define OSEL_DELAY(x)                           sleep(x)

typedef unsigned char                           osel_bool_t;
typedef unsigned char                           osel_uint8_t;
typedef signed   char                           osel_int8_t;
typedef unsigned int                            osel_uint16_t;
typedef signed   int                            osel_int16_t;
typedef unsigned long                           osel_uint32_t;
typedef signed long                             osel_int32_t;


/* DATA TYPES (Compiler Specific) */
typedef char                                    char_t;   /* char type */
typedef unsigned char                           bool_t;   /* Boolean type */
typedef unsigned char                           uint8_t;  /* Unsigned  8 bit quantity  */
typedef signed   char                           int8_t;   /* Signed    8 bit quantity  */
typedef unsigned int                            uint16_t; /* Unsigned 16 bit quantity  */
typedef signed   int                            int16_t;  /* Signed   16 bit quantity  */
typedef unsigned long                           uint32_t; /* Unsigned 32 bit quantity  */
typedef signed   long                           int32_t;  /* Signed   32 bit quantity  */
typedef unsigned int                            uint;
typedef float                                   fp32_t;
typedef double                                  fp64_t;
typedef signed long long                        int64_t;
typedef unsigned long long                      uint64_t;

enum
{
    OSEL_FALSE,
    OSEL_TRUE
};

extern volatile osel_uint32_t osel_int_nest;

void osel_start(void);

void osel_exit(void);

void osel_eoi(void);

#endif
