/**
 * @brief       :
 *
 * @file        : radio.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/10/19
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/19    v0.0.1      gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"
#include "platform/platform.h"

#include "../radio_defs.h"

#include "radio.h"
#include "radio_config.h"

#include "si446x_api_lib.h"
#include "si446x_cmd.h"

DBG_THIS_MODULE("radio")

const SEGMENT_VARIABLE(Radio_Configuration_Data_Array[], uint8_t, SEG_CODE) = \
        RADIO_CONFIGURATION_DATA_ARRAY;


const SEGMENT_VARIABLE(RadioConfiguration, tRadioConfiguration, SEG_CODE) =  RADIO_CONFIGURATION_DATA;

const SEGMENT_VARIABLE_SEGMENT_POINTER(pRadioConfiguration, tRadioConfiguration, SEG_CODE, SEG_CODE) = \
        &RadioConfiguration;

static volatile rf_state_t rf_current_state = RF_INVALID_STATE;
static volatile uint8_t rf_channel = 0;
static volatile uint8_t rf_power = 0;
static volatile uint8_t rf_send_len = 0;
static volatile uint8_t rssi_latch = 0;
static rf_int_reg_t rf_int_reg[RF_INT_MAX_NUM];


static const uint8_t table[] =
{
    0x11, 0x08, 0x2B, 0x00,
    0x10, 0x08, 0x25, 0x00,
    0x0F, 0x08, 0x21, 0x00,
    0x0E, 0x08, 0x1D, 0x00,
    0x0D, 0x08, 0x19, 0x00,
    0x0C, 0x08, 0x16, 0x00,
    0x0B, 0x08, 0x14, 0x00,
    0x0A, 0x08, 0x12, 0x00,
    0x09, 0x08, 0x10, 0x00,
    0x08, 0x08, 0x0E, 0x00,
    0x07, 0x08, 0x14, 0xC0,
    0x06, 0x08, 0x11, 0xC0,
    0x05, 0x09, 0x25, 0x25,
    0x04, 0x09, 0x23, 0x23,
    0x03, 0x09, 0x21, 0x21,
    0x02, 0x09, 0x1F, 0x1F,
    0x01, 0x09, 0x1D, 0x1D,
    0x00, 0x09, 0x1C, 0x1B,
};

rf_result_t rf_get_value(rf_cmd_t cmd, void *value);
rf_result_t rf_set_value(rf_cmd_t cmd, uint8_t value);

/*!
 *  Set Radio to RX mode, fixed packet length.
 *
 *  @param channel Freq. Channel
 *
 *  @note
 *
 */
void radio_recv_start(uint8_t channel)
{
    // Read ITs, clear pending ones
    si446x_get_int_status(0u, 0u, 0u);

    /* Start Receiving packet, channel 0, START immediately, Packet n bytes long */
    si446x_start_rx(channel, 0u, 0,
                    SI446X_CMD_START_RX_ARG_NEXT_STATE1_RXTIMEOUT_STATE_ENUM_NOCHANGE,
                    SI446X_CMD_START_RX_ARG_NEXT_STATE2_RXVALID_STATE_ENUM_READY,
                    SI446X_CMD_START_RX_ARG_NEXT_STATE3_RXINVALID_STATE_ENUM_RX );
}

#if 0
static void radio_set_freq(uint32_t freq)
{
    ;
}
#endif

static void radio_set_state(rf_state_t state)
{
    uint8_t set = SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_NOCHANGE;
    
    
    switch (state)
    {
    case RF_IDLE_STATE:
//        radio_4259_idle();
//        radio_spi_init();
//        radio_nirq_enable();
        
        set = SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_READY;
        si446x_change_state(set);
        break;

    case RF_RX_STATE:
        if(rf_current_state == RF_SLEEP_STATE)
        {
//            radio_4259_idle();
//            radio_spi_init();
//            radio_nirq_enable();
        }
        radio_4259_rx();
        si446x_set_property(0x12, 0x02, 0x0d, 0x00, 0x02);
        si446x_set_property(0x12, 0x02, 0x11, 0x00, 0x01);
        si446x_set_property(0x12, 0x02, 0x15, 0x00, 0x3D);
        set = SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_RX;
        radio_recv_start(rf_channel);
        break;

    case RF_TX_STATE:
        if(rf_current_state == RF_SLEEP_STATE)
        {
//            radio_4259_idle();
//            radio_spi_init();
//            radio_nirq_enable();
        }
        radio_4259_tx();
        set = SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_TX;
        si446x_change_state(set);
        break;

    case RF_SLEEP_STATE:
        set = SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_SLEEP;
        si446x_get_int_status(0, 0, 0);
        si446x_change_state(set);
        
//        radio_nirq_disable();
//        radio_4259_idle();
//        radio_spi_deinit();
        break;

    default:
        break;
    }
    
    rf_current_state = state;
}

static rf_result_t radio_set_power(uint8_t args)
{
    uint8_t *p = (uint8_t *)table;

    if (args > 0x12)
    {
        return RF_RESULT_ERROR;
    }

    rf_power = args;

    while (args != *p++);

    si446x_set_property(SI446X_CMD_ID_GET_MODEM_STATUS,
                        0x03, 0x00, p[0], p[1], p[2]);

    return RF_RESULT_OK;
}

static void radio_powerup(void)
{
    /* Power Up the radio chip */
    SEGMENT_VARIABLE(delay,  uint16_t, SEG_XDATA) = 0u;
    /* Hardware reset the chip */
    si446x_reset();

    /* Wait until reset timeout or Reset IT signal */
    for (; delay < pRadioConfiguration->Radio_Delay_Cnt_After_Reset; delay++);
}


bool_t rf_init(void)
{
    uint16_t delay = 0;
    
    radio_spi_init();
    
    radio_port_init();
    
    radio_unb_on();

    radio_powerup();


    si446x_part_info();//for test
//    DBG_LOG(DBG_LEVEL_INFO, "radio is 0x%x\r\n", Si446xCmd.PART_INFO.PART);
    
    /* Load radio configuration */
    while (SI446X_SUCCESS != si446x_configuration_init(pRadioConfiguration->Radio_ConfigurationArray))
    {
        /* Error hook */
        for (delay = 0x7FFF; delay--; ) ;
        /* Power Up the radio chip */
        radio_powerup();
    }

    rf_set_value(RF_POWER, 0x11);
    rf_set_value(RF_CHANNEL, CH_SN);
    rf_set_value(RF_STATE, RF_IDLE_STATE);
    /*Read ITs, clear pending ones*/
    si446x_get_int_status(0u, 0u, 0u);
    
    radio_nirq_init();

    radio_nirq_enable();

    return TRUE;
}


int8_t rf_prepare(uint8_t const *payload, uint8_t len)
{
    radio_4259_tx();
    //*< 设置第三块区域为MAC域，
    si446x_set_property(0x12, 0x02, 0x15, 0x00, len);
    si446x_write_tx_fifo(len, (uint8_t *)payload);
    return len;
}

int8_t rf_transmit(uint8_t len)
{
    si446x_start_tx(rf_channel, (0x03 << 4), 0); //*< set to ready when tx ok
    return len;
}

int8_t rf_send(uint8_t const *payload, uint8_t len)
{
    rf_prepare(payload, len);
    // Read ITs, clear pending ones
    si446x_get_int_status(0u, 0u, 0u);
    si446x_start_tx(rf_channel, (0x03 << 4), len); //*< set to ready when tx ok
    return len;
}

int8_t rf_recv(uint8_t *const buf, uint8_t len)
{
    si446x_read_rx_fifo(len, buf);

    return len;
}

rf_result_t rf_get_value(rf_cmd_t cmd, void *value)
{
    uint8_t *buf = value;
    
    switch (cmd)
    {
    case RF_STATE:
        *(uint8_t *)value = rf_current_state;
        return RF_RESULT_OK;

    case RF_POWER:
        *(uint8_t *)value = rf_power;
        return RF_RESULT_OK;

    case RF_CHANNEL:
        si446x_request_device_state();
        *(uint8_t *)value = Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL;
        return RF_RESULT_OK;

    case RF_DATARATE:
        si446x_set_property(0x20, 3, 0x03, buf[2], buf[1], buf[0]);
        return RF_RESULT_OK;

    case RF_RXRSSI:
        si446x_get_modem_status(0xff);
        rssi_latch = Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI;
        *(int8_t *)value = (rssi_latch/2)-0x40-70;
        return RF_RESULT_OK;

    case RF_CCA:
        si446x_get_modem_status(0xff);
        *(int8_t *)value = (Si446xCmd.GET_MODEM_STATUS.CURR_RSSI/2)-0x40-70;
        return RF_RESULT_OK;

    case RF_RXFIFO_CNT:
        si446x_fifo_info(0);
        uint8_t temp[3];
        si446x_read_rx_fifo(2, temp);
        *(uint8_t *)value = Si446xCmd.FIFO_INFO.RX_FIFO_COUNT - 2;
        return RF_RESULT_OK;

    case RF_TXFIFO_CNT:
        si446x_fifo_info(0);
        *(uint8_t *)value = 64 - Si446xCmd.FIFO_INFO.TX_FIFO_SPACE;
        return RF_RESULT_OK;

    case RF_RXFIFO_OVERFLOW:
        //@todo:
        //
        break;

    case RF_TXFIFO_UNDERFLOW:
        //@todo:
        //
        break;


    default:
        break;
    }

    return RF_RESULT_NOT_SUPPORTED;
}

rf_result_t rf_set_value(rf_cmd_t cmd, uint8_t value)
{
    switch (cmd)
    {
    case RF_STATE:
        radio_set_state((rf_state_t)value);
        return RF_RESULT_OK;

    case RF_POWER:
        return radio_set_power(value);

    case RF_CHANNEL:
        rf_channel = value;
        break;

    case RF_DATARATE:
        //@todo : ADD CODE
        break;

    case RF_LADDR0:
        //*< MATCH 1 OR 2 and 3 or 4
        si446x_set_property(0x30, 0x03, 0x00,   //*< property, addr, len
                            value, 0xFF, 0x40); //*< match1 val, mask, ctl
        si446x_set_property(0x30, 0x03, 0x03,   //*< property, addr, len
                            0xFF, 0xFF, 0x40); //*< match1 val, mask, ctl
        break;

    case RF_LADDR1:
        //*< MATCH 3 OR 4
        si446x_set_property(0x30, 0x03, 0x06,   //*< property, addr, len
                            value, 0xFF, 0x01); //*< match1 val, mask, ctl
        si446x_set_property(0x30, 0x03, 0x09,   //*< property, addr, len
                            0xFF, 0xFF, 0x41); //*< match1 val, mask, ctl
        break;
        
    case RF_DADDR0:
        si446x_set_property(0x12, 0x02, 0x0d, 0x00, 0x01);
        si446x_write_tx_fifo(1, (uint8_t *)&value);
        break;
        
    case RF_DADDR1:
        si446x_set_property(0x12, 0x02, 0x0d, 0x00, 0x02);
        si446x_write_tx_fifo(1, (uint8_t *)&value);
        break;

    case RF_RXFIFO_CNT:
        break;

    case RF_RXFIFO_FLUSH:
        si446x_fifo_info(0x02);
        return RF_RESULT_OK;

    case RF_TXFIFO_CNT:
        //*< 设置第一块区域为长度域，长度为1
        rf_send_len = value;
        si446x_set_property(0x12, 0x02, 0x11, 0x00, 0x01);
        si446x_write_tx_fifo(1, (uint8_t *)&rf_send_len);
        return RF_RESULT_OK;

    case RF_TXFIFO_FLUSH:
        si446x_fifo_info(0x01);
        return RF_RESULT_OK;

    default:
        break;
    }

    return RF_RESULT_NOT_SUPPORTED;
}

static bool_t rf_int_cfg(rf_int_t state, rf_int_reg_t int_cb, uint8_t type)
{
    (type == INT_ENABLE) ? (radio_nirq_enable()) : (radio_nirq_disable());
    (type == INT_ENABLE) ? ((int_cb != NULL) ? (rf_int_reg[state] = int_cb) : (NULL)) : (NULL);

    return TRUE;
}

const struct radio_driver si446x_driver =
{
    rf_init,

    rf_prepare,
    rf_transmit,

    rf_send,
    rf_recv,

    rf_get_value,
    rf_set_value,

    rf_int_cfg,
};

void rf_int_handler(uint16_t time)
{
    si446x_get_int_status(0, 0, 0);

    if (Si446xCmd.GET_INT_STATUS.PH_PEND &
            SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_CRC_ERROR_PEND_BIT)
    {
        si446x_fifo_info(2);
        radio_recv_start(rf_channel);
        return;
    }
    
//    if(Si446xCmd.GET_INT_STATUS.PH_PEND &
//       SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_FILTER_MATCH_PEND_BIT)
//    {
//        _NOP();
//    }
//    
//    if(Si446xCmd.GET_INT_STATUS.PH_PEND &
//       SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_FILTER_MISS_PEND_BIT)
//    {
//        _NOP();
//    }

    if (Si446xCmd.GET_INT_STATUS.PH_PEND &
            SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
    {
        if (rf_current_state == RF_RX_STATE)
        {
            rf_current_state = RF_RXOK_STATE;

            if (rf_int_reg[RX_OK_INT] != NULL)
            {
                ( *(rf_int_reg[RX_OK_INT]) )(time);
            }
        }
    }

    if (Si446xCmd.GET_INT_STATUS.PH_PEND &
            SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)
    {
        rf_current_state = RF_TXOK_STATE;
        if ( rf_int_reg[TX_OK_INT] != NULL )
        {
            ( *(rf_int_reg[TX_OK_INT]) )(0);
        }
    }

    if (Si446xCmd.GET_INT_STATUS.MODEM_PEND &
            SI446X_CMD_GET_INT_STATUS_REP_MODEM_PEND_SYNC_DETECT_PEND_BIT)
    {
        si446x_get_modem_status(0xff);
        rssi_latch = Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI;
        if (rf_int_reg[RX_SFD_INT] != NULL)
        {
            ( *(rf_int_reg[RX_SFD_INT]) )( time );
        }
    }
}




