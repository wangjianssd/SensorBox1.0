/**
 * @brief       : w5100驱动
 *
 * @file        : socket.c
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/5/8
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/8    v0.0.1      shenghao.xu    some notes
 */

#include <socket.h>
#include <osel_arch.h>
#include <debug.h>
#include <driver.h>
/*网络参数配置*/
w5100_info_t w5100_dev[CARD_NUM];

socket_int_reg_t socket_int_reg[SOCKET_INT_MAX_NUM];

volatile bool_t tcps_tx_ok = TRUE;

#define SPI_BEGIN(w5100_index)              		                    \
do                                                                      \
    {                                                                   \
        osel_int_status_t s;                                            \
            OSEL_ENTER_CRITICAL(s);                                     \
                P3OUT &= ~BIT6;							                \
                    OSEL_EXIT_CRITICAL(s);                              \
    } while(__LINE__ == -1)
        
#define SPI_END(w5100_index) 						                    \
    {                                                                   \
        osel_int_status_t s;                                            \
            OSEL_ENTER_CRITICAL(s);                                     \
                (P3OUT |=  BIT6);                                       \
                    OSEL_EXIT_CRITICAL(s);                              \
    }

#define SPI_SEND_CHAR(w5100_index,x)                                    \
do                                                                      \
    {                                                                   \
        osel_int_status_t s;                                            \
            OSEL_ENTER_CRITICAL(s);                                     \
                uint8_t i   =  0;                                       \
                    if (w5100_index == 0)						        \
                        {											    \
                            while (!(UCB1IFG&UCTXIFG))                  \
                                {                                       \
                                    if(i++ > 200)                       \
                                        WDTCTL = 0xFFFF;                \
                                }                                       \
                                    UCB1TXBUF = (x);		            \
                                        while (!(UCB1IFG&UCRXIFG))      \
                                            {                           \
                                                if(i++ > 200)           \
                                                    WDTCTL = 0xFFFF;    \
                                            }                           \
                        }											    \
                    else										        \
                        {											    \
                            while (!(UCB1IFG&UCTXIFG));                 \
                                UCB1TXBUF = (x);		                \
                                    while (!(UCB1IFG&UCRXIFG));         \
                        }											    \
                            UCB1IFG &= ~UCRXIFG;                        \
                                OSEL_EXIT_CRITICAL(s);                  \
    } while(__LINE__ == -1)
        
#define SPI_RECEIVE_CHAR(w5100_index,buf)			                    \
    {                                                                   \
        osel_int_status_t s;                                            \
            OSEL_ENTER_CRITICAL(s);                                     \
                buf = (UCB1RXBUF);                                      \
                    OSEL_EXIT_CRITICAL(s);                              \
    }

static void write_reg(uint8_t w5100_index, uint16_t addr, uint8_t value)
{                  
    SPI_BEGIN(w5100_index);
    SPI_SEND_CHAR(w5100_index, 0xF0);		// Send the header byte
    SPI_SEND_CHAR(w5100_index, addr>>8);
    SPI_SEND_CHAR(w5100_index, addr);
    SPI_SEND_CHAR(w5100_index, value);
    SPI_END(w5100_index);
}

static uint8_t read_reg(uint8_t w5100_index ,uint16_t addr)
{
    uint8_t temp;
    SPI_BEGIN(w5100_index);
    SPI_SEND_CHAR(w5100_index, 0x0F);		// Send the header byte
    SPI_SEND_CHAR(w5100_index, addr>>8);
    SPI_SEND_CHAR(w5100_index, addr);
    SPI_SEND_CHAR(w5100_index, 0xFF);
    SPI_RECEIVE_CHAR(w5100_index,temp);
    SPI_END(w5100_index);
    return temp;
}
static void spi_init()
{
    UCB1CTL1 |= UCSWRST;					// Put state machine in reset for reconfiguration
    UCB1CTL0 = UCCKPH + UCMSB + UCMST + UCSYNC;
    UCB1CTL1 = UCSSEL1 + UCSSEL0;
    UCB1BR0 =  0x02;
    UCB1BR1 =  0x00;						// fBitClock = fBRCLK/UCBRx = SMCLK
    
    UCB1CTL1 &= ~UCSWRST;					// Initialize USCI state machine
    UCB1IE &= ~UCRXIE;						// Receive interrupt disabled
    
    P3SEL &= ~BIT6;							//P5.5 as RF_STE
    P3SEL |= BIT7;
    P5SEL |= BIT4 + BIT5;
    
    P3DIR |= BIT6 + BIT7;
    P5DIR |= BIT5;
    P5DIR &= ~BIT4;
    
    P3OUT |= BIT6;							// P5.5 output high, RF disabled
}

static void socket_int_cfg(void)
{
    //	TA0CTL = TASSEL_1 + MC_2 + TACLR + TAIE;
    TA0CCTL3 &= ~CCIFG;
    TA0CCTL3 |= CCIE;
}

static void port_init()					//中断管脚配置
{
    P8SEL |= BIT3;
    P8DIR &= ~BIT3;	//nIRQ captured by TA1CCR2
    TA0CCTL3 = CM_2 + CCIS_1 + SCS + CAP; ///  下降沿capture，CCI3B，同步
}

static void socket_open_port(uint8_t w5100_index, uint8_t socket_port)
{
    uint8_t s = socket_port;
    write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_CLOSE);
    write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_OPEN);		//打开Socket端口
}

static void W5100rest()
{
    P7SEL &= ~BIT3;
    P7DIR |= BIT3;
    P7OUT &= ~BIT3;
    delay_ms(500);
    P7OUT |= BIT3;
    delay_ms(500);
}

void socket_conf_init(uint8_t w5100_index, uint8_t socket_port)					//初始化配置参数
{
    int i = 0;
    uint8_t s = socket_port;
    for (i=0;i<4;i++)
    {
        write_reg(w5100_index, IPADDRESS+i,w5100_dev[w5100_index].ip_addr[i]);					//设置本机IP
        write_reg(w5100_index, SUBWAYADDRESS+i, w5100_dev[w5100_index].sub_mask[i]);			//设置子网掩码
        write_reg(w5100_index, GATEWAYADDRESS+i, w5100_dev[w5100_index].gateway_ip[i]);			//设置网关的IP地址
        write_reg(w5100_index, W5100_S0_DIPR+(s<<8)+i,w5100_dev[w5100_index].dip_addr[i]);				//设置目标地址
    }
    
    write_reg(w5100_index, W5100_S0_PORT+(s<<8), w5100_dev[w5100_index].port[0]);	//设置本地端口号
    write_reg(w5100_index, W5100_S0_PORT+(s<<8)+1, w5100_dev[w5100_index].port[1]);
    write_reg(w5100_index, W5100_S0_DPORT+(s<<8), w5100_dev[w5100_index].dport[0]);	//设置目标端口号
    write_reg(w5100_index, W5100_S0_DPORT+(s<<8)+1,w5100_dev[w5100_index].dport[1]);
    
    for (i=0;i<6;i++)
        write_reg(w5100_index, HARDWAREWAYADDRESS+i,w5100_dev[w5100_index].phy_addr[i]);		//设置硬件网络地址
    
    write_reg(w5100_index, RMSR, 0x55);									//设置发送缓冲区
    write_reg(w5100_index, TMSR,0x55);									//设置接收缓冲区
    
    /* 设置重试时间，默认为2000(200ms) */
    write_reg(w5100_index, RTR,0x07);
    write_reg(w5100_index, RTR+1,0xd0);
    
    write_reg(w5100_index, RCR,8);										//设置重试次数，默认为8次
    
    
    write_reg(w5100_index, (W5100_S0_MSS+(s<<8)),MAX_ZONE);				//最大分片字节数=16
    write_reg(w5100_index, (W5100_S0_MSS+(s<<8)+1),0x10);
    write_reg(w5100_index,IMR,(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT|IMR_S1_INT|IMR_S2_INT|IMR_S3_INT));
}

void socket_server_reset(uint8_t w5100_index, uint8_t port)
{
    uint8_t s = port;
    uint8_t state = read_reg(w5100_index, W5100_S0_SSR+(s*0x100));
    if(state == S_SSR_CLOSE_WAIT || state == S_MR_CLOSED)
    {
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_CLOSE);
        socket_mode(w5100_index,w5100_dev[w5100_index].msg_mode,port);
    }
    socket_interrupt_clear(w5100_index,port); //在这里补偿清掉中断
}

bool_t socket_mode(uint8_t w5100_index, uint8_t mode, uint8_t socket_port)	//客户端模式
{
    uint8_t s = socket_port;
    
    if (((mode & UDP_CLIENT_MODE) == UDP_CLIENT_MODE) && (socket_port != TCP_SERVICE_PORT))
    {
        write_reg(w5100_index, (W5100_S0_MR+(s*0x100)), S_MR_UDP);		//设置Socket为UDP模式
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_OPEN);		//打开Socket
        if (read_reg(w5100_index, W5100_S0_SSR+(s*0x100))!=S_SSR_UDP)
        {
            write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_CLOSE);	//打开不成功，关闭Socket，然后返回
            return FALSE;
        }
    }
    else if (((mode & TCP_CLIENT_MODE) == TCP_CLIENT_MODE) && (socket_port != TCP_SERVICE_PORT))
    {
        write_reg(w5100_index, (W5100_S0_MR+(s*0x100)), S_MR_TCP);		//设置socket为TCP模式
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_OPEN);		//打开Socket
        if (read_reg(w5100_index, W5100_S0_SSR+(s*0x100))!=S_SSR_INIT)
        {
            write_reg(w5100_index, W5100_S0_CR+(s*0x100),S_CR_CLOSE);		//打开不成功，关闭Socket，然后返回
        }
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)),S_CR_CONNECT);		//设置Socket为Connect模式
        
        /*至此完成了Socket的打开连接工作，至于它是否与远程服务器建立连接，则需要等待Socket中断，
        以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态*/
        
    }
    else if (((mode & TCP_SERVER_MODE) == TCP_SERVER_MODE) && (socket_port == TCP_SERVICE_PORT))
    {
        for (int i=0;i<4;i++)
        {
            write_reg(w5100_index, W5100_S0_DIPR+(s<<8)+i,w5100_dev[w5100_index].dip_addr[i]+1);				//设置目标地址
        }
        write_reg(w5100_index, (W5100_S0_MR+(s*0x100)), S_MR_TCP);		//设置Socket为TCP模式
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_OPEN);		//打开Socket
        if (read_reg(w5100_index, W5100_S0_SSR+(s*0x100))!=S_SSR_INIT)
        {
            write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_CLOSE);	//打开不成功，关闭Socket，然后返回
            return FALSE;
        }
        
        write_reg(w5100_index, (W5100_S0_CR+(s*0x100)),S_CR_LISTEN);
        if (read_reg(w5100_index, W5100_S0_SSR+(s*0x100))!=S_SSR_LISTEN)
        {
            
            write_reg(w5100_index, (W5100_S0_CR+(s*0x100)), S_CR_CLOSE);	//设置不成功，关闭Socket，然后返回
            return FALSE;
        }
        /*至此完成了Socket的打开和设置侦听工作，至于远程客户端是否与它建立连接，则需要等待Socket中断，
        以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态在服务器侦听模式不需要设置目的IP和目的端口号*/
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
    return TRUE;
}

//获取接收缓冲区内容大小
uint16_t socket_get_rxfifo_cnt(uint8_t w5100_index, uint8_t socket_port)	//读取接收到的字节数
{
    uint8_t s = socket_port;
    uint16_t rx_size = 0;
    //读取接收数据的字节数
    rx_size = read_reg(w5100_index, W5100_S0_RX_RSR+(s<<8));
    rx_size *= 256;
    rx_size += read_reg(w5100_index, W5100_S0_RX_RSR+(s<<8)+1);
    return(rx_size);   //6个物理ID字节，2个长度字节
}

//读取接收缓冲区内容不偏移
void socket_read_fifo_only(uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port)
{
    uint16_t rx_offset = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint8_t s = socket_port;
    //读取接收缓冲区的偏移量
    rx_offset = read_reg(w5100_index, W5100_S0_RX_RR+(s<<8));
    rx_offset *= 256;
    rx_offset += read_reg(w5100_index, W5100_S0_RX_RR+(s<<8)+1);
    i = rx_offset/S_RX_SIZE;					//计算实际的物理偏移量
    rx_offset = rx_offset-i*S_RX_SIZE;
    j = W5100_RX+s*S_RX_SIZE+rx_offset;			//实际物理地址为W5100_RX+rx_offset
    for (i=0;i<count;i++)
    {
        if (rx_offset >= S_RX_SIZE)
        {
            j = W5100_RX+s*S_RX_SIZE;
            rx_offset = 0;
        }
        p_data[i] = read_reg(w5100_index,j);
        j++;
        rx_offset++;
    }
}

//读取接收缓冲区内容
void socket_read_fifo(uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port)
{
    uint16_t rx_offset = 0;
    uint16_t i = 0;
    uint16_t j = 0;
    uint8_t s = socket_port;
    //读取接收缓冲区的偏移量
    rx_offset = read_reg(w5100_index, W5100_S0_RX_RR+(s<<8));
    rx_offset *= 256;
    rx_offset += read_reg(w5100_index, W5100_S0_RX_RR+(s<<8)+1);
    i = rx_offset/S_RX_SIZE;					//计算实际的物理偏移量
    rx_offset = rx_offset-i*S_RX_SIZE;
    j = W5100_RX+s*S_RX_SIZE+rx_offset;			//实际物理地址为W5100_RX+rx_offset
    for (i=0;i<count;i++)
    {
        if (rx_offset >= S_RX_SIZE)
        {
            j = W5100_RX+s*S_RX_SIZE;
            rx_offset = 0;
        }
        p_data[i] = read_reg(w5100_index,j);
        j++;
        rx_offset++;
    }
    //计算下一次偏移量
    rx_offset = read_reg(w5100_index, W5100_S0_RX_RR+(s<<8));
    rx_offset *= 256;
    rx_offset += read_reg(w5100_index, W5100_S0_RX_RR+(s<<8)+1);
    
    rx_offset += count;
    write_reg(w5100_index, (W5100_S0_RX_RR+(s<<8)), (rx_offset/256));
    write_reg(w5100_index, (W5100_S0_RX_RR+(s<<8)+1), rx_offset);
    write_reg(w5100_index, (W5100_S0_CR+(s<<8)), S_CR_RECV);
}

//将要发送的内容写入发送缓冲区并发送
bool_t socket_write_fifo_send(const uint8_t *p_data, uint16_t count, uint8_t w5100_index, uint8_t socket_port)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint8_t s = socket_port;
    uint16_t tx_free_size = 0,tx_offset = 0;
    //读取缓冲区剩余的长度
    tx_free_size = read_reg(w5100_index, W5100_S0_TX_FSR+(s<<8));
    tx_free_size *= 256;
    tx_free_size += read_reg(w5100_index, W5100_S0_TX_FSR+(s<<8)+1);
    if (tx_free_size<count)						 //如果剩余的字节长度小于发送字节长度,则返回
        return FALSE;
    
    //读取发送缓冲区的偏移量
    tx_offset = read_reg(w5100_index, W5100_S0_TX_WR+(s<<8));
    tx_offset *= 256;
    tx_offset += read_reg(w5100_index, W5100_S0_TX_WR+(s<<8)+1);
    
    i = tx_offset/S_TX_SIZE;
    tx_offset = tx_offset-i*S_TX_SIZE;
    j = W5100_TX+s*S_TX_SIZE+tx_offset;
    
    for (i=0; i<count; i++)
    {
        if (tx_offset >= S_TX_SIZE)
        {
            j = W5100_TX+s*S_TX_SIZE;
            tx_offset = 0;
        }
        write_reg(w5100_index, j, p_data[i]);						//将p_data写入到发送缓冲区
        j++;
        tx_offset++;
    }
    //计算下一次的偏移量
    tx_offset = read_reg(w5100_index, W5100_S0_TX_WR+(s<<8));
    tx_offset *= 256;
    tx_offset += read_reg(w5100_index, W5100_S0_TX_WR+(s<<8)+1);
    
    tx_offset += count;
    write_reg(w5100_index, (W5100_S0_TX_WR+(s<<8)),(tx_offset/256));
    write_reg(w5100_index, (W5100_S0_TX_WR+(s<<8)+1),tx_offset);
    
    write_reg(w5100_index, (W5100_S0_CR+(s<<8)), S_CR_SEND);			//设置SEND命令,启动发送
    return TRUE;
}

void socket_clear_iar(uint8_t w5100_index, uint8_t port, uint8_t value)
{
    write_reg(w5100_index, W5100_S0_IR + port*0x100, value);		//回写清中断标志
}

void socket_cnn(uint8_t w5100_index,uint8_t port)
{
    socket_open_port(w5100_index, port);
}

void socket_interrupt_clear(uint8_t w5100_index, uint8_t port)
{   
    write_reg(w5100_index, W5100_IR, 0xF0);
    socket_clear_iar(w5100_index, port, 0x1F);
}

static void socket_port_process(uint8_t w5100_index, uint8_t port)
{
    uint8_t s = port;
    uint8_t state2 = 0;
    
    state2 = read_reg(w5100_index, (W5100_S0_IR + s*0x100));
    
    if (state2 & S_IR_CON)								//在TCP模式下,Socket成功连接
    {
        DBG_ASSERT(socket_int_reg[SOCKET_CON_OK_INT] != NULL __DBG_LINE);
        ( *(socket_int_reg[SOCKET_CON_OK_INT]) )(w5100_index,port);
    }
    if (state2 & S_IR_DISCON)							//在TCP服务模式下Socket断开连接处理
    {//服务端调用socket_cnn；客户端定时器调用socket_cnn直到连上服务端
        socket_server_reset(w5100_index,port);
        DBG_ASSERT(socket_int_reg[SOCKET_DISCON_OK_INT] != NULL __DBG_LINE);
        ( *(socket_int_reg[SOCKET_DISCON_OK_INT]) )(w5100_index,port);
    }
    if (state2 & S_IR_SENDOK)							//Socket数据发送完成
    {
        DBG_ASSERT(socket_int_reg[SOCKET_TX_OK_INT] != NULL __DBG_LINE);
        ( *(socket_int_reg[SOCKET_TX_OK_INT]) )(w5100_index,port);
    }
    if (state2 & S_IR_RECV)								//Socket接收到数据
    {
        IINCHIP_ISR_DISABLE();
        DBG_ASSERT(socket_int_reg[SOCKET_RX_OK_INT] != NULL __DBG_LINE);
        ( *(socket_int_reg[SOCKET_RX_OK_INT]) )(w5100_index,port);
    }
    if (state2 & S_IR_TIMEOUT)							//Socket连接或数据传输超时处理
    {
        if((w5100_dev[w5100_index].msg_mode & TCP_CLIENT_MODE) == TCP_CLIENT_MODE)
        {
            DBG_ASSERT(socket_int_reg[SOCKET_DISCON_OK_INT] != NULL __DBG_LINE);
            ( *(socket_int_reg[SOCKET_DISCON_OK_INT]) )(w5100_index,port);
        }
        else
        {
            socket_cnn(w5100_index,port);
            socket_interrupt_clear(w5100_index, port);
        }
    }
}

void socket_interrupt_process(uint8_t w5100_index)
{
    uint8_t state1 = read_reg(w5100_index, W5100_IR);		//读取中断寄存器
    
    if ((state1 & IR_CONFLICT) == IR_CONFLICT)				//IP地址冲突异常处理
    {
        _NOP();
        socket_interrupt_clear(w5100_index, PORT_0);
    }
    if ((state1 & IR_UNREACH) == IR_UNREACH)				//UDP模式下地址无法到达异常处理
    {
        socket_cnn(w5100_index, PORT_0);
        socket_interrupt_clear(w5100_index, PORT_0);
    }
    if ((state1 & IR_S0_INT) == IR_S0_INT)					//中断寄存器端口0事件
    {
        socket_port_process(w5100_index, PORT_0);
    }
    if ((state1 & IR_S1_INT) == IR_S1_INT)					//中断寄存器端口1事件
    {
        socket_port_process(w5100_index, PORT_1);
    }
    if ((state1 & IR_S2_INT) == IR_S2_INT)					//中断寄存器端口2事件
    {
        socket_port_process(w5100_index, PORT_2);
    }
    if ((state1 & IR_S3_INT) == IR_S3_INT)					//中断寄存器端口3事件
    {
        socket_port_process(w5100_index, PORT_3);
    }
}

void socket_init()
{
    W5100rest();
    spi_init();
    port_init();
	socket_int_cfg();
}

