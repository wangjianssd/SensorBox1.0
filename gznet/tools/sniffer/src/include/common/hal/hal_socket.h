/**
 * @brief       : w5100驱动接口
 *
 * @file        : hal_socket.h
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      shenghao.xu    some notes
 */
#pragma once
#include <data_type_def.h>
#include <socket.h>

typedef  struct
{
	uint8_t msg;			//中断消息
	uint8_t card;			//网卡id
    uint8_t port;           //socket号
}hal_socket_msg_t;

typedef  struct
{
	uint8_t *buf;			//载荷
	uint16_t length;		//载荷长度
	uint16_t seq;			//序列号
	uint8_t
		ag_send	:1,			//是否重传
		ag_num	:2;			//重传次数
}hal_socket_buf_t;

enum
{
    CONFIG_REPLAY_NON,
    CONFIG_REPLAY_SUCCESS,
    CONFIG_REPLAY_FAILED,
};

void hal_interrupt_maintenance(uint8_t index, uint8_t port);
/**
 * 初始化网口
 *
 * @author xushenghao (2013-9-9)
 */
void hal_socket_init(void);

/**
 * 软模式打开接收功能
 *
 * @author xushenghao (2013-9-9)
 *
 * @param index 		网卡ID
 */
void hal_socket_recv_enable();

/**
 * 软模式关闭接收功能
 *
 * @author xushenghao (2013-9-9)
 *
 * @param index
 */
void hal_socket_recv_disenable();

/**
 * 返回socket连接状态
 *
 * @author xushenghao (2013-9-11)
 *
 * @param index
 *
 * @return bool_t TRUE:连接成功
 */
bool_t hal_socket_card_cnn_state(uint8_t index);

/**
 * 获取接收区字节数
 *
 * @author xushenghao (2013-9-9)
 *
 * @param index 		网卡ID
 *
 * @return uint16_t 	返回数据长度
 */
uint16_t hal_socket_rxfifo_cnt(hal_socket_msg_t *W5100);

/**
 * 接收数据
 *
 * @author xushenghao (2013-9-9)
 *
 * @param index
 *
 * @return bool_t
 */
bool_t hal_socket_read(hal_socket_msg_t *W5100);

/**
 * 发送数据
 *
 * @author xushenghao (2013-9-9)
 *
 * @param index 				网卡ID
 *
 * @return bool_t
 */
bool_t hal_socket_send(const hal_socket_buf_t *hal_socket_buf, uint8_t index);

/**
 * 从重传队列中移除
 *
 * @author xushenghao (2013-9-16)
 *
 * @param seq 					序列号
 *
 * @return bool_t
 */
bool_t hal_socket_cache_remove(uint16_t seq);


void hal_socket_cnn(uint8_t index,uint8_t port);

bool_t hal_tcp_service_read(hal_socket_msg_t *W5100);

void hal_socket_discon_deal(uint8_t index_card);
