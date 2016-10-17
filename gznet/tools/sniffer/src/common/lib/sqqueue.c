/**
 * @brief       : 
 *
 * @file        : sqqueue.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <platform.h>
#include <wsnos.h>

#define SQQ_ENTRY_SIZE  (queue_ptr->entry_size)
#define SQQ_LEN         (queue_ptr->sqq_len)

static bool_t sqqueue_init(sqqueue_ctrl_t *const p_this,
                           uint8_t entry_size,
                           uint16_t sqq_len)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if (p_this != NULL)
    {
        queue_ptr->entry_size = entry_size;
        queue_ptr->sqq_len = sqq_len;
        queue_ptr->base = (uint8_t *)osel_mem_alloc(SQQ_LEN*SQQ_ENTRY_SIZE);
        if (queue_ptr->base == NULL)
        {
            return FALSE;
        }
        queue_ptr->front = 0;
        queue_ptr->rear = 0;

        return TRUE;
    }

    return FALSE;
}

static uint16_t sqqueue_length(const sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    const sqqueue_t *const queue_ptr = &(p_this->sqq);
    uint16_t length = 0;
    hal_int_state_t s;

    if (p_this != NULL)
    {
        HAL_ENTER_CRITICAL(s);
        length = (queue_ptr->rear + SQQ_LEN - queue_ptr->front);
        HAL_EXIT_CRITICAL(s);

        if (length >= SQQ_LEN)
        {
            length -= SQQ_LEN;
        }
    }

    return length;
}

static bool_t sqqueue_full(const sqqueue_ctrl_t *const p_this)
{
    uint16_t rear = 0;

    DBG_ASSERT(p_this != NULL __DBG_LINE);

    if (p_this != NULL)
    {
        const sqqueue_t *const queue_ptr = &(p_this->sqq);
        rear = queue_ptr->rear + 1;
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
        }
        if ( rear == queue_ptr->front)
        {
            return TRUE;
        }
    }

    return FALSE;
}


static bool_t enter_sqqueue(sqqueue_ctrl_t *const p_this, const void *const e)
{
    uint16_t rear = 0;
    hal_int_state_t s;
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if ((p_this!=NULL) && (e!=NULL))
    {
        HAL_ENTER_CRITICAL(s);
        rear = queue_ptr->rear + 1;        
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
        }

        if (rear == queue_ptr->front)
        {
            HAL_EXIT_CRITICAL(s);
            return FALSE;
        }
        
        /* 根据e的长度进行内存拷贝 */
        DBG_ASSERT(queue_ptr->rear != SQQ_LEN __DBG_LINE);
        osel_memcpy(queue_ptr->base+(queue_ptr->rear*SQQ_ENTRY_SIZE),
                    e,
                    SQQ_ENTRY_SIZE);
        queue_ptr->rear = rear;
        
        HAL_EXIT_CRITICAL(s);
        return TRUE;
    }
    return FALSE;
}

static bool_t string_enter_sqqueue(sqqueue_ctrl_t *const p_this,
                                   const void *const string,
                                   uint16_t cnt)
{
    uint16_t  rear = 0;
    uint16_t  length = 0;
    hal_int_state_t s;
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if ((p_this!=NULL) && (string!=NULL))
    {
        /* 判断是否超出队列长度 */
        HAL_ENTER_CRITICAL(s);
        length =  sqqueue_length(p_this); // 已有元素个数
        if (length == 0xFFFF)
        {
            HAL_EXIT_CRITICAL(s);
            return FALSE;
        }

        length = (SQQ_LEN-1)-length;        // 可写入个数
        if (length < cnt)
        {
            HAL_EXIT_CRITICAL(s);
            return FALSE;
        }

        rear = queue_ptr->rear + cnt;
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
            uint8_t half = SQQ_LEN - queue_ptr->rear;
            osel_memcpy(queue_ptr->base+(queue_ptr->rear*SQQ_ENTRY_SIZE), 
                        string, half*SQQ_ENTRY_SIZE);
            uint8_t *half_p = (uint8_t *)string;
            osel_memcpy(queue_ptr->base, (uint8_t *)&half_p[half], rear*SQQ_ENTRY_SIZE);
        }
        else
        {
            osel_memcpy(queue_ptr->base+(queue_ptr->rear*SQQ_ENTRY_SIZE),
                        string, SQQ_ENTRY_SIZE*cnt);
        }

        queue_ptr->rear = rear;
        
        HAL_EXIT_CRITICAL(s);
        return TRUE;
    }
    return FALSE;
}

static void *delete_sqqueue(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    uint16_t front = 0;
    hal_int_state_t s;
    sqqueue_t *queue_ptr = NULL;

    if (p_this != NULL)
    {
        void *p_elem = NULL;

        HAL_ENTER_CRITICAL(s);
        queue_ptr = &(p_this->sqq);
        if (queue_ptr->rear == queue_ptr->front)
        {
            HAL_EXIT_CRITICAL(s);
            return NULL;
        }
        /* 根据元素类型大小计算出偏移量，得到该元素首地址 */
        p_elem = (void *)((queue_ptr->base)+(queue_ptr->front*SQQ_ENTRY_SIZE));
        front = queue_ptr->front+1;
        if (front >= SQQ_LEN)
        {
            front -=  SQQ_LEN;
        }
        queue_ptr->front = front;
       
        HAL_EXIT_CRITICAL(s);
        return p_elem;
    }
    return NULL;
}

static void *revoke_sqqueue(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    uint16_t rear = 0;
    hal_int_state_t s;
    sqqueue_t *queue_ptr = NULL;

    if (p_this != NULL)
    {
        void *p_elem = NULL;

        HAL_ENTER_CRITICAL(s);
        queue_ptr = &(p_this->sqq);
        if (queue_ptr->rear == queue_ptr->front)
        {
            HAL_EXIT_CRITICAL(s);
            return NULL;
        }

        rear = queue_ptr->rear;
        if (rear == 0)
        {
            rear = SQQ_LEN-1;
        }
        else
        {
            rear--;
        }
        queue_ptr->rear = rear;
        /* 根据元素类型大小计算出偏移量，得到该元素首地址*/
        p_elem = (void *)((queue_ptr->base)+(queue_ptr->rear*SQQ_ENTRY_SIZE));
        
        HAL_EXIT_CRITICAL(s);
        return p_elem;
    }
    return NULL;
}

static void clear_sqq(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    hal_int_state_t s;
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if (p_this != NULL)
    {
        HAL_ENTER_CRITICAL(s);
        queue_ptr->front = 0;
        queue_ptr->rear = 0;
        HAL_EXIT_CRITICAL(s);
    }
}


static void traverse(sqqueue_ctrl_t *const p_this, void (*vi)(const void *e))
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = NULL;
    uint16_t i = 0;
    
    if (p_this != NULL)
    {
        queue_ptr = &(p_this->sqq);
        
        if (queue_ptr->rear == queue_ptr->front)
        {
            return;
        }
        
        i = queue_ptr->front;        
        while(i != queue_ptr->rear)
        {
            vi( (void *)((queue_ptr->base)+(i*SQQ_ENTRY_SIZE)) );
            if (++i >= SQQ_LEN)
            {
                i = 0;
            }
        }
    }
}

/* 删除相对队头指定偏移位置的元素，输入参数为相对于队首的偏移位置 */
static void qremove(sqqueue_ctrl_t *const p_this, uint16_t offset_to_front)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = NULL;
    uint16_t i = 0;
    
    if (p_this != NULL)
    {
        queue_ptr = &(p_this->sqq);
        DBG_ASSERT(offset_to_front < SQQ_LEN __DBG_LINE);
        
        if (queue_ptr->rear == queue_ptr->front)
        {
            return;
        }

        uint16_t j = 0;
        
        for (i=offset_to_front; i>0; i--)
        {
            /* 定位待删除元素在队列中的位置 */
            j = queue_ptr->front + i;
            
            if (j >= SQQ_LEN)
            {
                j -=  SQQ_LEN;
            }
              
            if (j == 0)  // 在翻转位置特殊处理拷贝的源地址
            {
                osel_memcpy(queue_ptr->base+(0*SQQ_ENTRY_SIZE), 
                            queue_ptr->base+((SQQ_LEN-1)*SQQ_ENTRY_SIZE),
                            SQQ_ENTRY_SIZE);
            }
            else
            {
                osel_memcpy(queue_ptr->base+(j*SQQ_ENTRY_SIZE), 
                            queue_ptr->base+((j-1)*SQQ_ENTRY_SIZE),
                            SQQ_ENTRY_SIZE);
            }
        }
        
        /* 减少队列长度 */
        uint16_t front = queue_ptr->front+1;
        if (front >= SQQ_LEN)
        {
            front -=  SQQ_LEN;
        }
        
        queue_ptr->front = front;
    }
}

bool_t sqqueue_ctrl_init(sqqueue_ctrl_t *const p_this,
                         uint8_t entry_size,
                         uint16_t sqq_len)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);

    if (p_this != NULL)
    {
       if (sqqueue_init(p_this, entry_size, sqq_len) != FALSE)
       {
           p_this->enter          = enter_sqqueue;
           p_this->string_enter   = string_enter_sqqueue;
           p_this->del            = delete_sqqueue;
           p_this->revoke         = revoke_sqqueue;
           p_this->get_len        = sqqueue_length;
           p_this->full           = sqqueue_full;
           p_this->clear_sqq      = clear_sqq;
           p_this->traverse       = traverse;
           p_this->remove         = qremove;
           return TRUE;
       }
    }
    return FALSE;
}
