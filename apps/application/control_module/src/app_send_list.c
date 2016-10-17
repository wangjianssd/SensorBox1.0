
 /**
 * @brief       : 链表的插入、删除、查询等操作处理
 *
 * @file        : app_send_list.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h> 
//#include <string.h>
//#include <pbuf.h>
#include <app_send_list.h>

/**
* @breif 向发送对列中添加新发送对象, 先后位置按优先级排列;
* @param[in] *obj 待插入的对象地址
* @param[out] *send_list 链表地址
* @return TRUE 插入成功
*/
bool_t send_list_add(send_list_t *send_list,
                     const void  *obj)
{
    DBG_ASSERT(send_list != NULL __DBG_LINE);
    DBG_ASSERT(obj != NULL __DBG_LINE);
	
    pbuf_t *pbuf = pbuf_alloc(send_list->obj_size __PLINE1);
	DBG_ASSERT(pbuf != NULL __DBG_LINE);
	DBG_ASSERT(((uint32_t)pbuf->data_p % sizeof(int)) == 0 __DBG_LINE);
    
	memcpy(pbuf->data_p, obj, send_list->obj_size);
	
	pbuf_t *next_pos;
	list_entry_for_each(next_pos, &send_list->head, pbuf_t, list)
	{
	    if (TRUE == send_list->priority_cmp_func(pbuf->data_p, next_pos->data_p))
		{
		    break;
		}
	}
	
	list_insert_forwards(&pbuf->list, &next_pos->list);
	return TRUE;
}

/**
* @breif 查找第一个符合条件的发送对象, 返回其地址;
*       - 如果check_func == NULL, 就返回对列中的第一个
* @param[in] *obj 待插入的对象地址
* @param[out] *send_list 链表地址
* @return 符合条件的对象地址
*/
void *send_list_find_first(send_list_t      *send_list,
                           send_list_check_t check_func,
						   void             *arg)
{
    DBG_ASSERT(send_list != NULL __DBG_LINE);
	
	pbuf_t *pbuf;
	
	if (check_func == NULL)
	{
        
	    pbuf = list_entry_get_head(&send_list->head, pbuf_t, list);
		return (pbuf == NULL) ? NULL : pbuf->data_p;
	}
	
	pbuf_t *next_pos;
	list_entry_for_each(next_pos, &send_list->head, pbuf_t, list)
	{
	    if (TRUE == check_func(next_pos->data_p, arg))
		{
		    break;
		}
	}
	
	return (&next_pos->list == &send_list->head) ? NULL : next_pos->data_p;
}

/**
* @breif 移除一个发送对象
*/
void send_list_remove(send_list_t *send_list,
                      void        *obj)
{
    DBG_ASSERT(send_list != NULL __DBG_LINE);
	
	pbuf_t *next_pos;
	list_entry_for_each(next_pos, &send_list->head, pbuf_t, list)
	{
	    if (next_pos->data_p == obj)
		{
		    break;
		}
	}
	
	if (&next_pos->list != &send_list->head)
	{
	    list_del(&next_pos->list);
	    pbuf_free(&next_pos __PLINE1);
		DBG_ASSERT(next_pos == NULL __DBG_LINE);
	}
}

/**
* @breif 释放发送对列	
*/
void send_list_free(send_list_t *send_list)
{
    pbuf_t *next_pos;
    pbuf_t *temp;
	list_entry_for_each_safe(next_pos, temp, &send_list->head, pbuf_t, list)
	{
	    list_del(&next_pos->list);
	    pbuf_free(&next_pos __PLINE1);
		DBG_ASSERT(next_pos == NULL __DBG_LINE);
	}
}

/**
* @breif 查询发送链表的长度
* @return 发送链表的长度
*/
uint16_t send_list_length(send_list_t *send_list)
{
    uint16_t length;
    
    list_count(&send_list->head, length);
    
    return length;
}

/**
* @breif 发送链表初始化
*/
void send_list_init(send_list_t             *send_list,
                    uint16_t                 obj_size,
					send_list_priority_cmp_t prio_cmp_func)
{
    list_init(&send_list->head);
	send_list->obj_size = obj_size;
	send_list->priority_cmp_func = prio_cmp_func;
}