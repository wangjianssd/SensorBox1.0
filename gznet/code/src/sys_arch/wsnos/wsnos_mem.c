/**
 * @brief       : 这个文件实现了内存的静态申请管理
 *
 * @file        : wsnos_mem.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "wsnos.h"

static struct _heap
{
    osel_uint16_t padding;
    osel_uint16_t size;
    osel_uint8_t *store;
} mem_heap;

static osel_uint32_t next_free_byte = (osel_uint32_t)0;

void osel_mem_init(osel_uint8_t *buf, osel_uint16_t size)
{
    osel_uint8_t *temp_buf = buf;
    osel_uint16_t temp_size = size;
    
#if OSEL_MEM_ALIGNMENT != 1
    if ( (osel_uint32_t)temp_buf & HEAP_BYTE_ALIGNMENT_MASK )
    {
        temp_buf += (OSEL_MEM_ALIGNMENT - ((osel_uint32_t)temp_buf & HEAP_BYTE_ALIGNMENT_MASK));
    }
    
    osel_uint8_t *end_buf = buf + size;
    if ( (osel_uint32_t)buf & HEAP_BYTE_ALIGNMENT_MASK )
    {
        end_buf -= ((osel_uint32_t)end_buf & HEAP_BYTE_ALIGNMENT_MASK);
    }
    
    temp_size = end_buf - temp_buf;
#endif
    
    osel_memset(temp_buf, 0, temp_size);
    mem_heap.size   = temp_size;
    mem_heap.store  = temp_buf;
    next_free_byte = 0;
}

void *osel_mem_alloc(osel_uint16_t size)
{
    void *mem = NULL;

#if OSEL_MEM_ALIGNMENT != 1
    if ( size & HEAP_BYTE_ALIGNMENT_MASK )
    {
        size += (OSEL_MEM_ALIGNMENT - (size & HEAP_BYTE_ALIGNMENT_MASK));
    }
#endif

    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);
    {
        if ((( next_free_byte + size) <= mem_heap.size)
                && (( next_free_byte + size) > next_free_byte))
        {
            mem = &(mem_heap.store[next_free_byte]);
            next_free_byte += size;
        }
    }
    OSEL_EXIT_CRITICAL(status);

    return mem;
}

void osel_mem_free(void *mem)
{
    (void)mem;
}

void osel_memset(void *const dst, osel_uint8_t val, osel_uint16_t len)
{
    DBG_ASSERT(dst != OSEL_NULL __DBG_LINE);
    osel_uint8_t *p = (osel_uint8_t *)dst;

    while (len--)
    {
        *p++ = val;
    }
}

void osel_memcpy(void *dst, const void *const src, osel_uint16_t len)
{
    DBG_ASSERT(dst != OSEL_NULL __DBG_LINE);
    DBG_ASSERT(src != OSEL_NULL __DBG_LINE);
    osel_uint8_t *tmp = (osel_uint8_t *)dst;
    osel_uint8_t *s = (osel_uint8_t *)src;

    while (len--)
    {
        *tmp++ = *s++;
    }
}

bool_t osel_memcmp(void *const dst, const void *const src, osel_uint16_t len)
{
    DBG_ASSERT(dst != OSEL_NULL __DBG_LINE);
    DBG_ASSERT(src != OSEL_NULL __DBG_LINE);
    osel_uint8_t *tmp = (osel_uint8_t *)dst;
    osel_uint8_t *s = (osel_uint8_t *)src;
    while (len--)
    {
        if (*tmp++ != *s++)
        {
            return FALSE;
        }
    }
    return TRUE;
}


