/**
 * @brief       : 主要是加速度传感器算法队列处理
 *
 * @file        : acc_sensor_queue.c
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */
#include <stdio.h>
#include <string.h>
#include <acc_sensor_queue.h>

/**
  *初始化，构造空队
  *
  *@param: 队列名称
  *@return:
  *
  */
void acc_queue_create(acc_circular_queue_t* queue)
{
    queue->front = queue->rail = 0;
    queue->count = 0;
    queue->personal_drop_front = 0;
    queue->personal_topple_bak_front = 0;
    queue->personal_topple_filter_front = 0;
    queue->personal_turn_front = 0;
}

/**
  *判断队列是否为空
  *
  *@param: 队列名称
  *@return: TRUE:空
  *         FALSE:非空
  */
bool_t is_acc_queue_empty(acc_circular_queue_t* queue)
{
    return 0 == queue->count ? TRUE : FALSE;
}

/**
  *判断队列是否已满
  *
  *@param: 队列名称
  *@return: TRUE:满
  *         FALSE:没满
  */
bool_t is_acc_queue_full(acc_circular_queue_t* queue,uint16_t acc_queue_max_len)
{
    return acc_queue_max_len  == queue->count ? TRUE : FALSE;
}

/**
  *元素入队
  *
  *@param: queue：队列名称
  *         item：准备插入的元素
  *@return: TRUE:插入成功
  *         FALSE:队列已满
  */
bool_t acc_queue_send(acc_circular_queue_t* queue, 
                      acc_data_type item)
{
    static uint8_t buf_full_flag = 0;
    // 入队前，判断队满
    if(is_acc_queue_full(queue, ACC_QUEUE_MAXLEN))
        return FALSE;   

    queue->item[queue->rail] = item;
    queue->rail = (queue->rail + 1) % ACC_QUEUE_MAXLEN ;       
    queue->count++;    
    if (buf_full_flag == 1)//队列长度达到最大
    {
        if (queue->personal_turn_front >= ACC_QUEUE_MAXLEN)
        {
            queue->personal_turn_front = 0;
        }
        else
        {
            queue->personal_turn_front ++;
        }

        if (queue->personal_drop_front >= ACC_QUEUE_MAXLEN)
        {
            queue->personal_drop_front = 0;
        }
        else
        {
            queue->personal_drop_front ++;
        }        

        if (queue->personal_topple_bak_front >= ACC_QUEUE_MAXLEN)
        {
            queue->personal_topple_bak_front = 0;
        }
        else
        {
            queue->personal_topple_bak_front ++;
        }

        if (queue->personal_topple_filter_front >= ACC_QUEUE_MAXLEN)
        {
            queue->personal_topple_filter_front = 0;
        }
        else
        {
            queue->personal_topple_filter_front ++;
        }        
//        queue->personal_turn_front = (queue->personal_turn_front + 1) % ACC_QUEUE_MAXLEN;
//        queue->personal_drop_front = (queue->personal_drop_front + 1) % ACC_QUEUE_MAXLEN;
//        queue->personal_topple_bak_front = (queue->personal_topple_bak_front + 1) % ACC_QUEUE_MAXLEN;
//        queue->personal_topple_filter_front = (queue->personal_topple_filter_front + 1) % ACC_QUEUE_MAXLEN;
    }
    else//队列长度未达到最大
    {   
        if (queue->count > TURN_COUNT_THRESH)
        {
            queue->personal_turn_front ++;
        }

        if (queue->count > DROP_COUNT_THRESH)
        {
            queue->personal_drop_front ++;
        }
        
        if (queue->count > TOPPLE_BAK_COUNT_THRESH)
        {
            queue->personal_topple_bak_front ++;
        }

        if (queue->count > TOPPLE_FILTER_COUNT_THRESH)
        {
            queue->personal_topple_filter_front ++;
        }
        
        //if (queue->count >= (ACC_QUEUE_MAXLEN - 1))
        if (queue->count >= ACC_QUEUE_MAXLEN)
        {
            buf_full_flag = 1;
        }
    }
    //queue->count++;   
    return TRUE;
}

/**
  *元素出队
  *
  *@param: queue：队列名称
  *         item：出队的首元素存放的地址
  *@return: TRUE:出队成功
  *         FALSE:队列为空
  */
bool_t acc_queue_receive(acc_circular_queue_t* queue, 
                         acc_data_type* item,
                         uint16_t acc_queue_max_len)
{
    //出队前，判断队空
    if(is_acc_queue_empty(queue))
        return FALSE;

    *item = queue->item[queue->front];
    queue->front = (queue->front + 1) % acc_queue_max_len;
    queue->count--;

    return TRUE;
}

/**
  *查看队首元素
  *
  *@param: queue：队列名称
  *         item：首元素存放的地址
  *@return: TRUE:查看成功
  *         FALSE:队列为空
  */
bool_t acc_queue_peek(acc_circular_queue_t* queue, acc_data_type* item)
{
    //判断队空
    if(is_acc_queue_empty(queue))
        return FALSE;

    *item = queue->item[queue->front];
    return TRUE;
}

/**
  *查看队列的长度
  *
  *@param: queue：队列名称
  *@return: 队列的长度
  *
  */
uint8_t acc_queue_count(acc_circular_queue_t* queue)
{
    return queue->count;
}
