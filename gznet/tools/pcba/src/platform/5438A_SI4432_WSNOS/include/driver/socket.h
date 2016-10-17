/**
 * @brief       : w5100驱动
 *
 * @file        : socket.h
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/5/8
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/8    v0.0.1      shenghao.xu    some notes
 */
#pragma once
#include <data_type_def.h>


#define S_RX_SIZE	2048		/*定义Socket接收缓冲区的大小，可以根据W5100_RMSR的设置修改(2048) */
#define S_TX_SIZE	2048  		/*定义Socket发送缓冲区的大小，可以根据W5100_TMSR的设置修改(2048) */

#define COMMON_BASE					0x0000				//Base address
#define GATEWAYADDRESS 				COMMON_BASE+0x01	//网关地址
#define	SUBWAYADDRESS				COMMON_BASE+0x05	//子网掩码
#define	HARDWAREWAYADDRESS			COMMON_BASE+0x09	//硬件网络物理地址
#define IPADDRESS					COMMON_BASE+0x0f	//本机IP
#define RMSR						COMMON_BASE+0x1a	//接收缓冲区
#define TMSR						COMMON_BASE+0x1b	//发送缓冲区
#define RTR							COMMON_BASE+0x17	//重传时间
#define RCR							COMMON_BASE+0X19	//重传次数
#define	IMR							COMMON_BASE+0x16	//R/W, Default=0x00
#define IMR_CONFLICT				0x80		//IP conflict
#define IMR_UNREACH					0x40		//Destination unreachable
#define IMR_PPPOE					0x20		//PPOE close
#define IMR_S3_INT					0x08		//Occurrence of socket 3 socket interrupt
#define IMR_S2_INT					0x04		//Occurrence of socket 2 socket interrupt
#define IMR_S1_INT					0x02		//Occurrence of socket 1 socket interrupt
#define IMR_S0_INT					0x01		//Occurrence of socket 0 socket interrupt

#define MODE_RST				0x80		//Software reset
#define MODE_MT					0x20		//Memory test
#define MODE_PB					0x10		//Ping block mode
#define MODE_PPPOE				0x08		//PPOE mode
#define MODE_LB					0x04		//Little_endian/Big_endian ordering in indirect bus I/F
#define MODE_AI					0x02		//Address auto increment in indirect bus I/F
#define MODE_IND				0x01		//Indirect bus I/F mode

#define S_MR_CLOSED		0x00		//  0    0    0    0      Closed
#define S_MR_TCP		0x01		//  0    0    0    1      TCP
#define S_MR_UDP		0x02		//  0    0    1    0      UDP
#define S_MR_IPRAW		0x03		//  0    0    1    1      IPRAW
#define S_MR_MACRAW		0x04		//  0    1    0    0      MACRAW
#define S_MR_PPPOE		0x05		//  0    1    0    1      PPPOE

//Socket maximum segment size register, R/W, default=0x00
#define W5100_S0_MSS	COMMON_BASE+0x0412		//Socket 0
#define W5100_S1_MSS	COMMON_BASE+0x0512		//Socket 1
#define W5100_S2_MSS	COMMON_BASE+0x0612		//Socket 2
#define W5100_S3_MSS	COMMON_BASE+0x0712		//Socket 3
//Socket Source port register, R/W, default=0x00
#define W5100_S0_PORT	COMMON_BASE+0x0404		//Socket 0
#define W5100_S1_PORT	COMMON_BASE+0x0504		//Socket 1
#define W5100_S2_PORT	COMMON_BASE+0x0604		//Socket 2
#define W5100_S3_PORT	COMMON_BASE+0x0704		//Socket 3
//Socket destionation port register, R/W, default=0x00
#define W5100_S0_DPORT	COMMON_BASE+0x0410		//Socket 0
#define W5100_S1_DPORT	COMMON_BASE+0x0510		//Socket 1
#define W5100_S2_DPORT	COMMON_BASE+0x0610		//Socket 2
#define W5100_S3_DPORT	COMMON_BASE+0x0710		//Socket 3
//Socket destination IP address register, R/W, default=0x00
#define W5100_S0_DIPR	COMMON_BASE+0x040c		//Socket 0
#define W5100_S1_DIPR	COMMON_BASE+0x050c		//Socket 1
#define W5100_S2_DIPR	COMMON_BASE+0x060c		//Socket 2
#define W5100_S3_DIPR	COMMON_BASE+0x070c		//Socket 3
//Socket mode register, R/W, default=0x00
#define W5100_S0_MR	COMMON_BASE+0x0400		//Socket 0
#define W5100_S1_MR	COMMON_BASE+0x0500		//Socket 1
#define W5100_S2_MR	COMMON_BASE+0x0600		//Socket 2
#define W5100_S3_MR	COMMON_BASE+0x0700		//Socket 3
//Socket command register, R/W, default=0x00
#define W5100_S0_CR	COMMON_BASE+0x0401		//Socket 0
#define W5100_S1_CR	COMMON_BASE+0x0501		//Socket 1
#define W5100_S2_CR	COMMON_BASE+0x0601		//Socket 2
#define W5100_S3_CR	COMMON_BASE+0x0701		//Socket 3
#define S_CR_OPEN	0x01		//It is used to initialize the socket
#define S_CR_LISTEN	0x02		//In TCP mode, it waits for a connection request from any remote
//peer(Client)
#define S_CR_CONNECT	0x04		//In TCP mode, it sends a connection request to remote peer(SERVER)
#define S_CR_DISCON	0x08		//In TCP mode, it sends a connection termination request
#define S_CR_CLOSE	0x10		//Used to close the socket
#define S_CR_SEND	0x20		//It transmit the data as much as the increase size of write pointer
#define S_CR_SEND_MAC	0x21		//In UDP mode, same as SEND
#define S_CR_SEND_KEEP	0x22		//In TCP mode
#define S_CR_RECV	0x40		//Receiving is processed including the value of socket RX read
//pointer register

/* Definition for PPPOE */
#define S_CR_PCON	0x23		//Start of ADSL connection
#define S_CR_PDISCON	0x24		//End of ADSL connection
#define S_CR_PCR	0x25		//Send REQ message ineach phase
#define S_CR_PCN	0x26		//Send NAK message in each phase
#define S_CR_PCJ	0x27		//Senf REJECT message in each phase
//Socket status register, RO, default=0x00
#define W5100_S0_SSR	COMMON_BASE+0x0403		//Socket 0
#define W5100_S1_SSR	COMMON_BASE+0x0503		//Socket 1
#define W5100_S2_SSR	COMMON_BASE+0x0603		//Socket 2
#define W5100_S3_SSR	COMMON_BASE+0x0703		//Socket 3
#define S_SSR_CLOSED	0x00		//In case that OPEN command are given to Sn_CR, Timeout interrupt
//is asserted or connection is terminated
#define S_SSR_INIT	0x13		//In case that Sn_MR is set as TCP and OPEN commands are given to Sn_CR
#define S_SSR_LISTEN	0x14		//In case that under the SOCK_INIT status, LISTEN commands
//are given to Sn_CR
#define S_SSR_ESTABLISHED	0x17	//In case that connection is established
#define S_SSR_CLOSE_WAIT	0x1c	//In case that connection temination request is received
#define S_SSR_UDP	0x22		//In case that OPEN command is given to Sn_CR when Sn_MR is set as UDP
#define S_SSR_IPRAW	0x32		//In case that OPEN command is given to Sn_CR when Sn_MR is set as IPRAW
#define S_SSR_MACRAW	0x42		//In case that OPEN command is given to S0_CR when S0_MR is set as MACRAW
#define S_SSR_PPPOE	0x5f		//In case that OPEN command is given to S0_CR when S0_MR is set as PPPOE
//Below is shown in the status change, and does not need much attention
#define S_SSR_SYNSEND	0x15
#define S_SSR_SYNRECV	0x16
#define S_SSR_FIN_WAIT	0x18
#define S_SSR_CLOSING	0x1a
#define S_SSR_TIME_WAIT	0x1b
#define S_SSR_LAST_ACK	0x1d
#define S_SSR_ARP0	0x11
#define S_SSR_ARP1	0x21
#define S_SSR_ARP2	0x31

//Interrupt and interrupt mask register
#define	W5100_IR	COMMON_BASE+0x15	//RO, Default=0x00
#define IR_CONFLICT	0x80		//IP conflict
#define IR_UNREACH	0x40		//Destination unreachable
#define IR_PPPOE	0x20		//PPOE close
#define IR_S3_INT	0x08		//Occurrence of socket 3 socket interrupt
#define IR_S2_INT	0x04		//Occurrence of socket 2 socket interrupt
#define IR_S1_INT	0x02		//Occurrence of socket 1 socket interrupt
#define IR_S0_INT	0x01		//Occurrence of socket 0 socket interrupt

//Socket interrup register, RO, default=0x00
#define W5100_S0_IR	COMMON_BASE+0x0402		//Socket 0
#define W5100_S1_IR	COMMON_BASE+0x0502		//Socket 1
#define W5100_S2_IR	COMMON_BASE+0x0602		//Socket 2
#define W5100_S3_IR	COMMON_BASE+0x0702		//Socket 3
#define S_IR_SENDOK	0x10		//Send data complete
#define S_IR_TIMEOUT	0x08		//Set if timeout occurs during connection or termination
//or data transmission
#define S_IR_RECV		0x04		//Set if data is received
#define S_IR_DISCON	0x02		//Set if receiv connection termination request
#define S_IR_CON		0x01		//Set if connetion is established

//Socket TX free size register, RO, default=0x0800
//should read upper byte first and lower byte later
#define W5100_S0_TX_FSR		COMMON_BASE+0x0420		//Socket 0
#define W5100_S1_TX_FSR		COMMON_BASE+0x0520		//Socket 1
#define W5100_S2_TX_FSR		COMMON_BASE+0x0620		//Socket 2
#define W5100_S3_TX_FSR		COMMON_BASE+0x0720		//Socket 3

//Socket TX read pointer register, RO, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_TX_RR		COMMON_BASE+0x0422		//Socket 0
#define W5100_S1_TX_RR		COMMON_BASE+0x0522		//Socket 1
#define W5100_S2_TX_RR		COMMON_BASE+0x0622		//Socket 2
#define W5100_S3_TX_RR		COMMON_BASE+0x0722		//Socket 3

//Socket TX write pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_TX_WR		COMMON_BASE+0x0424		//Socket 0
#define W5100_S1_TX_WR		COMMON_BASE+0x0524		//Socket 1
#define W5100_S2_TX_WR		COMMON_BASE+0x0624		//Socket 2
#define W5100_S3_TX_WR		COMMON_BASE+0x0724		//Socket 3
//Socket RX read pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_RX_RSR		COMMON_BASE+0x0426		//Socket 0
#define W5100_S1_RX_RSR		COMMON_BASE+0x0526		//Socket 1
#define W5100_S2_RX_RSR		COMMON_BASE+0x0626		//Socket 2
#define W5100_S3_RX_RSR		COMMON_BASE+0x0726		//Socket 3
//should read upper byte first and lower byte later
#define W5100_S0_RX_RR		COMMON_BASE+0x0428		//Socket 0
#define W5100_S1_RX_RR		COMMON_BASE+0x0528		//Socket 1
#define W5100_S2_RX_RR		COMMON_BASE+0x0628		//Socket 2
#define W5100_S3_RX_RR		COMMON_BASE+0x0728		//Socket 3
//Socket RX read pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_RX_WR		COMMON_BASE+0x042A		//Socket 0
#define W5100_S1_RX_WR		COMMON_BASE+0x052A		//Socket 1
#define W5100_S2_RX_WR		COMMON_BASE+0x062A		//Socket 2
#define W5100_S3_RX_WR		COMMON_BASE+0x072A		//Socket 3

//TX memory
#define W5100_TX	COMMON_BASE+0x4000

//RX memory
#define W5100_RX	COMMON_BASE+0x6000


/******************自定义*********************/
#define CARD_NUM			2u			//网卡数量
#define CARD_0				0x00
#define CARD_1				0x01
#define PORT_0				0x00
#define PORT_1				0x01
#define PORT_2				0x02
#define PORT_3				0x03
#define MAX_ZONE			0xff	//最大分片数

#define TCP_SERVICE_PORT    PORT_3

#define UDP_CLIENT_MODE		BIT0
#define TCP_CLIENT_MODE		BIT1
#define TCP_SERVER_MODE		BIT2

//中断宏定义
#define SOCKET_RX_OK_INT		                    0u
#define SOCKET_TX_OK_INT		                    1u
#define SOCKET_CON_OK_INT		                    2u
#define SOCKET_DISCON_OK_INT		                3u
#define SOCKET_INT_MAX_NUM                          4u

#define IINCHIP_ISR_DISABLE() (TA0CCTL3 &= ~CCIE)
#define IINCHIP_ISR_ENABLE()  (TA0CCTL3 |= CCIE)

typedef void (*socket_int_reg_t)(uint8_t w5100_index,uint8_t port);
extern socket_int_reg_t socket_int_reg[SOCKET_INT_MAX_NUM];

#pragma pack(1)
//网卡信息
typedef struct
{
	uint8_t ip_addr[4];			//IP地址
	uint8_t sub_mask[4];		//子网掩码
	uint8_t gateway_ip[4];		//网关IP
	uint8_t port[2];			//本地端口
	uint8_t phy_addr[6];		//物理地址
	uint8_t msg_mode;			//通讯方式(UDP_CLIENT_MODE|TCP_CLIENT_MODE|TCP_SERVER_MODE)

	uint8_t dip_addr[4];		//目的IP
	uint8_t dport[2];			//目的端口

	uint8_t cnn_state	:1,		//连接状态
			recv_state	:1;		//是否允许接收数据
}w5100_info_t;

void socket_interrupt_clear(uint8_t w5100_index, uint8_t port);   //中断复位
#pragma pack()
/**
 * 获取接收缓冲区内容大小
 *
 * @author xushenghao (2013-5-22)
 *
 * @param w5100_index 			网卡序号
 * @param socket_port 			socket端口号
 *
 * @return uint16_t 			返回接收缓冲区的字节数
 */
uint16_t socket_get_rxfifo_cnt(uint8_t w5100_index, uint8_t socket_port);

void socket_read_fifo_only(uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port);
/**
 * 读取接收缓冲区内容
 *
 * @author xushenghao (2013-5-22)
 *
 * @param p_data 				存储接收内容的BUF
 * @param count 				读取内容长度
 * @param w5100_index 			网卡序号
 * @param socket_port 			socket端口号
 */
void socket_read_fifo(uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port);

/**
 * 将要发送的内容写入发送缓冲区并发送
 *
 * @author xushenghao (2013-5-22)
 *
 * @param p_data 				存储发送内容的BUF
 * @param count 				发送内容长度
 * @param w5100_index 			网卡序号
 * @param socket_port 			socket端口号
 *
 * @return bool_t 				失败:缓冲区长度不够
 *  	   						成功:发送成功
 */
bool_t socket_write_fifo_send(const uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port);

/**
 * 中断处理函数
 *
 * @author xushenghao (2013-5-22)
 *
 * @param w5100_index 			网卡序号
 */
void socket_interrupt_process(uint8_t w5100_index);

/**
 * 清中断
 *
 * @author xushenghao (2013-5-22)
 *
 * @param w5100_index 			网卡序号
 */
void socket_clear_iar(uint8_t w5100_index, uint8_t port, uint8_t value);

/**
 *	网络初始化
 *
 * @author xushenghao (2013-4-9)
 *
 */
void socket_init();

/**
 * 设置网络模式
 *
 * @author xushenghao (2013-9-9)
 *
 * @param w5100_index
 * @param mode
 * @param socket_port
 *
 * @return bool_t
 */
bool_t socket_mode(uint8_t w5100_index, uint8_t mode, uint8_t socket_port);

/**
 * 连接网络
 *
 * @author xushenghao (2013-9-9)
 *
 * @param w5100_index
 */
void socket_cnn(uint8_t w5100_index,uint8_t port);

/**
 * 将配置好的信息写入进网卡
 *
 * @author xushenghao (2013-9-9)
 *
 * @param w5100_index
 * @param socket_port
 */
void socket_conf_init(uint8_t w5100_index, uint8_t socket_port);

void socket_server_reset(uint8_t w5100_index, uint8_t port);

