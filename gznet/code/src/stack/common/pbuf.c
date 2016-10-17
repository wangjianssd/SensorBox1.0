/**
 * @brief       : create a buffer segment to temporarily store packet data
 *                (sending & recieving)
 * @file        : pbuf.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "pbuf.h"
#include "sys_arch/osel_arch.h"
#include "common/hal/hal.h"

enum _PBUF_TYPE
{
    SMALL_PBUF,
    MEDIUM_PBUF,
    LARGE_PBUF,
    PBUF_TYPE_INVALID
};

#define PBUF_DATA_SIZE(pbuf)        (pbuf->end - pbuf->head)

typedef struct _pbuf_type_t
{
    uint8_t type;
    uint8_t size;
    uint8_t num;
} pbuf_type_t;

static list_head_t pbuf_free_blocks[PBUF_TYPE_MAX_NUM];
uint8_t pbuf_cnt[PBUF_TYPE_MAX_NUM] = {0};

#if PBUF_DBG_EN > 0
static pbuf_t *pbuf_used_p[PBUF_TYPE_MAX_NUM][PBUF_NUM_MAX];
#endif

static void poly_type_pbuf_init(uint8_t type, uint8_t pkt_len, uint8_t num)
{
    void *mem = NULL;
	pbuf_t *pbuf = NULL;

	list_init(&pbuf_free_blocks[type]);

    if (num == 0)
    {
        return ;
    }

	mem = osel_mem_alloc((sizeof(pbuf_t)+pkt_len) * num);
	DBG_ASSERT(mem != NULL __DBG_LINE);

	for (uint8_t i=0; i<num; i++)
	{
        pbuf = (pbuf_t *)((uint8_t *)mem + i*(sizeof(pbuf_t)+pkt_len));
        pbuf->head = (uint8_t *)pbuf + sizeof(pbuf_t);
        pbuf->end = (uint8_t *)pbuf + sizeof(pbuf_t) + pkt_len;
        pbuf->data_p = pbuf->head;
		list_add_to_head(&pbuf->list, &pbuf_free_blocks[type]);
	}

    pbuf_cnt[type] = num;
}

void pbuf_init(void)
{
    poly_type_pbuf_init(SMALL_PBUF,
                        SMALL_PBUF_BUFFER_SIZE,
                        SMALL_PBUF_NUM);

    poly_type_pbuf_init(MEDIUM_PBUF,
                        MEDIUM_PBUF_BUFFER_SIZE,
                        MEDIUM_PBUF_NUM);

    poly_type_pbuf_init(LARGE_PBUF,
                        LARGE_PBUF_BUFFER_SIZE,
                        LARGE_PBUF_NUM);
}

static pbuf_type_t search_free_pbuf(uint8_t pbuf_type)
{
    pbuf_type_t free_pbuf_temp;

    switch (pbuf_type)
    {   // 没有break让代码顺序执行
    case SMALL_PBUF:
        if (!list_empty(&pbuf_free_blocks[SMALL_PBUF]))
        {
            free_pbuf_temp.type = SMALL_PBUF;
            free_pbuf_temp.size = SMALL_PBUF_BUFFER_SIZE;
            free_pbuf_temp.num  = SMALL_PBUF_NUM;
            return free_pbuf_temp;
        }
    case MEDIUM_PBUF:
        if (!list_empty(&pbuf_free_blocks[MEDIUM_PBUF]))
        {
            free_pbuf_temp.type = MEDIUM_PBUF;
            free_pbuf_temp.size = MEDIUM_PBUF_BUFFER_SIZE;
            free_pbuf_temp.num  = MEDIUM_PBUF_NUM;
            return free_pbuf_temp;
        }
    case LARGE_PBUF:
        if (!list_empty(&pbuf_free_blocks[LARGE_PBUF]))
        {
            free_pbuf_temp.type = LARGE_PBUF;
            free_pbuf_temp.size = LARGE_PBUF_BUFFER_SIZE;
            free_pbuf_temp.num  = LARGE_PBUF_NUM;
            return free_pbuf_temp;
        }
    default:
        free_pbuf_temp.type = PBUF_TYPE_INVALID;
        return free_pbuf_temp;
    }
}

static pbuf_type_t pbuf_type_select(uint8_t size)
{
    pbuf_type_t free_pbuf;
    if (size <= SMALL_PBUF_BUFFER_SIZE)
    {
        free_pbuf = search_free_pbuf(SMALL_PBUF);
    }
    else if ((size > SMALL_PBUF_BUFFER_SIZE)
             && (size <= MEDIUM_PBUF_BUFFER_SIZE))
    {
        free_pbuf = search_free_pbuf(MEDIUM_PBUF);
    }
    else if ((size > MEDIUM_PBUF_BUFFER_SIZE)
             && (size <= LARGE_PBUF_BUFFER_SIZE))
    {
        free_pbuf = search_free_pbuf(LARGE_PBUF);
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    if (free_pbuf.type == PBUF_TYPE_INVALID)
    {
        //DBG_ASSERT(FALSE __DBG_LINE);
    }

    return free_pbuf;
}

#if PBUF_DBG_EN > 0
static void add_to_pbuf_used_ptr(pbuf_t *pbuf, pbuf_type_t pbuf_type)
{
    hal_int_state_t s;

    HAL_ENTER_CRITICAL(s);
    for(uint8_t i=0; i<PBUF_NUM_MAX; i++)
    {
        if (pbuf_used_p[pbuf_type.type][i] == NULL)
        {
            pbuf_used_p[pbuf_type.type][i] = pbuf;
            break;
        }
    }
    HAL_EXIT_CRITICAL(s);
}
#endif

pbuf_t *pbuf_alloc(uint8_t size _PLINE1_)
{
    hal_int_state_t s;
    pbuf_type_t avilable_pbuf_type;
	pbuf_t *pbuf = NULL;

    HAL_ENTER_CRITICAL(s);
    avilable_pbuf_type = pbuf_type_select(size);
    if(avilable_pbuf_type.type == PBUF_TYPE_INVALID)
    {
        return NULL;
    }
    pbuf = list_entry_decap(&pbuf_free_blocks[avilable_pbuf_type.type],
                            pbuf_t,
                            list);
    pbuf_cnt[avilable_pbuf_type.type]--;
    HAL_EXIT_CRITICAL(s);

    DBG_ASSERT(pbuf != NULL __DBG_LINE);
	if (pbuf == NULL)
	{
		return NULL;
	}

    osel_memset(pbuf->head, 0, avilable_pbuf_type.size);
    osel_memset(&pbuf->attri, 0, sizeof(pbuf->attri));

    HAL_ENTER_CRITICAL(s);
    pbuf->used = TRUE;
	pbuf->data_len = 0;
    pbuf->data_p = pbuf->head;
	list_init(&pbuf->list);
    HAL_EXIT_CRITICAL(s);

#if PBUF_DBG_EN > 0
    pbuf->alloc_line = line;
    pbuf->free_line = 0;
    add_to_pbuf_used_ptr(pbuf, avilable_pbuf_type);
#endif
	return pbuf;
}

static pbuf_type_t get_pbuf_type(pbuf_t **pbuf)
{
    pbuf_type_t current_pbuf_type;
    uint8_t size_temp;

    DBG_ASSERT(*pbuf != NULL __DBG_LINE);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    size_temp = (*pbuf)->end - (*pbuf)->head;

    if (size_temp == SMALL_PBUF_BUFFER_SIZE)
    {
        current_pbuf_type.type = SMALL_PBUF;
        current_pbuf_type.size = SMALL_PBUF_BUFFER_SIZE;
        current_pbuf_type.num  = SMALL_PBUF_NUM;
    }

    else if (size_temp == MEDIUM_PBUF_BUFFER_SIZE)
    {
        current_pbuf_type.type = MEDIUM_PBUF;
        current_pbuf_type.size = MEDIUM_PBUF_BUFFER_SIZE;
        current_pbuf_type.num  = MEDIUM_PBUF_NUM;
    }

    else if (size_temp == LARGE_PBUF_BUFFER_SIZE)
    {
        current_pbuf_type.type = LARGE_PBUF;
        current_pbuf_type.size = LARGE_PBUF_BUFFER_SIZE;
        current_pbuf_type.num  = LARGE_PBUF_NUM;
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    return current_pbuf_type;
}

#if PBUF_DBG_EN > 0
static void delete_from_pbuf_used_ptr(pbuf_t **pbuf, pbuf_type_t pbuf_type_temp)
{
    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    for(uint8_t i=0; i<PBUF_NUM_MAX; i++)
    {
        if (pbuf_used_p[pbuf_type_temp.type][i] == *pbuf)
        {
            pbuf_used_p[pbuf_type_temp.type][i] = NULL;
            break;
        }
    }

    HAL_EXIT_CRITICAL(s);
}
#endif

void pbuf_free(pbuf_t **const pbuf _PLINE2_)
{
    hal_int_state_t s;
    pbuf_type_t pbuf_type;

	DBG_ASSERT(*pbuf != NULL __DBG_LINE);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    DBG_ASSERT((*pbuf)->used == TRUE __DBG_LINE); //用于检测嵌套的重复释放

    if(pbuf==NULL || *pbuf==NULL || (*pbuf)->used == FALSE)
    {
        return;
    }

    pbuf_type = get_pbuf_type(pbuf);

#if PBUF_DBG_EN > 0
    delete_from_pbuf_used_ptr(pbuf, pbuf_type);
    (*pbuf)->free_line = line;
#endif
    osel_memset((*pbuf)->head, 0, pbuf_type.size);
    osel_memset(&((*pbuf)->attri), 0, sizeof((*pbuf)->attri));

    HAL_ENTER_CRITICAL(s);
    
    if( (*pbuf)->data_len > ((*pbuf)->end - (*pbuf)->head))
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }

    (*pbuf)->used = FALSE;
	(*pbuf)->data_len = 0;
    (*pbuf)->data_p = (*pbuf)->head;
	list_init(&(*pbuf)->list);

    list_add_to_tail(&(*pbuf)->list, &pbuf_free_blocks[pbuf_type.type]);

#if PBUF_DBG_EN > 0
    uint8_t list_cnt = 0;
    list_count(&pbuf_free_blocks[pbuf_type.type], list_cnt);
    DBG_ASSERT(list_cnt != 0 __DBG_LINE);
#endif

    pbuf_cnt[pbuf_type.type]++;

#if PBUF_DBG_EN > 0
    (*pbuf)->alloc_line = 0;
#endif

    HAL_EXIT_CRITICAL(s);

	*pbuf = NULL;
}

uint8_t *pbuf_skip_datap_forward(pbuf_t *const pbuf, uint8_t len)
{
    uint8_t *datap_tmp = NULL;

    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        return NULL;
    }

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    if ((pbuf->data_p+len) > pbuf->end)
    {
        HAL_EXIT_CRITICAL(s);
        return NULL;
    }

    pbuf->data_p += len;
    datap_tmp = pbuf->data_p;

    HAL_EXIT_CRITICAL(s);
    return datap_tmp;
}

uint8_t *pbuf_skip_datap_backward(pbuf_t *const pbuf, uint8_t len)
{
    uint8_t *datap_tmp = NULL;

    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        return NULL;
    }

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    if ((pbuf->data_p-len) < pbuf->head)
    {
        HAL_EXIT_CRITICAL(s);
        return NULL;
    }

    pbuf->data_p -= len;
    datap_tmp = pbuf->data_p;

    HAL_EXIT_CRITICAL(s);
    return datap_tmp;
}

bool_t pbuf_copy_data_in(pbuf_t *const pbuf, const uint8_t *const src, uint8_t len)
{
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        return FALSE;
    }

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    if ((pbuf->data_p + len) > pbuf->end)
    {
        HAL_EXIT_CRITICAL(s);
        return FALSE;
    }
    else
    {
        osel_memcpy(pbuf->data_p, src, len);
        pbuf->data_p += len;
        pbuf->data_len += len;

        HAL_EXIT_CRITICAL(s);
        return TRUE;
    }
}

bool_t pbuf_copy_data_out(uint8_t *const dst, pbuf_t *const pbuf, uint8_t len)
{
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf == NULL)
    {
        return FALSE;
    }

    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);

    if ((pbuf->data_p + len) > pbuf->end)
    {
        HAL_EXIT_CRITICAL(s);
        return FALSE;
    }
    else
    {
        osel_memcpy(dst, pbuf->data_p, len);
        pbuf->data_p += len;

        HAL_EXIT_CRITICAL(s);
        return TRUE;
    }
}
