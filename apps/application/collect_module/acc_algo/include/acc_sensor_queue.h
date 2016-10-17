#ifndef __ACC_QUEUE_H
#define __ACC_QUEUE_H

// 宏定义循环队列的空间大小
#define ACC_QUEUE_MAXLEN        127
#include <gznet.h>
//#include <osel_arch.h>

typedef enum
{
    ACC_TURN_FLAG,
    ACC_DROP_FLAG,
    ACC_TOPPLE_BAK_FLAG,
    ACC_TOPPLE_FILTER_FLAG,
}acc_queue_flag;

//元素类型定义
typedef struct
{
    int16_t  src_x;
    int16_t  src_y;
    int16_t  src_z;
}acc_data_type;

//新算法数据入口定义
typedef struct
{
    int16_t  src_x;
    int16_t  src_y;
    int16_t  src_z;
    int16_t  src_temp;
}acc_new_alg_type_t;

//循环队列结构的定义
typedef struct
{
    uint8_t personal_turn_front;
    uint8_t personal_drop_front;
    uint8_t personal_topple_bak_front;
    uint8_t personal_topple_filter_front;
    uint8_t front;                              // 队列头指针
    uint8_t rail;                               // 队列尾指针
    uint8_t count;                              // 计数器，统计队列中元素个数
    acc_data_type item[ACC_QUEUE_MAXLEN];       // 存储队列中的元素
} acc_circular_queue_t;

//定义不同节点入队列的计数门限值
#define TURN_COUNT_THRESH                   50
#define DROP_COUNT_THRESH                   10
#define TOPPLE_BAK_COUNT_THRESH             127
#define TOPPLE_FILTER_COUNT_THRESH          10

// 创建队列
void acc_queue_create(acc_circular_queue_t* queue);
// 判断队列是否为空
bool_t is_acc_queue_empty(acc_circular_queue_t* queue);
// 判断队是否为满
bool_t is_acc_queue_full(acc_circular_queue_t* queue, uint16_t acc_queue_max_len);
// 元素插入队列
bool_t acc_queue_send(acc_circular_queue_t* queue, 
                      acc_data_type item);
// 元素出队
bool_t acc_queue_receive(acc_circular_queue_t* queue, 
                         acc_data_type* item,
                         uint16_t acc_queue_max_len);
// 取队首元素，仅查看
bool_t acc_queue_peek(acc_circular_queue_t* queue, acc_data_type* item);
//返回队列长度
uint8_t acc_queue_count(acc_circular_queue_t* queue);

#endif