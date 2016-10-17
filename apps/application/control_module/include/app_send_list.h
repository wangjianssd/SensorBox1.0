 /**
 * @brief       : 心跳帧产生处理
 *
 * @file        : app_send_list.h
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#ifndef _APP_SEND_LIST_H_
#define _APP_SEND_LIST_H_
#include <gznet.h>
//#include <data_type_def.h>
//#include <list.h>
/**
* @breif 比较两个发送对象的优先级
* @return TRUE  obj_a应该排在obj_b的前面, 优先发送
*         FALSE obj_a应该排在obj_b的后面, 推迟发送
*/
typedef bool_t (*send_list_priority_cmp_t)(void *obj_a, void *obj_b);

/**
* @breif 检查对象是否符合条件
* @param[in] obj 被检查的对象
* -          arg 用来检查的参数
* @return TRUE  符合
*         FALSE 不符合
*/
typedef bool_t (*send_list_check_t)(void *obj_a, void *arg);

typedef struct
{
    list_head_t              head;
	uint16_t                 obj_size;
	send_list_priority_cmp_t priority_cmp_func;
}send_list_t;

/**
* @breif 发送链表初始化
*/
void send_list_init(send_list_t             *send_list,
                    uint16_t                 obj_size,
					send_list_priority_cmp_t prio_cmp_func);
/**
* @breif 向发送对列中添加新发送对象, 先后位置按优先级排列;
* @param[in] *obj 待插入的对象地址
* @param[out] *send_list 链表地址
* @return TRUE 插入成功
*/
bool_t send_list_add(send_list_t *send_list,
                     const void  *obj);
/**
* @breif 查找第一个符合条件的发送对象, 返回其地址;
*       - 如果check_func == NULL, 就返回对列中的第一个
* @param[in] *obj 待插入的对象地址
* @param[out] *send_list 链表地址
* @return 符合条件的对象地址
*/
void *send_list_find_first(send_list_t      *send_list,
                           send_list_check_t check_func,
						   void *arg);
/**
* @breif 移除一个发送对象
*/
void send_list_remove(send_list_t *send_list,
                      void        *obj);
/**
* @breif 释放发送对列	
*/
void send_list_free(send_list_t *send_list);
/**
* @breif 查询发送链表的长度
* @return 发送链表的长度
*/
uint16_t send_list_length(send_list_t *send_list);



#endif