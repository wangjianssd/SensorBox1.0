/**
 * @brief       : 
 *
 * @file        : nwk_addr.h
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/12/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/12/7    v0.0.1      WangJifang    some notes
 */
#include "../../mac/general/mac_frame.h"
#include "nwk_frames.h"
#define ROUTE_NUM                                           (5u)
#define DETEC_NUM                                           (ASSOC_NUM)

#define UNDEFINE_NWK_ADDR                                   (0x0000)
#define UNDEFINE_NUI                                        (0x0000000000000000)

/**
 * @brief 地址表初始化
 * @param[in] void
 * @return void
 */
void nwk_addr_table_init(void);

/**
 * @brief 地址表删除对应的地址，回收资源
 * @param[in] nwk_addr 要回收的网络地址
 * @return 更新成功或失败
 *  - true 回收成功
 *  - false 回收失败
 */
bool_t nwk_addr_del(uint16_t nwk_addr);

/**
 * @brief 地址表添加对应的地址
 * @param[in] nwk_addr 要添加的设备信息
 * @return 更新成功或失败
 *  - true节点更新成功
 *  - false 更新失败
 */
bool_t nwk_addr_table_add(nwk_join_req_t nwk_addr);

/**
 * @brief 查找地址表对应的网络地址
 * @param[in] nwk_join_req 要查找的设备信息
 * @return 查找结果
 *  - UNDEFINE_NWK_ADDR 查找失败
 */
uint16_t nwk_addr_get(nwk_join_req_t *nwk_join_req);