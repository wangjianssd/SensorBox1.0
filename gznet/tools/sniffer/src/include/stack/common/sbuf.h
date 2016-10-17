/**
 * @brief       : 
 *
 * @file        : sbuf.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 * 2015-09-07  v0.0.2      gang.cheng   add osel_etimer_t element
 */

#ifndef __SBUF_H
#define __SBUF_H

#include <prim.h>
#include <wsnos.h>

#define SBUF_DBG_EN     1u

#if SBUF_DBG_EN > 0
/* 形参 */
#define _SLINE1_  uint16_t line
#define _SLINE2_  ,uint16_t line
/* 实参 */
#define __SLINE1  __LINE__
#define __SLINE2  ,__LINE__

#else

#define _SLINE1_  void
#define _SLINE2_
#define __SLINE1
#define __SLINE2

#endif

typedef enum _orig_layer_t
{
    PHY_LAYER = 0,
    MAC_LAYER,
    NWK_LAYER,
    APP_LAYER,
    APPS_LAYER,
} orig_layer_t;

typedef enum _up_down_link_t_
{
    UP_LINK = 0,
    DOWN_LINK
} up_down_link_t;

/**
 * sbuf的数据类型定义
 */
typedef struct _sbuf_t
{
    list_head_t     list;
    orig_layer_t    orig_layer;        //转发不能改变原始归属层
    up_down_link_t  up_down_link;
    prim_type_t     primtype;
    prim_args_t     primargs;
    bool_t          used;
    osel_etimer_t   etimer;
    bool_t          was_armed;
	uint16_t        slot_seq;          // 发送时隙时使用的时隙序列号

#if SBUF_DBG_EN > 0
    uint16_t        alloc_line;
    uint16_t        free_line;
#endif
} sbuf_t;


/**
 * 对sbuf进行初始化
 */
bool_t sbuf_init(void);

/**
 * 申请一个空sbuf,从sbuf链表中获取一个空的sbuf
 *
 * @param return 指向空的sbuf的指针(成功)或空指针(失败)
 */
sbuf_t *sbuf_alloc(_SLINE1_);

/**
 * 释放sbuf，将一个sbuf归还回sbuf链表中
 *
 * @param **sbuf 指向欲归还的sbuf
 */
void sbuf_free(sbuf_t **sbuf _SLINE2_);

#endif

/**
 * @}
 */

