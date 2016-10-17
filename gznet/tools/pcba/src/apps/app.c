#include <serial.h>
#include <osel_arch.h>
#include <pbuf.h>
#include <prim.h>
#include <mac.h>
#include <driver.h>
#include <nwk_interface.h>
#include <dev.h>
#include <stdlib.h>
#include "app.h"
#include "phy_cca.h"

#define RF_RECV_FRAME_COUNT     1000
#define RF_RECV_FRAME_MAX_SEQ   16
#define DATA_LEN_MAX    64U

static uint8_t rf_rx_data[DATA_LEN_MAX] = {0xF0,0xF0};
static bool_t auto_rf_init_flag = FALSE;    //!< RF的初始化标志位
static bool_t auto_rf_send_flag = FALSE;    //!< RF的发送启动标志位
static bool_t auto_rf_recv_flag = FALSE;    //!< RF的接收到数据标志位
static bool_t auto_rf_start_recv = FALSE;   //!< RF的启动标志位
static uint32_t rf_tx_data_cnt;
static uint32_t pn9_cont = 0;
uint16_t frame_count = 0;//统计帧数
uint16_t maxseq = 0;//最大真序号
uint16_t receivenum = 0;		// RF接收到的帧数
uint16_t errorrate = 0;           //丢帧数
uint8_t LastSN = 0;            //每次发送接收到得最后1个数据
bool_t  frame_cal_flag = FALSE;
static int8_t rssi_dbm = 0;
uint8_t recv_buf[20];
static uint8_t frm_len = 0;
    
DBG_THIS_MODULE("app")

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

static uint8_t rf_channel = 7;
static uint8_t rf_power = 0x11;

static bool_t enter_lpm = FALSE;

PROCESS_NAME(app_serial_process);
PROCESS_NAME(app_rf_send_process);
PROCESS_NAME(app_rf_recv_process);

static void nfc_read_write_test(void)
{
    uint8_t write_buf[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    
    uint8_t read_buf[16];
    //*< nfc读写范围从0x0000~0x1000循环、
    if(!NFC_DEVICE.send(NFC_ADDRESS_E2_0 , write_buf, 16, 0x0100))
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "nfc write failed\r\n");
    }
    if(!NFC_DEVICE.recv(NFC_ADDRESS_E2_0 , read_buf, 16, 0x0100))
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "nfc read failed\r\n");
    }
    
    if(!osel_memcmp(read_buf, write_buf, 16))
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "nfc write/read cmp failed\r\n");
    }
    else
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "nfc write/read cmp ok\r\n");
    }
}


static void app_cmd_rf_channel(uint8_t *buf, uint8_t len)
{
    uint8_t *datap = buf;
    uint8_t ch_len = 0;
    
    if(*datap == '=')
    {
        datap += 1;
        
        rf_channel = atoi((char *)datap);
        
        DBG_LOG(DBG_LEVEL_ORIGIN, "RF CHANNEL:%d\r\n", rf_channel);
    }
    else
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_ERROR02);
    }
}

static void app_cmd_rf_power(uint8_t *buf, uint8_t len)
{
    uint8_t *datap = buf;
    uint8_t ch_len = 0;
    
    if(*datap == '=')
    {
        datap += 1;
        
        uint8_t power = atoi((char *)datap);
        if(power < 18)
        {
            rf_power = power;
            DBG_LOG(DBG_LEVEL_ORIGIN, "RF POWER:%d\r\n", rf_power);
        }
        else
        {
            DBG_LOG(DBG_LEVEL_ORIGIN, CMD_ERROR03);
        }        
    }
    else
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_ERROR02);
    }
}

static void app_cmd_exter_parse(uint8_t *buf, uint8_t len)
{
    if(osel_memcmp(buf, CMD_EXTER_NFC_ID, sizeof(CMD_EXTER_NFC_ID)-1))
    {
        uint32_t nfc_id = 0;
        NFC_DEVICE.get_value(NFC_ID, &nfc_id);
        uint32_t high = nfc_id;
        DBG_LOG(DBG_LEVEL_ORIGIN, "NFCID:0x%x", (high>>16));
        DBG_LOG(DBG_LEVEL_ORIGIN, "%x\r\n", nfc_id);
    }
    else if(osel_memcmp(buf, CMD_EXTER_NFC_RD, sizeof(CMD_EXTER_NFC_RD)-1))
    {
        nfc_read_write_test();
    }
    else if(osel_memcmp(buf, CMD_EXTER_RF_TX_ST, sizeof(CMD_EXTER_RF_TX_ST)-1))
    {
        osel_pthread_create(app_task_handle, &app_rf_send_process, NULL);
    }
    else if(osel_memcmp(buf, CMD_EXTER_RF_TX_ED, sizeof(CMD_EXTER_RF_TX_ED)-1))
    {
        osel_pthread_exit(app_task_handle, &app_rf_send_process, PROCESS_CURRENT());
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_OK);
    }
    
    else if(osel_memcmp(buf, CMD_EXTER_RF_RX_ST, sizeof(CMD_EXTER_RF_RX_ST)-1))
    {
        osel_pthread_create(app_task_handle, &app_rf_recv_process, NULL);
    }
    else if(osel_memcmp(buf, CMD_EXTER_RF_RX_ED, sizeof(CMD_EXTER_RF_RX_ED)-1))
    {
        osel_pthread_exit(app_task_handle, &app_rf_recv_process, PROCESS_CURRENT());
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_OK);
    }
    else if(osel_memcmp(buf, CMD_EXTER_RF_CH, sizeof(CMD_EXTER_RF_CH)-1))
    {
        app_cmd_rf_channel(buf + sizeof(CMD_EXTER_RF_CH) - 1,
                           len - (sizeof(CMD_EXTER_RF_CH) - 1));
    }
    else if(osel_memcmp(buf, CMD_EXTER_RF_PR, sizeof(CMD_EXTER_RF_PR)-1))
    {
        app_cmd_rf_power(buf + sizeof(CMD_EXTER_RF_PR) - 1,
                         len - (sizeof(CMD_EXTER_RF_PR) - 1));
    }
    else if(osel_memcmp(buf, CMD_EXTER_BAT, sizeof(CMD_EXTER_BAT)-1))
    {
        uint16_t energy = energy_get();
        DBG_LOG(DBG_LEVEL_ORIGIN, "ENERGY: %04d\r\n", energy);
    }
    else if(osel_memcmp(buf, CMD_EXTER_SLEEP, sizeof(CMD_EXTER_SLEEP)-1))
    {
        m_tran_sleep();
        LED_CLOSE(GREEN);
        enter_lpm = TRUE;
        DBG_LOG(DBG_LEVEL_ORIGIN, "OK");
    }
    else
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_ERROR01);
    }
}

static void app_cmd_parse(const uint8_t *buf, uint8_t len)
{
    switch(*buf)
    {
    case CMD_TEST_TYPE:
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_OK);
        break;
        
    case CMD_EXTER_TYPE:
        app_cmd_exter_parse((uint8_t *)(buf+1), len-1);
        break;
        
    default:
        DBG_LOG(DBG_LEVEL_ORIGIN, CMD_ERROR00);
        break;
    }
}

static void serial_event_handle(void)
{
	uint8_t frm_len = 0;
    uint8_t read_data = 0;
    uint8_t cmd_recv[UART_LEN_MAX];
    
    osel_memset(cmd_recv, 0x00, UART_LEN_MAX);
    
    while(serial_read(SERIAL_1, &read_data, sizeof(uint8_t)))
    {
        cmd_recv[frm_len++] = read_data;
        if(read_data == CMD_CR)
        {
            break;
        }
    }
    
    if(frm_len > CMD_AT_HEAD_SIZE)
    {
        app_cmd_parse(&cmd_recv[CMD_AT_HEAD_SIZE], 
                      frm_len - CMD_AT_HEAD_SIZE);
    }
}

static void uart_event(void)
{
    osel_event_t event;
    event.sig = APP_SERIAL_EVENT;
    event.param = NULL;
    osel_post(NULL, &app_serial_process, &event);
}

static void uart_frame_cfg(void)
{
	serial_reg_t serial;
    
	serial.sd.valid = TRUE;
	serial.sd.len = 2;
	serial.sd.pos = 0;
	serial.sd.data[0] = 'A';
	serial.sd.data[1] = 'T';
    
	serial.ld.valid = FALSE;
	serial.ld.little_endian = TRUE;
    
	serial.argu.len_max = UART_LEN_MAX;
	serial.argu.len_min = 2;
    
	serial.ed.valid = TRUE;
    serial.ed.len = 1;
    serial.ed.data[0] = CMD_CR;
    
	serial.echo_en = FALSE;
	serial.func_ptr = uart_event;
	serial_fsm_init(SERIAL_1);
	serial_reg(SERIAL_1, serial);
}

static void rx_ok_cb(uint16_t time)
{
    osel_event_t event;
    event.sig = APP_RF_RXOK_EVENT;
    event.param = NULL;
    osel_post(NULL, &app_rf_recv_process, &event);
}

static void tx_ok_cb(uint16_t time)
{
    osel_event_t event;
    event.sig = APP_RF_TXOK_EVENT;
    event.param = NULL;
    osel_post(NULL, &app_rf_send_process, &event);
}

PROCESS(app_serial_process, "app_serial_process");
PROCESS_THREAD(app_serial_process, ev, data)
{
    PROCESS_BEGIN();
    
    while(1)
    {   
        if(ev == APP_SERIAL_EVENT)
        {
            serial_event_handle();
        }
        
        PROCESS_YIELD();
    }
    
    PROCESS_END();
}

PROCESS(app_rf_send_process, "app_rf_send_process");
PROCESS_THREAD(app_rf_send_process, ev, data)
{
    PROCESS_BEGIN();
    static uint8_t daddr[] = {0xAA, 0x55};
    static uint8_t txarray[] = {16, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
    static osel_etimer_t cycl_etimer;
    
    while(1)
    {   
        OSEL_ETIMER_DELAY(&cycl_etimer, 1);
        
        PROCESS_EXITHANDLER(txarray[1]= 0);
        phy_set_channel(rf_channel);
        phy_set_power(rf_power);
        SSN_RADIO.set_value(RF_TXFIFO_FLUSH, 0);
        
        SSN_RADIO.set_value(RF_DADDR0, daddr[0]);
        SSN_RADIO.set_value(RF_DADDR1, daddr[1]);
    
        SSN_RADIO.set_value(RF_TXFIFO_CNT, sizeof(txarray));
        SSN_RADIO.prepare(txarray, sizeof(txarray));  
        SSN_RADIO.transmit(0);
        
        PROCESS_WAIT_UNTIL(ev == APP_RF_TXOK_EVENT);
        DBG_LOG(DBG_LEVEL_ORIGIN, "TX SEQ: %d\r\n", txarray[1]);
        txarray[1]++;
        
//        OSEL_ETIMER_DELAY(&cycl_etimer, 1);
        PROCESS_PAUSE();
    }
    
    PROCESS_END();
}

PROCESS(app_rf_recv_process, "app_rf_recv_process");
PROCESS_THREAD(app_rf_recv_process, ev, data)
{    
    PROCESS_BEGIN();
    
    static uint8_t laddr[] = {0xAA, 0x55};
//    SSN_RADIO.set_value(RF_LADDR0, laddr[0]);
//    SSN_RADIO.set_value(RF_LADDR1, laddr[1]);
    
    auto_rf_start_recv = TRUE;
    
    frame_count = RF_RECV_FRAME_COUNT;
    maxseq = RF_RECV_FRAME_MAX_SEQ;
    DBG_LOG(DBG_LEVEL_ORIGIN, " Start calulate data(frame num %d,max seq %d) ... \r\n",
            frame_count,maxseq);
    receivenum = 0;
    errorrate = 0; 

    while(1)
    {
        PROCESS_EXITHANDLER(rssi_dbm = 0);
        SSN_RADIO.set_value(RF_RXFIFO_FLUSH, 0);
        phy_set_channel(rf_channel);       
        phy_set_state(PHY_RX_STATE);
        
        PROCESS_WAIT_UNTIL(ev == APP_RF_RXOK_EVENT);
        SSN_RADIO.get_value(RF_RXRSSI, &rssi_dbm);
        auto_rf_recv_flag = FALSE;
        SSN_RADIO.get_value(RF_RXFIFO_CNT, &frm_len);
        
        if (frm_len <= DATA_LEN_MAX)
        {
            SSN_RADIO.recv(&rf_rx_data[0], frm_len);
        }
        else
        {
            SSN_RADIO.set_value(RF_RXFIFO_FLUSH, 0);                
        }
        
        if (frm_len < DATA_LEN_MAX)
        {
            if(receivenum == 0)
            {
                errorrate = 0;
                receivenum = 1;
            }
            else
            {
                if (rf_rx_data[17] != ((LastSN+1)%maxseq))
                {
                    if (rf_rx_data[17] > LastSN)
                    {
                        errorrate = errorrate + rf_rx_data[17] - LastSN;
                    }
                    else
                    {
                        errorrate = maxseq + errorrate + rf_rx_data[17] - LastSN;
                    }
                }
                
                receivenum ++;
                DBG_LOG(DBG_LEVEL_ORIGIN, "Rx:%d Er:%d\r\n", receivenum, errorrate);
                if ((receivenum + errorrate) >= frame_count)
                {
                    DBG_LOG(DBG_LEVEL_ORIGIN, "Caculate finished\r\n");
                    DBG_LOG(DBG_LEVEL_ORIGIN, " Calulate %d frame,received %d frame \r\n",frame_count,receivenum);
                    break;
                }                       
            }
            LastSN = rf_rx_data[17];                               
        }
        
        phy_set_state(PHY_RX_STATE);
        PROCESS_PAUSE();
    }
    PROCESS_END();
}


static void sys_enter_lpm_handler(void *p)
{
    debug_info_printf();
    if(enter_lpm)
    {
        LPM3;
    }
}

void app_init(void)
{
    app_task_handle = osel_task_create(NULL, APP_TASK_PRIO, app_event_store, APP_EVENT_MAX);
    
    osel_pthread_create(app_task_handle, &app_serial_process, NULL);
    
    uart_frame_cfg();
    osel_idle_hook(sys_enter_lpm_handler);
    
    if(SSN_RADIO.init() != TRUE)
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "radio init failed\r\n");
        return;
    }
    
    SSN_RADIO.int_cfg(RX_OK_INT, rx_ok_cb, INT_ENABLE);
    SSN_RADIO.int_cfg(TX_OK_INT, tx_ok_cb, INT_ENABLE);

    if(NFC_DEVICE.init() != TRUE)
    {
        DBG_LOG(DBG_LEVEL_ORIGIN, "nfc init failed\r\n");
        return;
    }
    
    DBG_LOG(DBG_LEVEL_ORIGIN, CMD_SYSSTART);
    m_tran_stop();
    
//    osel_pthread_create(app_task_handle, &app_rf_recv_process, NULL);
//    osel_pthread_create(app_task_handle, &app_rf_send_process, NULL);
}



