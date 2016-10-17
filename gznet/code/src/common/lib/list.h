/**
 * @brief       : 双向链表操作接口
 *                该双向链表的操作，请参照Linux内核 (include/linux/list.h)
 *
 * @file        : list.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 * @{
 */


#ifndef __LIST_H
#define __LIST_H

#include "common/lib/lib.h"

typedef struct list_head
{
	struct list_head *next;
	struct list_head *prev;
} list_head_t;


/**
 * 获得链表元素所在实体的地址, 该实体在链表中保存
 *
 * @param ptr:    链表的入口指针
 * @param type:   结构类型
 * @param member: 元素结构中链表变量的名字
 *
 * @return     指向该元素所在实体的指针
 */
//#define list_entry_addr_find(ptr, type, member)                         \
//        ((type *)((uint8_t *)(ptr)-(uintptr_t)(&((type *)0)->member)))

#define list_entry_addr_find(ptr, type, member) \
        ((type *)((char *)(ptr)-(unsigned long)(&((type *)NULL)->member)))

/**
 * 移除并返回链表中首元素所在的实体，该实体在链表中不保存
 *
 * @param head:	  链表的入口指针
 * @param type:	  结构类型
 * @param member: 指向该链表的第一个元素的指针
 *
 * @return 表首元素所在实体的指针，链表为空则返回空指针
 */
#define list_entry_decap(head, type, member)                            \
	(                                                                   \
        (list_empty(head)) ? (type *)NULL                               \
        : (list_entry_addr_find(list_next_elem_get(head), type, member))\
    )


#define list_entry_get_head(head, type, member)                         \
	((list_empty(head))?(type *)NULL:(list_entry_addr_find((head)->next,\
    type, member)))
/**
 * 移除并返回链表尾部所在实体，该实体在链表中不保存
 *
 * @param head:	  链表入口指针
 * @param type:	  链表所在结构体的名称
 * @param member: 结构体中，链表变量的名称
 *
 * @return 表尾部所在实体指针，链表为空则返回空指针
 */
#define list_entry_curtail(head, type, member)                       \
	(   (list_empty(head)) ? (type *)NULL                            \
        : (list_entry_addr_find(list_curtail(head), type, member))   \
    )


/**
 * 正向遍历链表，在该遍历中不能对链表做删除操作	--	list_for_each
 *
 * @param pos:   链表元素计数器
 * @param head:  链表的入口指针
 */
#define list_for_each_forwards(pos, head)                            \
	for ((pos)=(head)->next; (pos)!=(head); (pos)=(pos)->next)

/**
 * 反向遍历链表，在该遍历中不能对链表做删除操作
 *
 * @param pos:   链表元素计数器
 * @param head:  链表的入口指针
 */
#define list_for_each_backwards(pos, head)                           \
	for ((pos)=(head)->prev; (pos)!=(head); (pos)=(pos)->prev)

/**
 * 链表遍历，支持删除操作
 *
 * @param pos:	链表元素计数器
 * @param n:	临时链表元素
 * @param head:	链表入口指针
 */
#define list_for_each_safe(pos, n, head)                            \
	for ((pos)=(head)->next, n = (pos)->next; (pos)!=(head);        \
		 (pos)=n, n=(pos)->next)


/**
 * 遍历链表所在实体，不可删除实体
 *
 * @param pos:	  链表元素计数器
 * @param head:	  链表入口指针
 * @param type:	  链表所在结构体的名称
 * @param member: 结构体中，链表变量的名称
 */
#define list_entry_for_each(pos, head, type, member)				\
	for ((pos) = list_entry_addr_find((head)->next, type, member);	        \
	     &(pos)->member != (head); 					                \
	     (pos) = list_entry_addr_find((pos)->member.next, type, member))

/**
 * 遍历链表所在实体, 支持删除操作
 *
 * @param pos:	  链表元素计数器
 * @param n:	  临时链表元素
 * @param head:	  链表入口指针
 * @param type:	  链表所在结构体的名称
 * @param member: 结构体中，链表变量的名称
 */
#define list_entry_for_each_safe(pos, n, head, type, member)		\
	for ((pos) = list_entry_addr_find((head)->next, type, member),	        \
		 n = list_entry_addr_find((pos)->member.next, type, member);	        \
		 &(pos)->member != (head); 					                \
		 (pos) = n, n = list_entry_addr_find(n->member.next, type, member))


/**
 * 计算链表中元素的个数
 */
#define list_count(head, count)                                      \
do                                                                   \
{                                                                    \
    count = 0;                                                       \
    for (list_head_t *pos=(head)->next; pos!=(head); pos=pos->next)  \
    {                                                                \
        count++;                                                     \
    }                                                                \
} while(0)



/**
 * 将实体按顺序插入到链表中合适的地方，顺序由函数funcCompare来决定	--	list_sorted_add
 *
 * @param new_entry:   所要插入的实体
 * @param head:        链表头
 * @param type:        链表所在结构体的名称
 * @param member:      结构体中，链表变量的名称
 * @param func_compare: 顺序比较函数,声明：bool func_compare(Node* A, Node* B)
 *                     如果 A < B, 返回 true, 否则返回 false
 * @param pos:         链表元素计数器
 */
#define list_entry_sorted_add(new_entry, head, type, member, func_compare, pos)\
do                                                                             \
{                                                                              \
    type *entry_a = NULL;                                                      \
    type *entry_b = NULL;                                                      \
	for ((pos)=(head)->next; (pos)!=(head); (pos)=(pos)->next)                 \
    {                                                                          \
        entry_a = list_entry_addr_find((new_entry), type, member);             \
        entry_b = list_entry_addr_find((pos), type, member);                   \
		if (func_compare(entry_a, entry_b))                                    \
        {                                                                      \
			break;                                                             \
		}                                                                      \
	}                                                                          \
	if ((pos) != (head))                                                       \
    {                                                                          \
		list_insert_forwards((new_entry), (pos));                              \
	}                                                                          \
    else                                                                       \
    {                                                                          \
		list_add_to_tail((new_entry), (head));                                 \
	}                                                                          \
} while(__LINE__ == -1)                                                        \


/**
 * 链表头初始化
 *
 * @param ptr: 需要被初始化的链表头指针
 */
void list_init(list_head_t *const ptr);


/**
 * 在指定位置之前插入新的元素	
 *
 * @param new_entry: 需要放入链表中的新元素
 * @param pos:       链表中放入新元素的位置指针
 */
void list_insert_forwards(list_head_t *const new_entry, list_head_t *const pos);


/**
 * 在指定位置之后插入新的元素
 *
 * @param new_entry: 需要放入链表中的新元素
 * @param pos:       链表中放入新元素的位置指针
 */
void list_insert_backwards(list_head_t *const new_entry, list_head_t *const pos);


/**
 * 在链表尾部之后插入新的元素	-- list_append
 *
 * @param new_entry: 需要放入链表尾部的新元素
 * @param list:      链表头指针
 */
void list_add_to_tail(list_head_t *const new_entry, list_head_t *const list);


/**
 * 在链表头部之后插入新的元素
 *
 * @param new_entry: 需要放入链表尾部的新元素
 * @param list:      链表头指针
 */
void list_add_to_head(list_head_t *const new_entry, list_head_t *const list);


/**
 * 将指定元素从链表中删除
 *
 * @param elem:   需要删除的链表元素
 */
void list_del(list_head_t *const elem);


/**
 * 将链表的尾元素删除并返回
 *
 * @param head:  链表指针
 *
 * @return      链表尾元素
 */
list_head_t *list_curtail(const list_head_t *const head);


/**
  * 判断链表是否为空
 *
 * @param head: 链表头指针
 *
 * @return    为空时返回TRUE，否则为FALSE
 */
bool_t list_empty(const list_head_t *const head);


/**
 * 获取链表中第一个元素的地址	--	list_get_head
 *
 * @param head:   链表头指针
 *
 * @return 首元素的地址
 */
list_head_t *list_first_elem_look(const list_head_t *const head);


/**
 * 取出给定位置的下一个元素
 *
 * @param pos:  链表元素地址
 *
 * @return      下一个链表元素地址
 */
list_head_t *list_next_elem_get(const list_head_t *const pos);


/**
 * 将一个元素从一个链表中移除，然后再插入另外一个链表中的头部
 *
 * @param elem:  被移除的链表元素
 * @param head:   新链表头
 */
void list_move_to_another_head(list_head_t *const elem, list_head_t *const head);


/**
 * 将一个元素从一个链表中移除，然后再放入另外一个链表中的尾部
 *
 * @param elem: 被移除的链表元素
 * @param head:  新链表头
 */
void list_move_to_another_tail(list_head_t *const elem, list_head_t *const head);

	
#endif
/**
 * @}
 */
