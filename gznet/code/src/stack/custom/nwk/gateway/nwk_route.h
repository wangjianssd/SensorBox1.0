/***************************************************************************
 * @brief        : this
 * @file         : nwk_route.h
 * @version      : v0.1
 * @author       : gang.cheng
 * @date         : 2015-08-06
 * @change Logs  :
 * Date        Version      Author      Notes
 * 2015-08-06      v0.1      gang.cheng    first version
 ***************************************************************************/
#ifndef __NWK_ROUTE_H__
#define __NWK_ROUTE_H__

#include "../../mac/general/general.h"
#include "nwk_addr.h"

#define ROUTE_NODE_LIVE_MAX         (4u) //*< 路由表存活最大4个心跳周期

#define ROUTE_NODE_MAX              (DETEC_NUM*ROUTE_NUM)

typedef struct { 
    uint64_t nui;                   //*< NUI地址，作为散列值
    uint16_t nwk_addr;              //*< 要查找的目的网络短地址
    uint16_t next_hop;              //*< 要链路到目的地址的下一跳短地址
    uint8_t hop_num;                //*< 到达目的地址需要的跳数
    uint8_t live_time;              //*< 路由表项存活周期
} route_entry_t;

/**
 * @brief 网络路由模块初始化，输入路由表最大数量以及路由表节点存活周期
 * @param[in] route_node_num 路由表的最大容量
 * @param[in] live_cnt 路由表节点存活最大周期（一次心跳作为一次周期）
 * @return 初始化是否成功
 *  - 0 初始化成功
 *  - -1 初始化失败
 */
int8_t nwk_route_module_init(int32_t live_cnt);

/**
 * @brief 网络路由模块反初始化，释放申请的资源
 * @return 释放成功或失败
 *  - 0 反初始化成功
 *  - -1 反初始化失败
 */
int8_t nwk_route_module_deinit(void);

/**
 * @brief 网络路由管理模块更新，把超过最大存活周期还没有刷新的节点剔除掉，把缓冲区资源释放出来
 */
void nwk_route_module_update(void);

/**
 * @brief 网络路由管理模块节点插入，把节点以及到达节点的下一跳网络地址更新，如果
 *        节点已经存在则更新信息，如果节点不存在则插入一条新信息
 * @param[in] entry 指向路由节点信息结构体的指针
 * @return 更新成功或失败
 *  - 0 节点路由链路更新成功
 *  - -1 资源不够，节点路由链路无法更新进去
 */
bool_t nwk_route_module_insert(route_entry_t *entry);

/**
 *@brief 刷新路由表某个网络节点
 *@param[in] nwk_addr 网络设备的网络地址
 *@return 刷新成功或失败
 *  - 0 路由表节点刷新成功
 *  - -1 路由表节点不存在
 */
bool_t nwk_route_module_refresh(uint16_t nwk_addr);

/**
 * @brief 找到网络目的地址的下一跳MAC地址
 * 
 * @param[in] nwk_addr 要查询的网络目的短地址
 *
 * @return !NULL 查找到链路, NULL 没有查找到下一跳地址
 */
route_entry_t *nwk_route_module_nexthop(uint16_t nwk_addr);

bool_t nwk_route_moduel_fresh(route_entry_t *entry);


/**
 * @brief 网络路由管理模块查找到某个地址的下一跳地址
 * @param[in] key 要查找的目的网络节点的关键key
 * @return 
 *  - NULL 网络地址查找不到对应的路由信息
 *  - !NULL 查找到了路由节点信息
 */
route_entry_t * nwk_route_module_get(uint64_t key);

#endif
