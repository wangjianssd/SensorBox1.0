/**
 * @brief       : 
 *
 * @file        : sbuf.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <sbuf.h>
#include <hal.h>

/**
 *sbuf链表中空余和已使用sbuf个数的结构体定义
 */
typedef struct _sbuf_cnt_t
{
    uint8_t free;
    uint8_t used;
} sbuf_cnt_t;   

sbuf_cnt_t sbuf_cnt;
static sbuf_t sbuf_free_blocks; /* sbuf管理的链表头 */

/* 已分配sbuf的指针，sbuf链表用于构建数据缓冲，链表头不可用 */
#if SBUF_DBG_EN > 0
static sbuf_t *sbuf_used_p[MAX_SBUF_NUM];
#endif

bool_t sbuf_init(void)
{
	sbuf_t *buf = NULL;
    uint8_t i = 0;
	
	list_init(&sbuf_free_blocks.list);
	buf = (sbuf_t *)osel_mem_alloc(sizeof(sbuf_t) *MAX_SBUF_NUM);
	if(buf == NULL)
	{
		return FALSE;
	}

	for (i=0; i<MAX_SBUF_NUM; i++)
	{
		list_add_to_tail(&(buf + i)->list, &sbuf_free_blocks.list);
	}

    sbuf_cnt.free = MAX_SBUF_NUM;
    sbuf_cnt.used = 0;
	return TRUE;
}

sbuf_t *sbuf_alloc( _SLINE1_ )
{
    hal_int_state_t s;

    HAL_ENTER_CRITICAL(s);
	sbuf_t *sbuf = list_entry_decap(&sbuf_free_blocks.list, sbuf_t, list);
    HAL_EXIT_CRITICAL(s);

	if (sbuf != NULL)
	{

        osel_memset(sbuf, 0, sizeof(sbuf_t));
        sbuf->used = TRUE;

#if SBUF_DBG_EN > 0
        sbuf->alloc_line = line;
        sbuf->free_line = 0;

        HAL_ENTER_CRITICAL(s);
        for(uint8_t i=0; i<MAX_SBUF_NUM; i++)
        {
            if (sbuf_used_p[i] == NULL)
            {
                sbuf_used_p[i] = sbuf;
                break;
            }
        }
        HAL_EXIT_CRITICAL(s);
#endif
        sbuf_cnt.free--;
        sbuf_cnt.used++;
	}

	return sbuf;
}

void sbuf_free(sbuf_t **sbuf _SLINE2_)
{
    hal_int_state_t s;

    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    DBG_ASSERT(*sbuf != NULL __DBG_LINE);
    DBG_ASSERT((*sbuf)->used == TRUE __DBG_LINE);

	if (sbuf==NULL || *sbuf==NULL || (*sbuf)->used==FALSE)
    {
		return;
	}

#if SBUF_DBG_EN > 0
    /* 从已使用队列中删除 */
    HAL_ENTER_CRITICAL(s);
    for(uint8_t i=0; i<MAX_SBUF_NUM; i++)
    {
        if (sbuf_used_p[i] == *sbuf)
        {
            sbuf_used_p[i] = NULL;
            break;
        }
    }
    HAL_EXIT_CRITICAL(s);
    uint16_t line_tmp = (*sbuf)->alloc_line;// 将申请该sbuf的位置保留
#endif
	
    osel_memset((void *)(*sbuf), 0, sizeof(sbuf_t));
    (*sbuf)->used = FALSE;

#if SBUF_DBG_EN > 0
    (*sbuf)->alloc_line = line_tmp;
    (*sbuf)->free_line = line;
#endif	

    HAL_ENTER_CRITICAL(s);
	list_add_to_tail(&((*sbuf)->list), &sbuf_free_blocks.list);
    HAL_EXIT_CRITICAL(s);

    sbuf_cnt.free++;
    sbuf_cnt.used--;
    *sbuf = NULL;
}

