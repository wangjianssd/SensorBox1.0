/**
 * @brief       : 
 *
 * @file        : hal_rf.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <driver.h>
#include <hal.h>

uint8_t hal_rf_get_state(void)
{
    uint8_t state = rf_get_state();

    switch(state)
    {
    case RF_IDLE_STATE:
        state = HAL_RF_IDLE_STATE;
        break;

    case RF_RX_STATE:
        state = HAL_RF_RX_STATE;
        break;

    case RF_TX_STATE:
        state = HAL_RF_TX_STATE;
        break;

    case RF_SLEEP_STATE:
        state = HAL_RF_SLEEP_STATE;
        break;

    case RF_INVALID_STATE:
        state = HAL_RF_INVALID_STATE;
        break;

    default:
        break;
    }

    return state;
}

bool_t hal_rf_set_state(uint8_t rf_state)
{
	bool_t mark = TRUE;
    switch(rf_state)
    {
    case HAL_RF_RX_STATE:
        mark = rf_enter_rx();
        break;

    case HAL_RF_TX_STATE:
        mark = rf_enter_tx();
        break;

    case HAL_RF_IDLE_STATE:
        mark = rf_enter_idle();
        break;

    case HAL_RF_SLEEP_STATE:
        mark = rf_enter_sleep();
        break;

    default:
      break;
    }

    return mark;
}

bool_t hal_rf_init(void)
{
    return rf_init();
}

void hal_rf_reset(void)
{
	rf_sw_reset();
}

bool_t hal_rf_write_fifo(uint8_t *buffer, uint8_t length)
{
    if (buffer != NULL && length != 0)
    {
        rf_write_fifo(buffer, length);
		uint8_t txfifo_cnt = rf_get_txfifo_cnt() ;
		if (txfifo_cnt == length)
		{
			return TRUE;
		}
		else
		{
			rf_flush_txfifo();
			rf_enter_sleep();
		}
    }
    return FALSE;
}

bool_t hal_rf_read_fifo(uint8_t *buffer, uint8_t buf_len)
{
	if (buffer != NULL)
	{
        rf_read_fifo(buffer, buf_len);
	    return TRUE;
	}

    return FALSE;
}

void hal_rf_flush_rxfifo(void)
{
	rf_flush_rxfifo();
}

void hal_rf_flush_txfifo(void)
{
    rf_flush_txfifo();
}

bool_t hal_rf_cfg_int(uint16_t int_type, bool_t flag)
{
    switch (int_type)
    {
    case HAL_RF_RXOK_INT:
        rf_cfg_int(RX_OK_INT, flag);
        break;

    case HAL_RF_TXOK_INT:
        rf_cfg_int(TX_OK_INT, flag);
        break;

    case HAL_RF_RXSFD_INT:
        rf_cfg_int(RX_SFD_INT, flag);
        break;

    case HAL_RF_TXSFD_INT:
        rf_cfg_int(TX_SFD_INT, flag);
        break;

    case HAL_RF_TXUND_INT:
        rf_cfg_int(TX_UND_INT, flag);
        break;

    case HAL_RF_RXOVR_INT:
        rf_cfg_int(RX_OVR_INT, flag);
        break;

    default:
        DBG_ASSERT(FALSE __DBG_LINE);
        break;
    }

    return TRUE;
}

bool_t hal_rf_reg_int(uint16_t int_type, hal_rf_cb_t cb)
{
    switch (int_type)
    {
    case HAL_RF_RXOK_INT:
        int_type = RX_OK_INT;
        break;

    case HAL_RF_TXOK_INT:
        int_type = TX_OK_INT;
        break;

    case HAL_RF_RXSFD_INT:
        int_type = RX_SFD_INT;
        break;

    case HAL_RF_TXSFD_INT:
        int_type = TX_SFD_INT;
        break;

    case HAL_RF_TXUND_INT:
        int_type = TX_UND_INT;
        break;
    }

    rf_reg_int(int_type, cb);

    return TRUE;
}


bool_t hal_rf_unreg_int(uint8_t int_type)
{
    rf_unreg_int(int_type);

    return TRUE;
}

bool_t hal_rf_set_power( uint8_t index)
{
    rf_set_power(index);
    return TRUE;
}

uint8_t hal_rf_get_power( void )
{
    return 0;
}

bool_t hal_set_power(uint8_t power_index)
{
	DBG_ASSERT(power_index <= 7 __DBG_LINE);
	rf_set_power(power_index);
    return TRUE;
}

bool_t hal_rf_set_channel(uint8_t channel_index)
{
    rf_set_channel(channel_index);
	return TRUE;
}

int16_t hal_rf_get_rssi(void)
{
    return rf_get_rssi();
}

int8_t hal_rf_get_rx_rssi(void)
{
    return rf_get_rx_rssi();
}

uint8_t hal_rf_get_rxfifo_cnt(void)
{
    return rf_get_rxfifo_cnt();
}

bool_t hal_rf_rxfifo_overflow(void)
{
    return rf_rxfifo_overflow();
}

bool_t hal_rf_txfifo_underflow(void)
{
    return rf_txfifo_underflow();
}

void hal_rf_write_reg(uint8_t addr, uint8_t value)
{
    rf_write_reg(addr, value);
}
