/**
 * @brief       : \
 *
 * @file        : wsnos_mem.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/29
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/29    v0.0.1      gang.cheng    first version
 */
#ifndef __WSNOS_MEM_H__
#define __WSNOS_MEM_H__

#if OSEL_MEM_ALIGNMENT == 4
#define HEAP_BYTE_ALIGNMENT_MASK    ((osel_uint32_t)0x0003)
#endif

#if OSEL_MEM_ALIGNMENT == 2
#define HEAP_BYTE_ALIGNMENT_MASK    ((osel_uint32_t)0x0001)
#endif

#if OSEL_MEM_ALIGNMENT == 1
#define HEAP_BYTE_ALIGNMENT_MASK    ((osel_uint32_t)0x0000)
#endif

#ifndef HEAP_BYTE_ALIGNMENT_MASK
#error "Invalid OSEL_MEM_ALIGNMENT definition"
#endif

void osel_mem_init(osel_uint8_t *buf, osel_uint16_t size);

void *osel_mem_alloc(osel_uint16_t size);

void osel_mem_free(void *mem);

void osel_memset(void *const dst, osel_uint8_t val, osel_uint16_t len);

void osel_memcpy(void *dst, const void *const src, osel_uint16_t len);

osel_bool_t osel_memcmp(void *const dst, const void *const src, osel_uint16_t len);

#endif
