/**
 * @brief       :
 *
 * @file        : data_type_def.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __DATA_TYPE_DEF_H
#define __DATA_TYPE_DEF_H

#include "sys_arch/osel_arch.h"

#define DEBUG_PRINT                   (0u)  //串口打印

#ifndef STATIC
#define STATIC                  static
#endif

#ifndef NULL
#define NULL                    ((void*)0)
#endif

#ifndef TRUE
#define TRUE                    (uint8_t)1
#endif

#ifndef FALSE
#define FALSE                   (uint8_t)0
#endif

#ifndef SUCCESS
#define SUCCESS                 (uint8_t)1
#endif

#ifndef FAILE
#define FAILE                   (uint8_t)0
#endif

#ifndef __INLINE
#define __INLINE
#endif

#ifndef FAILED
#define FAILED                  (uint8_t)0
#endif

/**
 * STANDARD BITS
 */
#ifndef BIT0
#define BIT0                    (0x01u)
#define BIT1                    (0x02u)
#define BIT2                    (0x04u)
#define BIT3                    (0x08u)
#define BIT4                    (0x10u)
#define BIT5                    (0x20u)
#define BIT6                    (0x40u)
#define BIT7                    (0x80u)
#endif


#if DEBUG_PRINT > 0
#include <stdio.h>
#define PRINTF(...)              printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef st
#define st(x)                   do { x } while (__LINE__ == -1)
#endif

#ifndef BV
#define BV(n)                   (1 << (n))
#endif

#ifndef BM
#define BM(n)                   (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)               (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)                (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)                (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)                  (((n) < 0) ? -(n) : (n))
#endif

#define ARRAY_LEN(arr)          (sizeof(arr))/(sizeof(arr[0]))

#define HI_UINT16(a)            (((uint16_t)(a) >> 8) & 0xFF)
#define LO_UINT16(a)            ((uint16_t)(a) & 0xFF)

#define HI_1_UINT32(a)            (((uint32_t)(a) >> 24) & 0xFF)
#define HI_2_UINT32(a)            (((uint32_t)(a) >> 16) & 0xFF)
#define HI_3_UINT32(a)            (((uint32_t)(a) >> 8) & 0xFF)
#define HI_4_UINT32(a)            ((uint32_t)(a) & 0xFF)

#define S2B_UINT32(a)         \
                                  (((uint32_t)(a) & 0xFF000000)  >> 24)\
                                   + (((uint32_t)(a) & 0x00FF0000)  >> 8)\
                                   + (((uint32_t)(a) & 0x0000FF00)  << 8)\
                                   + (((uint32_t)(a) & 0x000000FF)  << 24)


#define S2B_UINT16(a)           ((((uint16_t)(a) & 0xFF00) >> 8) \
                                 + (((uint16_t)(a) & 0x00FF) << 8))

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#ifndef _NOP
#define  _NOP()                 NOP()
#endif

#define WHILE(exp)                                  \
    do                                              \
    {                                               \
        uint32_t i = 0;                             \
        while((exp))                                \
        {                                           \
            if (i++ >700000)                        \
            {                                       \
                DBG_ASSERT(FALSE __DBG_LINE);       \
            }                                       \
        }                                           \
    } while(__LINE__ == -1)

#endif
