/**
 * @brief       : 
 *
 * @file        : radio.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/11/3
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/11/3    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"

#include "../radio_defs.h"

#include "radio.h"

DBG_THIS_MODULE("radio")

#define	 TIMEOUT(i)		                            \
    i = 0;                                          \
    do                                              \
    {                                               \
        if((i)++ > 70000)                           \
        {                                           \
            DBG_ASSERT(FALSE __DBG_LINE);           \
        }                                           \
    } while(__LINE__ == -1)
        
       
#define RF_SET_REG_BIT(addr, bit)                   \
    do                                              \
    {                                               \
        uint8_t reg_value = rf_read_reg((addr));    \
        rf_write_reg((addr), (reg_value|(bit)));    \
    } while(__LINE__ == -1)


#define RF_CLR_REG_BIT(addr, bit)                   \
    do                                              \
    {                                               \
        uint8_t reg_value = rf_read_reg((addr));    \
        rf_write_reg((addr), (reg_value&(~bit)));   \
    } while(__LINE__ == -1)

#define RF_CLEAR_SFD_FLAG()                 MCU_RADIO_INT_CLR()
#define RF_CLEAR_RXOK_FLAG()                MCU_RADIO_INT_CLR()
#define RF_CLEAR_TXOK_FLAG()                MCU_RADIO_INT_CLR()
#define RF_CLEAR_TXUND_FLAG()               MCU_RADIO_INT_CLR()
#define RF_CLEAR_RXOVR_FLAG()               MCU_RADIO_INT_CLR()

#define RF_SFD_INT_ENABLE()             RF_SET_REG_BIT(INTERRUPT_ENABLE2, BIT7)
#define RF_RXOK_INT_ENABLE()            RF_SET_REG_BIT(INTERRUPT_ENABLE1, BIT1)
#define RF_TXOK_INT_ENABLE()            RF_SET_REG_BIT(INTERRUPT_ENABLE1, BIT2)
#define RF_TXUND_INT_ENABLE()           RF_SET_REG_BIT(INTERRUPT_ENABLE1, BIT7)
#define RF_RXOVR_INT_ENABLE()           RF_SET_REG_BIT(INTERRUPT_ENABLE1, BIT7)

#define RF_SFD_INT_DISABLE()            RF_CLR_REG_BIT(INTERRUPT_ENABLE2, BIT7)
#define RF_RXOK_INT_DISABLE()           RF_CLR_REG_BIT(INTERRUPT_ENABLE1, BIT1)
#define RF_TXOK_INT_DISABLE()           RF_CLR_REG_BIT(INTERRUPT_ENABLE1, BIT2)
#define RF_TXUND_INT_DISABLE()          RF_CLR_REG_BIT(INTERRUPT_ENABLE1, BIT7)
#define RF_RXOVR_INT_DISABLE()          RF_CLR_REG_BIT(INTERRUPT_ENABLE1, BIT7)

static int8_t  rx_rssi = 0;  
static rf_int_reg_t rf_int_reg[RF_INT_MAX_NUM];
static uint8_t rf_current_state;
static volatile uint8_t rf_power = 0;
static volatile uint8_t rf_channel = 0;

void rf_flush_rxfifo(void);
void rf_flush_txfifo(void);

void rf_write_reg(uint8_t addr, uint8_t value)
{
    RF_SPI_BEGIN();
    SPI_SEND_CHAR(addr | FIFO_WRITE_MASK);
    SPI_SEND_CHAR(value);
    RF_SPI_END();
}

uint8_t rf_read_reg(uint8_t addr)
{
    RF_SPI_BEGIN();
    SPI_SEND_CHAR(addr & FIFO_READ_MASK);
    SPI_SEND_CHAR(0xFF);
    RF_SPI_END();
	
	return RF_SPI_RECEIVE_CHAR();
}

static void rf_write_burst_reg(uint8_t addr, uint8_t *p_data, uint8_t count)
{
    RF_SPI_BEGIN();

    SPI_SEND_CHAR(addr | FIFO_WRITE_MASK);

    for( uint8_t i=0; i<count; i++ )
    {
        SPI_SEND_CHAR(p_data[i]);
    }

    RF_SPI_END();
}

static void rf_read_burst_reg(uint8_t addr, uint8_t *p_data, uint8_t count)
{
    RF_SPI_BEGIN();
    SPI_SEND_CHAR(addr & FIFO_READ_MASK);

    for (uint8_t i = 0; i < count; i++)
    {
        SPI_SEND_CHAR(0xFF);
        p_data[i] = RF_SPI_RECEIVE_CHAR();
    }

    RF_SPI_END();
}


static void rf_int_flag_clr(void)
{
    rf_read_reg(INTERRUPT_STATUS1); 
    rf_read_reg(INTERRUPT_STATUS2); 
    MCU_RADIO_INT_CLR(); 
}


bool_t rf_enter_sleep(void)
{
    uint32_t i = 0;
	if(rf_current_state == RF_SLEEP_STATE)
	{
	   return FALSE;
	}
    
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x00); ///  IDLE State, STANDBY Mode
	while ((rf_read_reg(DEVICE_STATUS) & 0x03) != 0) ///  注意&咿=的优先级＿的优先级使
	{
		TIMEOUT(i);
	}
	rf_current_state = RF_SLEEP_STATE;
    OSEL_EXIT_CRITICAL(s);
    
	return TRUE;
}

bool_t rf_enter_idle(void)
{
	if(rf_current_state == RF_IDLE_STATE)
	{
	   return FALSE;
	}
    
    uint32_t i = 0;
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x01); ///  IDLE State, READY Mode
	while ((rf_read_reg(DEVICE_STATUS) & 0x03) != 0) ///  注意&咿=的优先级＿的优先级使
    {
		TIMEOUT(i);
	}
	rf_current_state = RF_IDLE_STATE;
    OSEL_EXIT_CRITICAL(s);
    
	return TRUE;
}

bool_t rf_enter_rx(void)
{
    uint32_t i = 0;

	DBG_ASSERT(rf_current_state != RF_TX_STATE __DBG_LINE);
    if(rf_current_state == RF_RX_STATE)
	{
	   return FALSE;
	}
    
	rf_enter_idle();
	rf_flush_rxfifo();
	rf_int_flag_clr();
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);

	/* Enter RX mode */
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x05);
	while((rf_read_reg(DEVICE_STATUS) & 0x03) != 1) ///  注意&咿=的优先级＿的优先级使
	{
		TIMEOUT(i);
	}
	rf_current_state = RF_RX_STATE;
    OSEL_EXIT_CRITICAL(s);
	return TRUE;
}

bool_t rf_enter_tx(void)
{
	DBG_ASSERT(rf_current_state == RF_IDLE_STATE __DBG_LINE);   //必须从IDLE状态进入TX

	rf_enter_idle();
    
	rf_int_flag_clr();
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_current_state = RF_TX_STATE;

	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x09);
    
    uint32_t i = 0;
    do
    {
        TIMEOUT(i);
    } while((rf_read_reg(DEVICE_STATUS) & 0x03) != 0x02);   //!< 进入发送态就退出循环，不等待发送结板
    OSEL_EXIT_CRITICAL(s);
    
	return TRUE;
}

static bool_t rf_set_state(uint8_t state)
{
    switch(state)
    {
    case RF_IDLE_STATE:
        return rf_enter_idle();
    
    case RF_SLEEP_STATE:
        return rf_enter_sleep();
        
    case RF_TX_STATE:
        return rf_enter_tx();
        
    case RF_RX_STATE:
        return rf_enter_rx();
        
    default:
        return FALSE;
    }
}

void rf_flush_rxfifo(void)
{
    rf_enter_idle();
    rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x02);
    rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x00);	
}

void rf_flush_txfifo(void)
{
    rf_enter_idle();
    rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x01);
    rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x00);	
}

/* f_carrier = 中心频率 + (f_hch * f_hsx(20) * 10) [kHz] */
void rf_set_channel(uint8_t channel)
{    
//    rf_enter_idle();
    rf_channel = channel;
    rf_write_reg(FREQUENCY_HOPPING_CHANNEL_SELECT, channel);  
//    while((rf_read_reg(DEVICE_STATUS)&0x03) != RF_IDLE_STATE);
}

/*
 *  select power  { -1    2     5     8     11    14    17   20dbm }
 *  power index = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07  };
*/
void rf_set_power(uint8_t index)
{
    DBG_ASSERT(index <= 7 __DBG_LINE);
    if (index<=7)
    {
        rf_power = index;
//        rf_enter_idle();
        uint8_t reg_val = rf_read_reg(TX_POWER);
        reg_val &= ~0x07;
        rf_write_reg(TX_POWER, index|reg_val);
//        while((rf_read_reg(DEVICE_STATUS)&0x03) != RF_IDLE_STATE);
    }
}

static int8_t rf_get_rssi(void)
{
    uint8_t rssi1, rssi2, rssi3, rssi;

	rssi1 = rf_read_reg(RECEIVED_SIGNAL_STRENGTH_INDICATOR);
	rssi2 = rf_read_reg(RECEIVED_SIGNAL_STRENGTH_INDICATOR);
	rssi3 = rf_read_reg(RECEIVED_SIGNAL_STRENGTH_INDICATOR);
	///  majority polling
	if (rssi1 == rssi2)
		rssi = rssi1;
	else if (rssi2 == rssi3)
		rssi = rssi2;
	else
		rssi = rssi3;
	/// RSSI(dBm) = RSSI / 2 - 122，参考Si4432数据手册Figure 32. RSSI Value vs. Input Power
	return ((rssi>>1) -122);
}

int8_t rf_get_rx_rssi(void)
{
//    return rf_read_reg(RECEIVED_SIGNAL_STRENGTH_INDICATOR);
    return rx_rssi;
}


static void rf_sw_reset(void)
{
    radio_powerup();
    
    /* Wait at least 15ms befory any initialization SPI commands are sent to the radio
    (wait for the power on reset sequence)  */
	delay_ms(30);  
    
    /* read interrupt status registers to clear the interrupt flags and release NIRQ pin */
    rf_int_flag_clr();
    
	/* SW reset */
  	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x80);

	/* wait for POR interrupt from the radio (while the nIRQ pin is high) */
    MCU_RADIO_INT_GET();
    rf_int_flag_clr();

    MCU_RADIO_INT_GET();
    rf_int_flag_clr();
}

static void rf_settings(void)
{
/************************set the physical parameters***************************/	
    /* set the center frequency to 470 MHz */ 
    rf_write_reg(FREQUENCY_BAND_SELECT, 0x57);                  // reg.75
	rf_write_reg(NOMINAL_CARRIER_FREQUENCY1,0x00);              // reg.76
	rf_write_reg(NOMINAL_CARRIER_FREQUENCY0,0x00);              // reg.77

	/* set the desired TX data rate (100kbps) */
	rf_write_reg(TX_DATA_RATE1, 0x19);                          // reg.6E
	rf_write_reg(TX_DATA_RATE0, 0x9A);                          // reg.6F
	rf_write_reg(MODULATION_MODE_CONTROL1, 0x0C);               // reg.70
	rf_write_reg(CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE, 0xC0);   // reg.58
    
    rf_write_reg(FREQUENCY_DEVIATION, 0xa0);                    // reg.72
    /* Enable FIFO mode and GFSK modulation*/
	rf_write_reg(MODULATION_MODE_CONTROL2, 0x22);               // reg.71 

	/* set the modem parameters according to the exel calculator
    (parameters: 98 kbps, deviation: 47.6 kHz, channel BW 208.4kHz )
    IF Frequency 937.5 kHZ */
    rf_write_reg(IF_FILTER_BANDWIDTH, 0x88);					// reg.1C
	rf_write_reg(CLOCK_RECOVERY_OVERSAMPLING_RATIO, 0x78);		// reg.20
	rf_write_reg(CLOCK_RECOVERY_OFFSET2, 0x01);					// reg.21
	rf_write_reg(CLOCK_RECOVERY_OFFSET1, 0x11);					// reg.22
	rf_write_reg(CLOCK_RECOVERY_OFFSET0, 0x11);					// reg.23
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN1, 0x02);		// reg.24
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN0, 0x24);		// reg.25
	rf_write_reg(AFC_LOOP_GEARSHIFT_OVERRIDE, 0x40);			// reg.1D
	rf_write_reg(ANTENNA_DIVERSITY_REGISTER2, 0x48);			// reg.2A
	rf_write_reg(CLOCK_RECOVERY_GEARSHIFT_OVERRIDE, 0x03);		// reg.1F
	rf_write_reg(AGC_OVERRIDE1, 0x60);							// reg.69
    /* Frequency Hopping Step Size 20*10Khz*/
    rf_write_reg(FREQUENCY_HOPPING_STEP_SIZE, 0x14);            // reg.7A

/***************set the packet structure and the modulation type***************/   
	/* set the preamble length to 5 bytes */
	rf_write_reg(PREAMBLE_LENGTH, 0x0A);                        // reg.34

	/* set preamble detection threshold to 12bits*/
	rf_write_reg(PREAMBLE_DETECTION_CONTROL, 0x18);             // reg.35

	/* Set the sync word pattern to 0x2DD4*/
	rf_write_reg(SYNC_WORD3, 0x2D);                             // reg.36
	rf_write_reg(SYNC_WORD2, 0xD4);                             // reg.37

	/* Enable the TX & RX packet handler and CRC-16 (IBM) check without header,length */
    rf_write_reg(DATA_ACCESS_CONTROL, 0xAD);                    // reg.30
    
	/* disable the receive header filters(header 3) and Broadcast Addr*/
//    rf_write_reg(HEADER_CONTROL1, 0x00);                        // reg.32
    /* Broadcast Address (FF FFh) Check Enable, Received header check for byte 3 & byte 2.*/
    rf_write_reg(HEADER_CONTROL1, 0xCC);                        // reg.32
    /* 0 header byte; set variable packet length; synch word to two bytes*/
//    rf_write_reg(HEADER_CONTROL2, 0x02);                        // reg.33
    /* 2 header byte; set variable packet length; synch word to two bytes*/
    rf_write_reg(HEADER_CONTROL2, 0x22);                        // reg.33
    
/************************Some special Si4432 registers*************************/   
    /* set Crystal Oscillator Load Capacitance register */
    /* 晶体振荡器调谐电容，会影响载波中心频率 */
	rf_write_reg(CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE, 0x30);    // reg.09
    
    /* Errata-01  Register modifications are required for operation */
    rf_write_reg(DIVIDER_CURRENT_TRIMMING, 0x03);
    rf_write_reg(VCO_CURRENT_TRIMMING, 0x02);
    
    /* set GPIO0 & GPIO1 to control the TRX switch */
	rf_write_reg(GPIO0_CONFIGURATION, 0x12);
	rf_write_reg(GPIO1_CONFIGURATION, 0x15);
	    
	/*read interrupt status registers to release all pending interrupts */
    rf_int_flag_clr();
}

static void rf_int_cfg(void)
{
    /* Cover the default value, IntEn1 reset value = 0,  IntEn2 reset value = 3*/
    rf_write_reg(INTERRUPT_ENABLE1, 0x00); 
    rf_write_reg(INTERRUPT_ENABLE2, 0x00); 
    
    MCU_RADIO_INT_CLR(); 
    MCU_RADIO_INT_ENABLE();
}

bool_t si4432_init(void)
{
    radio_spi_init();
    radio_port_init();
    
    rf_sw_reset();
    rf_int_cfg();
    
    do
    {
        rf_settings();
    } while( rf_read_reg(CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE) != 0x30);

    rf_set_channel(CH_SN);
    rf_set_power(POWER_SN);
    rf_enter_idle();
    
    return TRUE;
}

int8_t si4432_prepare(uint8_t const *p_data, uint8_t count)
{
    rf_flush_txfifo();
    rf_write_burst_reg(FIFO_ACCESS, (uint8_t *)p_data, count);
    return count;
}

int8_t si4432_transmit(uint8_t len)
{
    rf_set_state(RF_TX_STATE);
    return len;
}

int8_t si4432_send(uint8_t const *p_data, uint8_t count)
{
    rf_flush_txfifo();
    rf_write_burst_reg(FIFO_ACCESS, (uint8_t *)p_data, count);
    rf_set_state(RF_TX_STATE);
    return count;
}

int8_t si4432_recv(uint8_t *p_data, uint8_t count)
{
    rf_read_burst_reg(FIFO_ACCESS, p_data, count);
    return count;
}


rf_result_t si4432_set_value(rf_cmd_t cmd, uint8_t value)
{
    rf_result_t ret = RF_RESULT_OK;
    switch(cmd)
    {
    case RF_STATE:
        if(!rf_set_state(value))
        {
            ret = RF_RESULT_ERROR;
        }
        break;
    
    case RF_POWER:
        rf_set_power(value);
        break;
        
    case RF_CHANNEL:
        rf_set_channel(value);
        break;
        
    case RF_DATARATE:
        ret = RF_RESULT_NOT_SUPPORTED;
        break;
        
    case RF_RXFIFO_FLUSH:
        rf_flush_rxfifo();
        break;
        
    case RF_TXFIFO_CNT:
        rf_write_reg(TRANSMIT_PACKET_LENGTH, value);
        break;
        
    case RF_TXFIFO_FLUSH:
        rf_flush_txfifo();
        break;
        
    case RF_LADDR1:
        rf_write_reg(CHECK_HEADER2, value);
        break;
        
    case RF_LADDR0:
        rf_write_reg(CHECK_HEADER3, value);
        break;
        
	case RF_DADDR1:
		rf_write_reg(TRANSMIT_HEADER2, value);
        break;
        
	case RF_DADDR0:
		rf_write_reg(TRANSMIT_HEADER3, value);
		break;
		
    default:
        ret = RF_RESULT_NOT_SUPPORTED;
        break;
    }
    
    return ret;
}


rf_result_t si4432_get_value(rf_cmd_t cmd, void *value)
{
    rf_result_t ret = RF_RESULT_OK;
    switch(cmd)
    {
    case RF_STATE:
        *(uint8_t *)value = rf_current_state;
        break;
    
    case RF_POWER:
        *(uint8_t *)value = rf_power;
        break;
        
    case RF_CHANNEL:
        *(uint8_t *)value = rf_channel;
        break;
        
    case RF_RXRSSI:
        *(int8_t *)value = rf_get_rx_rssi();
        break;
        
    case RF_CCA:
        *(int8_t *)value = rf_get_rssi();
        break;
        
    case RF_RXFIFO_CNT:
        *(uint8_t *)value = rf_read_reg(RECEIVED_PACKET_LENGTH);
        break;
    
    case RF_TXFIFO_CNT:
        *(uint8_t *)value = rf_read_reg(TRANSMIT_PACKET_LENGTH);
        break;
        
    default:
        ret = RF_RESULT_NOT_SUPPORTED;
        break;
    }
    
    return ret;
}

static bool_t si4432_int_cfg(rf_int_t state,
                           rf_int_reg_t int_cb,
                           uint8_t type)
{
    (type == INT_ENABLE) ? ((int_cb != NULL) ? (rf_int_reg[state] = int_cb) : (NULL)) : (NULL);
    
    switch(state)
    {
    case RX_SFD_INT:
    case TX_SFD_INT:
        RF_CLEAR_SFD_FLAG();
        if(type == INT_ENABLE)
            RF_SFD_INT_ENABLE();
        else
            RF_SFD_INT_DISABLE();
        break;
        
    case RX_OK_INT:
        RF_CLEAR_RXOK_FLAG();
        if(type == INT_ENABLE)
            RF_RXOK_INT_ENABLE();
        else
            RF_RXOK_INT_DISABLE();
        break;
       
    case TX_OK_INT:
        RF_CLEAR_TXOK_FLAG();
        if(type == INT_ENABLE)
            RF_TXOK_INT_ENABLE();
        else
            RF_TXOK_INT_DISABLE();
        break;
        
    case TX_UND_INT:
        RF_CLEAR_TXUND_FLAG();
        if(type == INT_ENABLE)
            RF_TXUND_INT_ENABLE();
        else
            RF_TXUND_INT_DISABLE();
        break;
        
    case RX_OVR_INT:
        RF_CLEAR_RXOVR_FLAG();
        if(type == INT_ENABLE)
            RF_RXOVR_INT_ENABLE();
        else
            RF_RXOVR_INT_DISABLE();
        break;
        
    default:
        break;
    }
    
    return TRUE;
}


const struct radio_driver si4432_driver =
{
    si4432_init,

    si4432_prepare,
    si4432_transmit,

    si4432_send,
    si4432_recv,

    si4432_get_value,
    si4432_set_value,

    si4432_int_cfg,
};

void rf_int_handler(uint16_t time)
{
	uint8_t status1 = rf_read_reg(INTERRUPT_STATUS1);
	uint8_t status2 = rf_read_reg(INTERRUPT_STATUS2);
    
	if (status1 & BIT7)
	{
        //rf_read_reg(DEVICE_STATUS) & BIT7
		if (rf_current_state == RF_RX_STATE)
		{
			DBG_ASSERT(rf_int_reg[RX_OVR_INT] != NULL __DBG_LINE);
			if (rf_int_reg[RX_OVR_INT] != NULL)
			{
				( *(rf_int_reg[RX_OVR_INT]) )(time);
			}
		}
		else if (rf_current_state == RF_TX_STATE)
		{
			DBG_ASSERT(rf_int_reg[TX_UND_INT] != NULL __DBG_LINE);
			if (rf_int_reg[TX_UND_INT] != NULL)
			{
				( *(rf_int_reg[TX_UND_INT]) )(time);
			}
		}
#if TEST_DEBUG_INFO_EN > 0
		static uint64_t rf_int_flow_cnt = 0;
		rf_int_flow_cnt++;
#endif
	}

	if (status1 & BIT2)
	{
		DBG_ASSERT(rf_int_reg[TX_OK_INT] != NULL __DBG_LINE);
		DBG_ASSERT(rf_current_state == RF_TX_STATE __DBG_LINE);
		rf_current_state = RF_TXOK_STATE;
		if ( rf_int_reg[TX_OK_INT] != NULL )
        {
            ( *(rf_int_reg[TX_OK_INT]) )(0);
        }
	}

	if (status1 & BIT1)
	{
		DBG_ASSERT(rf_int_reg[RX_OK_INT] != NULL __DBG_LINE);
		if (rf_current_state == RF_RX_STATE)
		{
			rf_current_state = RF_RXOK_STATE;

			if (rf_int_reg[RX_OK_INT] != NULL)
			{
				( *(rf_int_reg[RX_OK_INT]) )(time);
			}
		}
		else
		{
			return;
		}
	}

	if (status2 & BIT7)
	{
		if (rf_current_state == RF_RX_STATE)
		{
            rx_rssi = rf_get_rssi();
			DBG_ASSERT(rf_int_reg[RX_SFD_INT] != NULL __DBG_LINE);
			if (rf_int_reg[RX_SFD_INT] != NULL)
			{
				( *(rf_int_reg[RX_SFD_INT]) )( time );
			}
		}
		else if (rf_current_state == RF_TX_STATE)
		{
			DBG_ASSERT(rf_int_reg[TX_SFD_INT] != NULL __DBG_LINE);
			if (rf_int_reg[TX_SFD_INT] != NULL)
			{
				( *(rf_int_reg[TX_SFD_INT]) )( time );
			}
		}
#if TEST_DEBUG_INFO_EN > 0
		static uint64_t rf_int_sfd_cnt = 0;
		rf_int_sfd_cnt++;
#endif
	}
}





