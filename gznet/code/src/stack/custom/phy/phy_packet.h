/**
 * @brief       : 
 *
 * @file        : phy_packet.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#ifndef __PHY_PACKET_H__
#define __PHY_PACKET_H__

#include "common/lib/lib.h"
#include "stack/common/pbuf.h"

#define PHY_HEAD_SIZE       3u  //物理帧帧头长度

#define PHY_LEN_FEILD_SIZE  1u  //物理帧长度域帧头长度

#define PHY_FCS_SIZE        0u

typedef uint8_t             phy_addr_t;

/**
 * 将芯片缓冲中数据拷贝到协议数据包缓冲单元
 *
 * @return 已填入数据的数据包缓冲单元
 */
pbuf_t *phy_get_packet(void);


/**
 * 向物理芯片写入数据
 *
 * @param buf:  待写入的数据起始地址
 * @param stamp_size: 后续添加的字节数
 *
 * @return  void
 */
bool_t phy_write_buf(pbuf_t *pbuf, uint8_t stamp_size);


/**
 * 操作RF发送芯片缓冲中的数据
 *
 * @return  void
 */
bool_t phy_send_data(void);

#endif
