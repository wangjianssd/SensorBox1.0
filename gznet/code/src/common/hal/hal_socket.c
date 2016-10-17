/**
 * @brief       : w5100驱动接口
 *
 * @file        : hal_socket.c
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/5/8
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/8    v0.0.1      shenghao.xu    some notes
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pbuf.h>
#include <osel_arch.h>
#include <hal_socket.h>
#include <hal_timer.h>
#include <hal_board.h>

#define HEAD                    (1u)
#define HAL_SOCKET_SBUF_NUM		(5u)
#define RESEND_NUM				(2u)
#define RESEND_TIME				(MS_TO_TICK(10000u))		//单位毫秒
#define BUFFER_LENGTH           (1500u)

typedef w5100_info_t hal_card_info_t;

static hal_card_info_t *hal_card_info;

static hal_timer_t *discon_ok_handle = NULL;


typedef struct
{
	list_head_t     list;
	pbuf_t 			*pbuf;
	uint32_t 		time;				//记录入队列的时间
    uint16_t        seq;                //移除时使用
	uint8_t
used	:1,			//使用标记
index	:2,			//标记网口
count 	:3;			//重传次数
}hal_socket_sbuf_t;

bool_t hal_socket_tcp_client(uint8_t index);
bool_t hal_socket_tcp_server(uint8_t index);
static hal_socket_sbuf_t hal_socket_sbuf[HAL_SOCKET_SBUF_NUM];
static list_head_t cache_head;
static hal_timer_t *resend_handle = NULL;


//static uint8_t eo_check(uint8_t *data_p, uint8_t len)
//{
//	uint8_t eo_result = 0;
//	uint8_t *temp_p = data_p;
//
//	eo_result = temp_p[0];
//	while (len-- != 1)
//	{
//		eo_result = eo_result ^ temp_p[1];
//		temp_p++;
//	}
//
//	return eo_result;
//}

static void tx_ok_cb(uint8_t index,uint8_t port)
{
	//发送消息
    hal_socket_msg_t msg;
    msg.card = index;
    msg.msg = SOCKET_TX_OK_INT;
    msg.port = port;
    pbuf_t *pbuf = pbuf_alloc(sizeof(hal_socket_msg_t) __PLINE1);;
    DBG_ASSERT(pbuf != NULL __DBG_LINE);  
    osel_memcpy(pbuf->data_p, (uint8_t *)&msg, sizeof(hal_socket_msg_t));
    osel_post(APP_LAN_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
}

static void rx_ok_cb(uint8_t index,uint8_t port)
{
	//接收消息
    hal_socket_msg_t msg;
    msg.card = index;
    msg.msg = SOCKET_RX_OK_INT;
    msg.port = port;
    if(hal_socket_rxfifo_cnt(&msg) != 0)
    {
        pbuf_t *pbuf = pbuf_alloc(sizeof(hal_socket_msg_t) __PLINE1);;
        DBG_ASSERT(pbuf != NULL __DBG_LINE);  
        osel_memcpy(pbuf->data_p, (uint8_t *)&msg, sizeof(hal_socket_msg_t));
        osel_post(APP_LAN_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    }
    else
    {
        IINCHIP_ISR_ENABLE();
        socket_interrupt_clear(msg.card,msg.port);
    }
}

static void con_ok_cb(uint8_t index,uint8_t port)
{
	//发送消息
    hal_socket_msg_t msg;
    msg.card = index;
    msg.msg = SOCKET_CON_OK_INT;
    msg.port = port;
    pbuf_t *pbuf = pbuf_alloc(sizeof(hal_socket_msg_t) __PLINE1);;
    DBG_ASSERT(pbuf != NULL __DBG_LINE);  
    osel_memcpy(pbuf->data_p, (uint8_t *)&msg, sizeof(hal_socket_msg_t));
    if(port == TCP_SERVICE_PORT)
    {
        if(socket_get_rxfifo_cnt(index,port) > 4)
        {
            if(hal_card_info[index].recv_state)
            {
                osel_post(APP_LAN_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
            }
            else
            {
                pbuf_free(&pbuf __PLINE2);
            }
        }
    }
    else
    {
        hal_card_info[index].cnn_state = TRUE;
        osel_post(APP_LAN_EVENT, pbuf, OSEL_EVENT_PRIO_LOW);
    }
}

static void card_cfg_init(uint8_t index,uint8_t port);
static void discon_ok_cb(uint8_t index,uint8_t port);
bool_t hal_socket_udp_client(uint8_t index);
static void discon_timer_cb(void *param)
{
    discon_ok_handle = NULL;
    osel_post(APP_LAN_EVENT, (osel_param_t*)param, OSEL_EVENT_PRIO_LOW);
}

void hal_socket_discon_deal(uint8_t index_card)
{
    if(hal_card_info[index_card].cnn_state != TRUE)
    {
        hal_socket_tcp_client(index_card);
    }
}

static void discon_ok_cb(uint8_t index,uint8_t port)
{
	//发送消息
    hal_socket_msg_t msg;
    msg.card = index;
    msg.msg = SOCKET_DISCON_OK_INT;
    msg.port = port;
    pbuf_t *pbuf = pbuf_alloc(sizeof(hal_socket_msg_t) __PLINE1);;
    DBG_ASSERT(pbuf != NULL __DBG_LINE);  
    osel_memcpy(pbuf->data_p, (uint8_t *)&msg, sizeof(hal_socket_msg_t));
    
    if((hal_card_info[index].msg_mode & TCP_CLIENT_MODE) == TCP_CLIENT_MODE)
    {
        hal_card_info[index].cnn_state = FALSE;
        if(discon_ok_handle == NULL)
        {
            HAL_TIMER_SET_REL(MS_TO_TICK((5000ul)),
                              discon_timer_cb,pbuf,discon_ok_handle);
        }
    }
    socket_interrupt_clear(index, port);
}

static void card_info_init()
{
	extern w5100_info_t w5100_dev[CARD_NUM];
	hal_card_info = &w5100_dev[0];
    device_info_t device_info = hal_board_info_look();
//	if(device_info.socket_card.ip_addr[0] == 0xff || device_info.socket_card.ip_addr[0]==0x00)
//	{
#if NODE_TYPE == NODE_TYPE_COLLECTOR		//收集器
    uint8_t temp_ip_addr[4] = {192,168,0,11};
    uint8_t temp_sub_mask[4] = {255,255,255,0};
    uint8_t temp_gateway_ip[4] = {192,168,0,1};
    uint8_t temp_port[2] = {0x13,0x88};
    uint8_t temp_dip_addr[4] = {192,168,0,10};
#else
    uint8_t temp_ip_addr[4] = {192,168,0,10};
    uint8_t temp_sub_mask[4] = {255,255,255,0};
    uint8_t temp_gateway_ip[4] = {192,168,0,1};
    uint8_t temp_port[2] = {0x13,0x88};
    uint8_t temp_dip_addr[4] = {192,168,0,11};
#endif                
    osel_memcpy(hal_card_info[1].ip_addr, temp_ip_addr, 4);
    osel_memcpy(hal_card_info[1].sub_mask, temp_sub_mask, 4);
    osel_memcpy(hal_card_info[1].gateway_ip, temp_gateway_ip, 4);
    osel_memcpy(hal_card_info[1].port, temp_port, 2);
    osel_memcpy(&hal_card_info[1].phy_addr[1], &device_info.device_id[0], 5);	//这里的内网物理地址选择硬件分配地址
    hal_card_info[1].msg_mode = 0;
    osel_memcpy(hal_card_info[1].dip_addr, temp_dip_addr, 4);
    osel_memcpy(hal_card_info[1].dport, temp_port, 2);
    
    
    osel_memcpy(device_info.socket_card.ip_addr, temp_ip_addr, 4);
    osel_memcpy(device_info.socket_card.sub_mask, temp_sub_mask, 4);
    osel_memcpy(device_info.socket_card.gateway_ip, temp_gateway_ip, 4);
    osel_memcpy(device_info.socket_card.port, temp_port, 2);
    osel_memcpy(&device_info.socket_card.phy_addr[1], &device_info.device_id[0], 5);	//这里的内网物理地址选择硬件分配地址
    device_info.socket_card.msg_mode = 0;
    osel_memcpy(device_info.socket_card.dip_addr, temp_dip_addr, 4);
    osel_memcpy(device_info.socket_card.dport, temp_port, 2);
    //hal_board_info_save(&device_info, TRUE);			//有擦坏的风险
//	}
//	else
//	{
//		osel_memcpy(hal_card_info[1].ip_addr, device_info.socket_card.ip_addr, 4);
//		osel_memcpy(hal_card_info[1].sub_mask, device_info.socket_card.sub_mask, 4);
//		osel_memcpy(hal_card_info[1].gateway_ip, device_info.socket_card.gateway_ip, 4);
//		osel_memcpy(hal_card_info[1].port, device_info.socket_card.port, 2);
//		osel_memcpy(&hal_card_info[1].phy_addr[1], &device_info.device_id[0], 5);	//这里的内网物理地址选择硬件分配地址
//		hal_card_info[1].msg_mode = 0;
//		osel_memcpy(hal_card_info[1].dip_addr, device_info.socket_card.dip_addr, 4);
//		osel_memcpy(hal_card_info[1].dport, device_info.socket_card.dport, 2);
//	}
    
	for(int i=0;i<CARD_NUM;i++)
	{
        hal_card_info[i].cnn_state = FALSE;
		hal_card_info[i].recv_state = TRUE;
	}
}

static void card_cfg_init(uint8_t index,uint8_t port)
{
	socket_conf_init(index,port);
}

static void register_cb()
{
	socket_int_reg[SOCKET_RX_OK_INT] = rx_ok_cb;
	socket_int_reg[SOCKET_TX_OK_INT] = tx_ok_cb;
	socket_int_reg[SOCKET_CON_OK_INT] = con_ok_cb;
	socket_int_reg[SOCKET_DISCON_OK_INT] = discon_ok_cb;
}

static hal_socket_sbuf_t* socket_sbuf_malloc()
{
	hal_socket_sbuf_t *sbuf = NULL;
	for(int i=0;i<HAL_SOCKET_SBUF_NUM;i++)
	{
		if(hal_socket_sbuf[i].used == FALSE)
		{
			hal_socket_sbuf[i].used = TRUE;
			sbuf = &hal_socket_sbuf[i];
			sbuf->time = hal_timer_now().w;
			return sbuf;
		}
	}
	return NULL;
}

static void socket_sbuf_free(hal_socket_sbuf_t *sbuf)
{
	sbuf->count = RESEND_NUM;
	sbuf->time = 0;
	pbuf_free(&sbuf->pbuf __PLINE2);
	sbuf->pbuf = NULL;
	sbuf->index = 0;
	sbuf->used = FALSE;
}

static void cache_init()
{//重传数组初始化
	list_init(&cache_head);
	for(int i=0;i<HAL_SOCKET_SBUF_NUM;i++)
	{
		hal_socket_sbuf[i].count = RESEND_NUM;
		hal_socket_sbuf[i].time = 0;
		hal_socket_sbuf[i].index = 0;
		hal_socket_sbuf[i].pbuf = NULL;
		hal_socket_sbuf[i].used = FALSE;
	}
}
void hal_socket_recv_enable()
{
    //    IINCHIP_ISR_ENABLE();
}

void hal_socket_recv_disenable()
{
    //    IINCHIP_ISR_DISABLE();
}

bool_t hal_socket_card_cnn_state(uint8_t index)
{
	return hal_card_info[index].cnn_state;
}

uint16_t hal_socket_rxfifo_cnt(hal_socket_msg_t *W5100)
{
	return socket_get_rxfifo_cnt(W5100->card,W5100->port);
}

void hal_socket_rxfifo_clear(uint8_t index)
{
    
}

bool_t hal_socket_tcp_server(uint8_t index)
{
    card_cfg_init(index,TCP_SERVICE_PORT);
    hal_card_info[index].msg_mode |= TCP_SERVER_MODE;
	return socket_mode(index,hal_card_info[index].msg_mode,TCP_SERVICE_PORT);
}

bool_t hal_socket_tcp_client(uint8_t index)
{
    card_cfg_init(index,PORT_0);
    hal_card_info[index].msg_mode |= TCP_CLIENT_MODE;
	return socket_mode(index,hal_card_info[index].msg_mode,PORT_0);
}

bool_t hal_socket_udp_client(uint8_t index)
{
    hal_card_info[index].cnn_state = TRUE;
    card_cfg_init(index,PORT_0);
    hal_card_info[index].msg_mode |= UDP_CLIENT_MODE;
	return socket_mode(index,hal_card_info[index].msg_mode,PORT_0);
}

static uint16_t filter_udp(hal_socket_msg_t *W5100)
{
	uint8_t udp_head[8] = {0};
	uint16_t len = 0;
	if (socket_get_rxfifo_cnt(W5100->card,W5100->port) > 8)
	{
		socket_read_fifo(&udp_head[0],8,W5100->card,W5100->port);
		len = BUILD_UINT16(udp_head[7],udp_head[6]);
	}
	return len;
}

bool_t read_head(hal_socket_msg_t *W5100)
{
	uint8_t head[2]={0};
	while (socket_get_rxfifo_cnt(W5100->card,W5100->port) > 2)
	{
		socket_read_fifo(&head[0],1,W5100->card,W5100->port);
		if (head[0] == 0xd5)
		{
			socket_read_fifo(&head[1],1,W5100->card,W5100->port);
			if (head[1] == 0xc8)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool_t read_len(hal_socket_msg_t *W5100,uint16_t *data_len)
{
	uint8_t len[2]={0};
	if (socket_get_rxfifo_cnt(W5100->card,W5100->port)>2)
	{
		socket_read_fifo(&len[0],2,W5100->card,W5100->port);
		osel_memcpy(data_len, &len[0], 2);
		//*data_len = S2B_UINT16(*data_len);	//全部使用大端模式
		if (*data_len < UART_LEN_MAX && *data_len >= 1)
		{
			return TRUE;
		}
	}
	return FALSE;
}

bool_t read_len_only(hal_socket_msg_t *W5100,uint16_t *data_len)
{
	uint8_t len[4]={0};
	if (socket_get_rxfifo_cnt(W5100->card,W5100->port)>4)
	{
		socket_read_fifo_only(&len[0],4,W5100->card,W5100->port);
		osel_memcpy(data_len, &len[2], 2);
		*data_len = S2B_UINT16(*data_len);	//全部使用大端模式
		if (*data_len < UART_LEN_MAX && *data_len >= 1)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static bool_t read_data(hal_socket_msg_t *W5100,uint8_t *buf,uint16_t *data_len)
{
    uint16_t len= 0;
    uint8_t index = W5100->card;
    len = socket_get_rxfifo_cnt(index,W5100->port);
    if(((*data_len+4) > 16) && (hal_card_info[index].msg_mode == UDP_CLIENT_MODE) && index == CARD_0)
    {//W5100upd 16个字节1个包
        socket_read_fifo(&buf[0],12,index,W5100->port);   //先读取第一包
        uint8_t bags = 0;
        bags = (*data_len - 12)/16;
        if(((*data_len - 12)%16)>0)
        {
            bags+=1;
        }
        for(int i=0; i<bags; i++)
        {
            if (!filter_udp(W5100))
            {
                return FALSE;
            }
            if(i+1 < bags)
            {
                socket_read_fifo(&buf[12+i*16],16,index,W5100->port);
            }
            else
            {
                socket_read_fifo(&buf[12+i*16],(*data_len-12),index,W5100->port);
            }
        }
    }
    else
    {
        if ( len >= *data_len)
        {
            socket_read_fifo(&buf[0],*data_len,index,W5100->port);
            return TRUE;
        }
    }
    
	return TRUE;
}

bool_t hal_tcp_service_read(hal_socket_msg_t *W5100)
{
    
	return TRUE;
}

bool_t hal_socket_read(hal_socket_msg_t *W5100)
{
	uint16_t data_len = 0;
	if (hal_card_info[W5100->card].msg_mode == UDP_CLIENT_MODE)			//判断网卡类型
	{
		//过滤UDP包
		data_len = filter_udp(W5100);
		if (data_len == 0)
		{
			return FALSE;
		}
#if HEAD == 1
        if (!read_head(W5100))
		{
			return FALSE;
		}
		if (!read_len(W5100,&data_len))
		{
			return FALSE;
		}
#endif
	}
	else
	{//TCP需要判断帧头帧长
#if HEAD == 1
		if (!read_head(W5100))
		{
			return FALSE;
		}
		if (!read_len(W5100,&data_len))
		{
			return FALSE;
		}
#else
        if (!read_len_only(W5100,&data_len))
        {
            return FALSE;
        }
#endif
	}
    
    
	pbuf_t *frm_buf = pbuf_alloc(data_len __PLINE1);
	if (frm_buf == NULL)
	{
		return FALSE;
	}
    
	if (!read_data(W5100,frm_buf->data_p,&data_len))
	{
		return FALSE;
	}
    
	if(W5100->card == CARD_0)	//内网需要验证CRC
	{
        //		if (eo_check(frm_buf->data_p,(data_len-1)) != frm_buf->data_p[data_len-1])	//校验
        //		{
        //			pbuf_free(&frm_buf __PLINE2);
        //			return FALSE;
        //		}
	}
	frm_buf->data_len = data_len;
	if(W5100->card == CARD_0)
	{
        
	}
	else if(W5100->card == CARD_1)
	{
		osel_post(APP_LAN_DATA_EVENT, frm_buf, OSEL_EVENT_PRIO_LOW);
	}
	return TRUE;
}

static void socket_send_again(hal_socket_sbuf_t *sbuf)
{
	if(sbuf->count != 0)
	{
		osel_int_status_t s;
		socket_write_fifo_send(sbuf->pbuf->head, sbuf->pbuf->data_len, sbuf->index, PORT_0);
		sbuf->count--;
		sbuf->time = hal_timer_now().w;
		OSEL_ENTER_CRITICAL(s);
		list_add_to_tail(&(sbuf->list), &cache_head);
		OSEL_EXIT_CRITICAL(s);
	}
	else
	{
		socket_sbuf_free(sbuf);
	}
}
static void socket_send_again_start();
static void socket_send_again_cb(void *p)
{
	osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	hal_socket_sbuf_t *sbuf = list_entry_decap(&cache_head, hal_socket_sbuf_t, list);
    if(sbuf != NULL)
    {
        socket_send_again(sbuf);
        OSEL_EXIT_CRITICAL(s);
        resend_handle = NULL;
        socket_send_again_start();
    }
    else
    {
        OSEL_EXIT_CRITICAL(s);
    }
}

static void socket_send_again_start()
{
	if(resend_handle == NULL)
	{
		if(!list_empty(&cache_head))
		{
			hal_socket_sbuf_t *first = NULL;
			first = (hal_socket_sbuf_t *)list_first_elem_look(&cache_head);
			uint32_t now = hal_timer_now().w;
			int32_t abs_time = now - first->time;
			if(abs_time < RESEND_TIME)
			{
				uint32_t t = RESEND_TIME - abs_time;
				HAL_TIMER_SET_REL(t,
                                  socket_send_again_cb,
                                  NULL,
                                  resend_handle);
			}
			else
			{
				socket_send_again_cb(NULL);
			}
		}
	}
}

void hal_socket_cnn(uint8_t index, uint8_t port)
{
    socket_cnn(index,port);
}

bool_t hal_socket_send(const hal_socket_buf_t *hal_socket_buf, uint8_t index)
{
	DBG_ASSERT(hal_socket_buf->buf != NULL __DBG_LINE);
	DBG_ASSERT(hal_socket_buf->length != 0 __DBG_LINE);
	if(hal_socket_card_cnn_state(index) == FALSE)
	{
		return FALSE;
	}
    
	uint8_t count = 0;
    if(!list_empty(&cache_head))
    {
        list_count(&cache_head,count);
    }
    
	if(count < HAL_SOCKET_SBUF_NUM && hal_socket_buf->ag_send == TRUE)
	{
		osel_int_status_t s;
		pbuf_t *pbuf = pbuf_alloc(hal_socket_buf->length __PLINE1);
		DBG_ASSERT(pbuf != NULL __DBG_LINE);
		osel_memcpy(pbuf->head,hal_socket_buf->buf,hal_socket_buf->length);
		pbuf->data_len = hal_socket_buf->length;
		hal_socket_sbuf_t *sbuf = socket_sbuf_malloc();
		sbuf->pbuf = pbuf;
		sbuf->index = index;
		sbuf->seq = hal_socket_buf->seq;
		OSEL_ENTER_CRITICAL(s);
		list_add_to_tail(&(sbuf->list), &cache_head);
        socket_send_again_start();
		OSEL_EXIT_CRITICAL(s);
	}
    
	return socket_write_fifo_send(&hal_socket_buf->buf[0], hal_socket_buf->length, index, PORT_0);
}

bool_t hal_socket_cache_remove(uint16_t seq)
{//从重传队列中移除
	osel_int_status_t s;
	hal_socket_sbuf_t *sbuf = NULL;
	hal_socket_sbuf_t *pos = NULL;
    if(list_empty(&cache_head))
    {
        return FALSE;
    }
    if(resend_handle!=NULL)
    {
        hal_timer_cancel(&resend_handle);
    }
	list_entry_for_each_safe(pos,sbuf,&cache_head,hal_socket_sbuf_t,list)
	{
		if(pos != NULL && pos->seq == seq)
		{
			OSEL_ENTER_CRITICAL(s);
			list_del(&(pos->list));
            socket_sbuf_free(pos);
			OSEL_EXIT_CRITICAL(s);
			socket_send_again_start();
			return TRUE;
		}
	}
    socket_send_again_start();
	return FALSE;
}

void hal_interrupt_maintenance(uint8_t index, uint8_t port)
{
    socket_interrupt_clear(index, port);
}

void hal_socket_init(void)
{
	socket_init();
    
	card_info_init();
    
	register_cb();
    
    
	if(!hal_socket_udp_client(CARD_1))
	{
		return;
	}
    
    //    if(!hal_socket_tcp_client(CARD_1))//该模式下不能开启tcp_server
    //    {
    //        return;
    //    }
    //
    //    if(!hal_socket_tcp_server(CARD_1))
    //    {
    //        return;
    //    }
    cache_init();
}
