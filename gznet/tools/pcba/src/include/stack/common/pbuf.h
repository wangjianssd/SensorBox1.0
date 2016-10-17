/**
 * @brief       : configuration and interface of pbuf.c
 *
 * @file        : pbuf.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */


#ifndef __PBUF_H
#define __PBUF_H

#include <data_type_def.h>
#include <node_cfg.h>
#include <lib.h>

#define PBUF_DBG_EN                 (1u)

#if PBUF_DBG_EN > 0

/*形参*/
#define _PLINE1_  ,uint16_t line
#define _PLINE2_  ,uint16_t line
/*实参*/
#define __PLINE1  ,__LINE__
#define __PLINE2  ,__LINE__

#else

#define _PLINE1_
#define _PLINE2_
#define __PLINE1
#define __PLINE2

#endif

typedef uint16_t nwk_id_t;


typedef struct __send_times_t
{
    uint8_t app_send_times;
    uint8_t nwk_send_times;
    uint8_t mac_send_times;
} send_times_t;

typedef struct
{
    int8_t rssi_dbm;
    uint8_t  seq;

    nwk_id_t src_id;        // 接收到数据帧时，为同步模块提供同步对象信息；
    nwk_id_t dst_id;        // 填写帧的目的节点网络地址

    uint8_t  send_mode  : 2,
             is_ack     : 1,
             need_ack   : 1,
             crc_ok     : 1,
             is_pending : 1,
             debug_info : 1,
             reserved   : 1;
    uint8_t mac_length;
    send_times_t already_send_times;
} pkt_attri_t;

typedef struct
{
    struct list_head list;
    uint8_t *data_p;        //指向数据区
    uint8_t *head;          //指向数据区的第一个字节
    uint8_t *end;           //指向数据区的最后一个字节
    uint8_t data_len;       //该pbuf的实际数据长度
    pkt_attri_t attri;
    bool_t used;
#if PBUF_DBG_EN > 0
    uint16_t alloc_line;
    uint16_t free_line;
#endif
} pbuf_t;

/**
 * pbuf_init: 为pbuf申请一块内存区域，需要配置各种pbuf的大小和数量等
 */
void pbuf_init(void);

/**
 * 申请一个pbuf，用来存放用户数据
 *
 * @param size: 用户的数据长度
 * @param _PLINE1_: pbuf_alloc()位置的行号，调用时传入实参形式__PLINE1
 *
 * @return: 申请成功则返回pbuf的指针，失败则进入断言
 */
pbuf_t *pbuf_alloc(uint8_t size _PLINE1_);

/**
 * 释放已经使用完的pbuf
 *
 * @param pbuf: 需要操作的pbuf的指针的指针
 * @param _PLINE2_: 调用pbuf_free()位置的行号，调用时传入实参形式__PLINE2
 *
 * @return: 无
 */
void pbuf_free(pbuf_t **const pbuf _PLINE2_);

/**
 * 向pbuf->end方向移动pbuf->data_p指针，移动距离为len
 *
 * @param pbuf: 需要操作的pbuf的指针
 * @param len: data_p需要移动的距离
 *
 * @return: 成功则返回data_p指针，失败返回NULL
 */
uint8_t *pbuf_skip_datap_forward(pbuf_t *const pbuf,
                                 uint8_t len);

/**
 * 向pbuf->head方向移动pbuf->data_p指针，移动距离为len
 *
 * @param pbuf: 需要操作的pbuf的指针
 * @param len: data_p需要移动的距离
 *
 * @return: 成功则返回data_p指针，失败返回NULL
 */
uint8_t *pbuf_skip_datap_backward(pbuf_t *const pbuf,
                                  uint8_t len);

/**
 * 向pbuf的数据区拷贝数据，并移动data_p指针，改变data_len
 *
 * @param pbuf: 目的地址pbuf的指针(从pbuf->data_p开始拷贝)
 * @param src: 源地址的指针
 * @param len: 需要拷贝的数据长度
 *
 * @return: 成功则返回TRUE, 失败则返回FALSE
 */
bool_t pbuf_copy_data_in(pbuf_t *const pbuf,
                         const uint8_t *const src,
                         uint8_t len);

/**
 * 从pbuf的数据区拷贝数据，并移动data_p指针，不改变data_len
 *
 * @param dst: 目的地址的指针
 * @param pbuf: 源地址pbuf的指针(从pbuf->data_p开始拷贝)
 * @param len: 需要拷贝的数据长度
 *
 *  @return: 成功则返回TRUE, 失败则返回
 */
bool_t pbuf_copy_data_out(uint8_t *const dst,
                          pbuf_t *const pbuf,
                          uint8_t len);

#endif
/**
 * @}
 */

