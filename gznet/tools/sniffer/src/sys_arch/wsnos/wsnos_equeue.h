/**
 * @brief       : 
 *
 * @file        : wsnos_equeue.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/29
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/29    v0.0.1      gang.cheng    first version
 */
#ifndef __WSNOS_EQUEUE_H__
#define __WSNOS_EQUEUE_H__

#if (OSEL_EQUEUE_CTR_SIZE == 1)
typedef osel_uint8_t equeue_ctr_t;
#elif (OSEL_EQUEUE_CTR_SIZE == 2)
typedef osel_uint16_t equeue_ctr_t;
#elif (OSEL_EQUEUE_CTR_SIZE == 4)
typedef osel_uint32_t equeue_ctr_t;
#else
#error "WSNOS_EQUEUE_CTR_SIZE define incorrectly, expected 1, 2, or 4"
#endif

typedef osel_uint16_t osel_signal_t;

typedef void * osel_param_t;

typedef struct
{
    osel_signal_t sig;
    osel_param_t param;
    osel_uint8_t volatile ref_ctr;
    struct process *p;
} osel_event_t;

typedef struct
{
//    osel_event_t const * volatile front_evt;
    
    osel_event_t const *ring;       //*< 指向缓冲区起始的指针
    
    equeue_ctr_t end;               //*< 缓冲区从起始到结束的偏移量 

    equeue_ctr_t volatile head;     //*< 下一个要插入到缓冲区的事件的偏移量
    equeue_ctr_t volatile tail;     //*< 下一个要从缓冲区获取的事件的偏移量
    equeue_ctr_t volatile n_free;   //*< 当前缓冲区剩余的可用空间数目
    equeue_ctr_t n_min;             //*< 缓冲区里面最小可用空间数目
} osel_equeue_t;

void osel_equeue_init(osel_equeue_t *const this, 
                      osel_event_t const event_sto[],
                      osel_uint16_t const event_len);


osel_bool_t osel_equeue_post(osel_equeue_t *const this,
                            osel_event_t const *const event);

osel_bool_t osel_equeue_get(osel_equeue_t *const this, osel_event_t *event);

#endif

