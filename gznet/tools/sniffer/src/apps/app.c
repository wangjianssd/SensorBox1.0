#include <serial.h>
#include <osel_arch.h>
#include <hal_uart.h>
#include <hal_board.h>
#include <uart.h>
#include <pbuf.h>
#include <prim.h>
#include <mac.h>
#include <driver.h>
#include <nwk_interface.h>
#include <hal.h>

#include "app.h"
enum
{
	N_SYNC = 1,			//超帧配置
	N_ASYN = 2,
	N_REST = 3,
	N_DATA = 5,
}msg_type_e;


#define APP_EVENT_MAX       (10u)   //*< 最多处理10个事件
static osel_event_t app_event_store[APP_EVENT_MAX];
static osel_task_t *app_task_handle = NULL;

#define PRINTF_EVENT_MAX    (10u)
static osel_event_t printf_event_store[PRINTF_EVENT_MAX];
static osel_task_t *printf_task_handle = NULL;

PROCESS_NAME(app_process);

static void uart_frame_cb()
{
	osel_event_t event;
    event.sig = APP_SERIAL_DATA_EVENT;
    event.param = NULL;
    osel_post(NULL, &app_process, &event);
}

static void uart_frame_cfg(void)
{	
	serial_reg_t serial;
	serial.sd.valid = TRUE;
	serial.sd.len = 2;
	serial.sd.pos = 0;
	serial.sd.data[0] = 0xd5;
	serial.sd.data[1] = 0xc8;
	serial.ld.valid = TRUE;
	serial.ld.little_endian = TRUE;
	serial.argu.len_max = UART_LEN_MAX;
	serial.argu.len_min = 1;
	serial.ed.valid = FALSE;
	serial.echo_en = FALSE;
	serial.func_ptr = uart_frame_cb;
	serial_fsm_init(SERIAL_1);
	serial_reg(SERIAL_1, serial);
}


PROCESS_NAME(printf_sniffer_process);

/**< 传输模块回调函数 */
static pbuf_t *mac_frame_get(void)
{
    pbuf_t *frame = NULL;
    frame = phy_get_packet();
    
    if(frame != NULL)
    {
        hal_led_toggle(GREEN);
        osel_event_t event;
        event.sig = PRINTF_SNFFIER_EVENT;
        event.param = frame;
        osel_post(NULL, &printf_sniffer_process, &event);
    }
    
    phy_set_state(PHY_RX_STATE);
    return frame;
}

static bool_t mac_frame_head_parse(pbuf_t *pbuf)
{
    pbuf->attri.need_ack    = FALSE;
    return TRUE;
}

static bool_t mac_frame_parse(pbuf_t *pbuf)
{
    
    return TRUE;
}

static void mac_tx_finish(sbuf_t *const sbuf, bool_t result)
{
}

static void ack_tx_ok_callback(sbuf_t *sbuf, bool_t res)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
    sbuf_free(&sbuf __SLINE2);
}

static void mac_send_ack(uint8_t seqno)
{
	return;
}

PROCESS(printf_sniffer_process, "printf_sniffer_process");

PROCESS_THREAD(printf_sniffer_process, ev, data)
{
    PROCESS_BEGIN();
    static pbuf_t *pbuf = NULL;
    while(1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PRINTF_SNFFIER_EVENT);
        pbuf = (pbuf_t *)data;
        hal_uart_send_string(HAL_UART_1, (uint8_t *)pbuf->head, pbuf->data_len);
        pbuf_free(&pbuf __PLINE2);
//        PROCESS_YIELD();
    }
    
    PROCESS_END();
}

static void uart_recv_handler(void)
{
    uint8_t uart_recv_buf[6] = {0};

    serial_read(SERIAL_1, uart_recv_buf, 5);
    if(uart_recv_buf[0]==0xd5 && uart_recv_buf[1]==0xc8)
    {
			uint8_t s = 0;
            OSEL_ENTER_CRITICAL(s);
			m_tran_stop();
    		device_info.device_ch[0] = uart_recv_buf[4];
            hal_nvmem_erase((uint8_t *)DEVICE_INFO_ADDR, 0);
            hal_nvmem_write((uint8_t *)DEVICE_INFO_ADDR,
                            (uint8_t *)&device_info,
                            sizeof(device_info_t));
            PRINTF("\r\nch:%d\r\n",device_info.device_ch);
			OSEL_EXIT_CRITICAL(s);
            hal_board_reset();
    }
}

PROCESS(app_process, "app_process");
PROCESS_THREAD(app_process, ev, data)
{
	PROCESS_BEGIN();
	
	while(1)
	{   
		if(ev == APP_SERIAL_DATA_EVENT)
		{
			uart_recv_handler();
		}
		PROCESS_YIELD();
	}
	
	PROCESS_END();
}


void app_init(void)
{
    app_task_handle = osel_task_create(NULL, APP_TASK_PRIO, app_event_store, APP_EVENT_MAX);
    osel_pthread_create(app_task_handle, &app_process, NULL);
	
    printf_task_handle = osel_task_create(NULL, PRINTF_TASK_PRIO, printf_event_store, PRINTF_EVENT_MAX);
    osel_pthread_create(printf_task_handle, &printf_sniffer_process, NULL);
    
    m_tran_init(app_task_handle);
    
    tran_cfg_t mac_tran_cb;
    mac_tran_cb.frm_get             = mac_frame_get;
    mac_tran_cb.frm_head_parse      = mac_frame_head_parse;
    mac_tran_cb.frm_payload_parse   = mac_frame_parse;
    mac_tran_cb.tx_finish           = mac_tx_finish;
    mac_tran_cb.send_ack            = mac_send_ack;
    m_tran_cfg(&mac_tran_cb);
    
//    osel_idle_hook(sniffer_printf);
    
    phy_set_channel(device_info.device_ch[0]);
    m_tran_recv();
	uart_frame_cfg();
}