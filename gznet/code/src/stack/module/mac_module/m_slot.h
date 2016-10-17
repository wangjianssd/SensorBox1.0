/**
 * @brief       : configuration and interface of m_slot.c
 *
 * @file        : m_slot.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __M_SLOT_H
#define __M_SLOT_H

#include "common/lib/lib.h"
#include "hal_timer.h"

/* 时间同步类型定义 */
#define SLOT_GLOBAL_TIME        1
#define SLOT_LOCAL_TIME         0

/* 时隙描述表 */
typedef struct _slot_cfg_t
{

    uint32_t slot_duration;                 // 时隙持续时间，用户需要配置
    uint8_t  slot_repeat_cnt;               // 时隙重复次数，为0表示永远循环，用户需要配置
    void (*func)(void *args);               // 时隙回调函数，用户需要配置

    struct _slot_cfg_t *parent;             // 当前时隙节点的父节点，用户需要配置
    struct _slot_cfg_t *first_child;        // 当前时隙节点是否有孩子节点，用户需要配置
    struct _slot_cfg_t *next_sibling;       // 当前时隙节点是否有后续兄弟节点，用户需要配置

    uint32_t slot_start;                    // 时隙起始，用户不需要配置
    uint32_t slot_record_start;             // 时隙触发以后，记录当前的时隙起始，用作剩余时间判断
    uint8_t  slot_repeat_seq;               // 表示重复时隙当前处于第几个，用户不需要配置
    
    hal_timer_t *slot_timer;                //*< 定义时隙的定时器句柄
} slot_cfg_t;


/**
 * 时隙模块初始化
 *
 * @param:              无
 *
 * @return:             无
 */
void m_slot_init(osel_task_t *task);

/**
 * 获取当前时隙模块运行状态
 *
 * @param:              无
 *
 * @return:             1表示正在运行，0表示停止
 */
bool_t m_slot_get_state(void);

/**
 * 获取当前时隙的剩余tick数
 *
 * @param:              无
 *
 * @return:             返回叶子时隙剩余tick数，非叶子节点返回0
 */
uint32_t m_slot_get_remain_time(void);

/**
 * 获取根节点的起始时间
 *
 * @param:              无
 *
 * @return:             返回根节点的起始时间
 */
uint32_t m_slot_get_root_begin(void);

/**
 * 获取当前时隙的序列号，如果没有运行到叶子时隙，结果是保留上次的时隙号
 *
 * @param:              无
 *
 * @return:             当前叶子时隙的序列号
 */
uint16_t m_slot_get_seq(void);

/**
 *  向时隙模块注册时隙描述表，并且设置时间同步类型
 *
 * @param supfrm:       时隙配置表的指针
 * @param type:         时间同步类型，如果time_type == SLOT_GLOBAL_TIME, 则时隙采用本地时间，
 *                      否则采用全局时间
 *
 * @return:             无
 */
void m_slot_cfg(slot_cfg_t *const sup_frame, uint8_t type);

/**
 * 时隙开始运行,时隙是从一个没有过期的超帧起始点开始启动
 *
 * @param start_time:   start_time 的指针
 *
 * @return:             无
 */
void m_slot_run(hal_time_t *start_time);

/**
 * 时隙强制停止，需要等待一个MO时间来确保时隙全部停止
 *
 * @param :             无
 *
 * @return:             无
 */
void m_slot_stop(void);


#endif
