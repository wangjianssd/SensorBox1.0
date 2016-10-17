///** \file compiler_defs.h
// *
// * \brief Register/bit definitions for cross-compiler projects on the C8051F93x/2x family.
// *
// * \b COPYRIGHT
// * \n Portions of this file are copyright Maarten Brock
// * \n http://sdcc.sourceforge.net
// * \n Portions of this file are copyright 2010, Silicon Laboratories, Inc.
// * \n http://www.silabs.com
// *
// *


#ifndef COMPILER_DEFS_H
#define COMPILER_DEFS_H

#include "sys_arch/osel_arch.h"

# define SEG_FAR   
# define SEG_DATA  
# define SEG_NEAR  
# define SEG_IDATA 
# define SEG_XDATA 
# define SEG_PDATA 
# define SEG_CODE  
# define SEG_BDATA

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;

typedef uint8_t BIT;




#ifndef   FALSE
#define   FALSE     0
#endif
#ifndef   TRUE
#define   TRUE      !FALSE
#endif


#ifndef   LOW
#define   LOW     0
#endif
#ifndef   HIGH
#define   HIGH      !LOW
#endif

#ifndef   ZERO
#define   ZERO     0
#endif
#ifndef   ONE
#define   ONE      !ZERO
#endif

#ifndef   NULL
#define   NULL      ((void *) 0)
#endif

typedef union UU16
{
    U16 U16;
    S16 S16;
    U8 U8[2];
    S8 S8[2];
} UU16;

typedef union UU32
{
    U32 U32;
    S32 S32;
    UU16 UU16[2];
    U16 U16[2];
    S16 S16[2];
    U8 U8[4];
    S8 S8[4];
} UU32;

#define SEGMENT_VARIABLE(name, vartype, locsegment) vartype locsegment name
#define VARIABLE_SEGMENT_POINTER(name, vartype, targsegment) vartype targsegment * name
#define SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targsegment, locsegment) vartype targsegment * locsegment name

#define LSB 1
#define MSB 0

#define RADIO_DRIVER_EXTENDED_SUPPORT

#define RADIO_DRIVER_FULL_SUPPORT

#undef  SPI_DRIVER_EXTENDED_SUPPORT


#endif











