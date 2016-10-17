/**
 * @brief       : 提供循环队列功能
 *
 * @file        : sqqueue.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef  __SQQUEUE_H
#define  __SQQUEUE_H

#include "common/lib/data_type_def.h"

typedef struct _sqqueue_t
{
    uint8_t *base;          // 队列存储元素的首地址
    uint8_t entry_size;     // 队列元素的宽度
    uint16_t sqq_len;       // 队列总长，可用长度为sqq_len-1
    uint16_t front;         // 队列头下标
    uint16_t rear;          // 队列尾下标
} sqqueue_t;

/**
 * 通用循环队列伪类
 * 该队列有九个操作，分别为单元素入队列、多元素入队列、出队列，
 * 单元素撤销入队列(队尾删除)、取队列长度、判空、清空队列、遍历和删除指定位置
 */
typedef struct _sqqueue_ctrl_t
{
    sqqueue_t   sqq;
    bool_t      (*enter)       (struct _sqqueue_ctrl_t *const p_this, const void *const e);
    bool_t      (*string_enter)(struct _sqqueue_ctrl_t *const p_this, const void *const string, uint16_t len);
    void        *(*del)        (struct _sqqueue_ctrl_t *const p_this);
    void        *(*revoke)     (struct _sqqueue_ctrl_t *const p_this);
    uint16_t    (*get_len)     (const struct _sqqueue_ctrl_t *const p_this);
    bool_t      (*full)        (const struct _sqqueue_ctrl_t *const p_this);
    void        (*clear_sqq)   (struct _sqqueue_ctrl_t *const p_this);
    void        (*traverse)    (struct _sqqueue_ctrl_t *const p_this, void (*vi)(const void *e));
    void        (*remove)      (struct _sqqueue_ctrl_t *const p_this, uint16_t location);
} sqqueue_ctrl_t;

/**
 * 初始化循环队列对象
 *
 * @param p_this:     指向用sqqueue_ctrl_t实例化的对象
 * @param entry_size: 循环队列中元素的宽度
 * @param sqq_len: 队列长度，其中可用长度为sqq_len-1,其中一个成员用来维护队列
 *
 * @return 初始化成功返回true，否者返回false
 */
bool_t sqqueue_ctrl_init(sqqueue_ctrl_t *const p_this,
                         uint8_t entry_size,
                         uint16_t sqq_len);

#endif
/**
 * @}
 */


