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
 
typedef unsigned char osel_int_status_t;
static volatile int os_line = 0;

#define OSEL_INT_LOCK()                         
#define OSEL_INT_UNLOCK()                       

#define OSEL_ENTER_CRITICAL(x)                  \
do                                              \
{                                               \
    x = 0;                                      \
} while(__LINE__ == -1)

#define OSEL_EXIT_CRITICAL(x)                   x=x

#define CODE

#define OSEL_ISR_ENTRY()                        st(++osel_int_nest;)

#define OSEL_ISR_EXIT()                         \
do                          	                \
{                                               \
    if(--osel_int_nest == 0)                    \
    {                                           \
        if(osel_int_nest == 0)                  \
        {                                       \
            osel_schedule();                    \
        }                                       \
    }                                           \
} while (__LINE__ == -1)

#define OSEL_POST_SIGNAL(this)                  \
do                                              \
{                                               \
    osel_int_status_t status = 0;               \
    OSEL_ENTER_CRITICAL(status);                \
    if(osel_int_nest == 0)                      \
    {                                           \
        osel_schedule();                        \
    }                                           \
    OSEL_EXIT_CRITICAL(status);                 \
} while (__LINE__ == -1)

#define OSEL_POST_WAIT(this)

typedef unsigned char                           osel_bool_t;
typedef unsigned char                           osel_uint8_t;
typedef signed   char                           osel_int8_t;
typedef unsigned short                          osel_uint16_t;
typedef signed   short                          osel_int16_t;
typedef unsigned int                            osel_uint32_t;
typedef signed int                              osel_int32_t;

/* DATA TYPES (Compiler Specific) */
typedef char                                    char_t;   /* char type */
typedef unsigned char                           bool_t;   /* Boolean type */
typedef unsigned char                           uint8_t;  /* Unsigned  8 bit quantity  */
typedef signed   char                           int8_t;   /* Signed    8 bit quantity  */
typedef unsigned short                          uint16_t; /* Unsigned 16 bit quantity  */
typedef signed   short                          int16_t;  /* Signed   16 bit quantity  */
typedef unsigned int                            uint32_t; /* Unsigned 32 bit quantity  */
typedef signed   int                            int32_t;  /* Signed   32 bit quantity  */
typedef unsigned int                            uint;
typedef float                                   fp32_t;
typedef double                                  fp64_t;
typedef signed long                             int64_t;
typedef unsigned long                           uint64_t;


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
