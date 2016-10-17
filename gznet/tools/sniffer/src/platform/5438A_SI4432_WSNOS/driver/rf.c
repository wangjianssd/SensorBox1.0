/**
 * @brief       : 
 *
 * @file        : rf.c
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
#include <wsnos.h>

#define  TEST_DEBUG_INFO_EN    0u
#define	 TIMEOUT(i)		                            \
    i = 0;                                          \
    do                                              \
    {                                               \
        if((i)++ > 70000)                           \
        {                                           \
            DBG_ASSERT(FALSE __DBG_LINE);           \
        }                                           \
    } while(__LINE__ == -1)

static int8_t  rx_rssi = 0;
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

#define RF_SPI_BEGIN()                              \
    do                                              \
    {                                               \
        P3OUT &= ~BIT0;                             \
    } while(__LINE__ == -1)

#define RF_SPI_END() (P3OUT |=  BIT0)

#define RF_SPI_RECEIVE_CHAR()	(UCB0RXBUF)
#define SPI_SEND_CHAR(x)                            \
    do                                              \
    {                                               \
		WHILE (!(UCB0IFG&UCTXIFG));					\
        UCB0TXBUF = (x);		                    \
        WHILE (!(UCB0IFG&UCTXIFG));                 \
        WHILE (!(UCB0IFG&UCRXIFG));					\
    } while(__LINE__ == -1)

#define CLR_RF_MCU_INT_FLG()                        \
    do                                              \
    {                                               \
        rf_read_reg(INTERRUPT_STATUS1);             \
        rf_read_reg(INTERRUPT_STATUS2);             \
        TA0CCTL1 &= ~CCIFG;                         \
    } while(__LINE__ == -1)

#define DISABLE_RF_MCU_INT()                        \
    do                                              \
    {                                               \
        rf_write_reg(INTERRUPT_ENABLE1, 0);         \
        rf_write_reg(INTERRUPT_ENABLE2, 0);         \
        TA0CCTL1 &= ~CCIFG;                         \
    } while(__LINE__ == -1)

#define RF_CLEAR_SFD_FLAG()				TA0CCTL1 &= ~CCIFG
#define RF_CLEAR_RXOK_FLAG()			TA0CCTL1 &= ~CCIFG
#define RF_CLEAR_TXOK_FLAG()			TA0CCTL1 &= ~CCIFG
#define RF_CLEAR_TXUND_FLAG()			TA0CCTL1 &= ~CCIFG
#define RF_CLEAR_RXOVR_FLAG()			TA0CCTL1 &= ~CCIFG

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

static rf_int_reg_t rf_int_reg[RF_INT_MAX_NUM];
static volatile uint8_t rf_current_state = RF_INVALID_STATE;

void rf_write_reg(uint8_t addr, uint8_t value)
{
	uint8_t temp;
	osel_int_status_t s;

	OSEL_ENTER_CRITICAL(s);
	RF_SPI_BEGIN();
	SPI_SEND_CHAR(addr | FIFO_WRITE_MASK);
	temp = RF_SPI_RECEIVE_CHAR();
	SPI_SEND_CHAR(value);
	temp = RF_SPI_RECEIVE_CHAR();
	RF_SPI_END();
	OSEL_EXIT_CRITICAL(s);
	temp = temp;
}

static void rf_write_burst_reg(uint8_t addr, uint8_t *p_data, uint8_t count)
{
	uint8_t temp;
	osel_int_status_t s;
    OSEL_ENTER_CRITICAL(s);
	RF_SPI_BEGIN();
	SPI_SEND_CHAR(addr | FIFO_WRITE_MASK);
	temp = RF_SPI_RECEIVE_CHAR();

	for (uint8_t i=0; i<count; i++)
	{
		SPI_SEND_CHAR(p_data[i]);
		temp = RF_SPI_RECEIVE_CHAR();
	}

	RF_SPI_END();
	OSEL_EXIT_CRITICAL(s);
	temp = temp;
}

uint8_t rf_read_reg(uint8_t addr)
{
	uint8_t temp;
	osel_int_status_t s;

	OSEL_ENTER_CRITICAL(s);
	RF_SPI_BEGIN();
	SPI_SEND_CHAR(addr & FIFO_READ_MASK);
	temp = RF_SPI_RECEIVE_CHAR();
	SPI_SEND_CHAR(0xFF);
	temp = RF_SPI_RECEIVE_CHAR();
	RF_SPI_END();
	OSEL_EXIT_CRITICAL(s);

	return temp;
}

static void rf_read_burst_reg(uint8_t addr, uint8_t *p_data, uint8_t count)
{
	uint8_t temp;
	osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	RF_SPI_BEGIN();
	SPI_SEND_CHAR(addr & FIFO_READ_MASK);
	temp = RF_SPI_RECEIVE_CHAR();

	for (uint8_t i = 0; i < count; i++)
	{
		SPI_SEND_CHAR(0xFF);
		p_data[i] = RF_SPI_RECEIVE_CHAR();
	}

	RF_SPI_END();
	OSEL_EXIT_CRITICAL(s);
	temp = temp;
}

uint8_t rf_write_fifo(uint8_t *p_data, uint8_t count)
{
    rf_enter_idle();
	rf_flush_txfifo();
	rf_write_burst_reg(FIFO_ACCESS, p_data, count);

	return count;
}

uint8_t rf_read_fifo(uint8_t *p_data, uint8_t count)
{
	rf_read_burst_reg(FIFO_ACCESS, p_data, count);
	return count;
}

/* 仅当fixpklen = 0时有效，即帧头中包括长度�?*/
uint8_t rf_get_rxfifo_cnt(void)
{
	return rf_read_reg(RECEIVED_PACKET_LENGTH);
}

uint8_t rf_get_txfifo_cnt(void)
{
	return rf_read_reg(TRANSMIT_PACKET_LENGTH);
}

bool_t rf_enter_sleep(void)
{
    uint32_t i = 0;
//    DBG_ASSERT(rf_current_state != RF_TX_STATE __DBG_LINE);
	if(rf_current_state == RF_SLEEP_STATE)
	{
	   return FALSE;
	}
    
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x00); ///  IDLE State, STANDBY Mode
	while ((rf_read_reg(DEVICE_STATUS) & 0x03) != 0) ///  注意&�?=的优先级�?的优先级�?
	{
		TIMEOUT(i);
	}
	rf_current_state = RF_SLEEP_STATE;
    OSEL_EXIT_CRITICAL(s);
    
	return TRUE;
}

bool_t rf_enter_idle(void)
{
//    DBG_ASSERT(rf_current_state != RF_TX_STATE __DBG_LINE);
	if(rf_current_state == RF_IDLE_STATE)
	{
	   return FALSE;
	}
    
    uint32_t i = 0;
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x01); ///  IDLE State, READY Mode
	while ((rf_read_reg(DEVICE_STATUS) & 0x03) != 0) ///  注意&�?=的优先级�?的优先级�?
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
//    do
//    {
//        TIMEOUT(i);
//    } while(rf_current_state == RF_TX_STATE);
    
	DBG_ASSERT(rf_current_state != RF_TX_STATE __DBG_LINE);
    if(rf_current_state == RF_RX_STATE)
	{
	   return FALSE;
	}
    
	rf_enter_idle();
	rf_flush_rxfifo();
	CLR_RF_MCU_INT_FLG();
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);

	/* Enter RX mode */
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x05);
	while((rf_read_reg(DEVICE_STATUS) & 0x03) != 1) ///  注意&�?=的优先级�?的优先级�?
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
    
	CLR_RF_MCU_INT_FLG();
    osel_int_status_t s;
	OSEL_ENTER_CRITICAL(s);
	rf_current_state = RF_TX_STATE;

	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x09);
    
    uint32_t i = 0;
    do
    {
//        if(rf_current_state == RF_TXOK_STATE)
//        {
//            break;
//        }
        TIMEOUT(i);
    } while((rf_read_reg(DEVICE_STATUS) & 0x03) != 0x02);   //!< 进入发送态就退出循环，不等待发送结�?
    OSEL_EXIT_CRITICAL(s);
    
	return TRUE;
}

void rf_wakeup(void)
{
	uint8_t temp8;

	temp8 = rf_read_reg(OPERATING_FUNCTION_CONTROL1);
	if (temp8 != 0x00)
	{
		return;
	}
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x01);
	delay_us(100);
}

/* f_carrier = 中心频率 + (f_hch * f_hsx(20) * 10) [kHz] */
void rf_set_channel(uint8_t channel)
{
	rf_write_reg(FREQUENCY_HOPPING_CHANNEL_SELECT, channel);
}

void rf_set_address(uint16_t id)
{
    rf_write_burst_reg(CHECK_HEADER3, (uint8_t *)&id, 2);
}

uint8_t rf_get_state(void)
{
	return rf_current_state;
}

bool_t rf_rxfifo_overflow(void)
{
	if (rf_read_reg(DEVICE_STATUS) & 0x80)
		return TRUE;
	return FALSE;
}

bool_t rf_txfifo_underflow(void)
{
	if (rf_read_reg(DEVICE_STATUS) & 0x40)
		return TRUE;
	return FALSE;
}

void rf_flush_rxfifo(void)
{
	rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x02);
	rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x00);
}

void rf_flush_txfifo(void)
{
	rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x01);
	rf_write_reg(OPERATING_FUNCTION_CONTROL2, 0x00);
}

int16_t rf_get_rssi(void)
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
	return rx_rssi;
}

static void rf_settings(void)
{
	/************************set the physical parameters***************************/
	/* set the center frequency to 470 MHz */
	rf_write_reg(FREQUENCY_BAND_SELECT, 0x57);					// reg.75
	rf_write_reg(NOMINAL_CARRIER_FREQUENCY1,0x00);				// reg.76
	rf_write_reg(NOMINAL_CARRIER_FREQUENCY0,0x00);				// reg.77
	/* Frequency Hopping Step Size 20*10Khz*/
	rf_write_reg(FREQUENCY_HOPPING_STEP_SIZE, 0x14);			// reg.7A

#if DATA_RATE == 100u
	/* set the desired TX data rate (100kbps) */
	rf_write_reg(TX_DATA_RATE1, 0x19);							// reg.6E
	rf_write_reg(TX_DATA_RATE0, 0x9A);							// reg.6F
	/* Data Whitening enabled*/
	//	rf_write_reg(MODULATION_MODE_CONTROL1, 0x01);               // reg.70
	rf_write_reg(MODULATION_MODE_CONTROL1, 0x0c);				// reg.70 -- debug : no data whitening
	rf_write_reg(CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE, 0xC0);	// reg.58
    rf_write_reg(FREQUENCY_DEVIATION, 0x4C);					// reg.72
	/* TX Data CLK via GPIO, Enable FIFO mode and GFSK modulation*/
	rf_write_reg(MODULATION_MODE_CONTROL2, 0x63);				// reg.71

#elif DATA_RATE == 10u
    rf_write_reg(TX_DATA_RATE1, 0x4E);							// reg.6E
	rf_write_reg(TX_DATA_RATE0, 0xA5);							// reg.6F
	rf_write_reg(MODULATION_MODE_CONTROL1, 0x2C);				// reg.70 -- debug : no data whitening
	rf_write_reg(CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE, 0x80);	// reg.58
    rf_write_reg(FREQUENCY_DEVIATION, 0x48);					// reg.72
	/* TX Data CLK via GPIO, Enable FIFO mode and GFSK modulation*/
	rf_write_reg(MODULATION_MODE_CONTROL2, 0x63);				// reg.71
#endif

	/* set the modem parameters according to the exel calculator
	(parameters: 98 kbps, deviation: 47.6 kHz, channel BW 208.4kHz )
	IF Frequency 937.5 kHZ */
#if DATA_RATE == 100u
	rf_write_reg(IF_FILTER_BANDWIDTH, 0x9A);					// reg.1C
	rf_write_reg(CLOCK_RECOVERY_OVERSAMPLING_RATIO, 0x3C);		// reg.20
	rf_write_reg(CLOCK_RECOVERY_OFFSET2, 0x02);					// reg.21
	rf_write_reg(CLOCK_RECOVERY_OFFSET1, 0x22);					// reg.22
	rf_write_reg(CLOCK_RECOVERY_OFFSET0, 0x22);					// reg.23
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN1, 0x07);		// reg.24
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN0, 0xFF);		// reg.25
	rf_write_reg(AFC_LOOP_GEARSHIFT_OVERRIDE, 0x44);			// reg.1D
	rf_write_reg(AFC_TIMING_CONTROL, 0x0A);						// reg.1E
	rf_write_reg(ANTENNA_DIVERSITY_REGISTER2, 0x48);			// reg.2A
	rf_write_reg(CLOCK_RECOVERY_GEARSHIFT_OVERRIDE, 0x03);		// reg.1F
	rf_write_reg(AGC_OVERRIDE1, 0x60);							// reg.69

#elif DATA_RATE == 10u
    rf_write_reg(IF_FILTER_BANDWIDTH, 0xAB);					// reg.1C
	rf_write_reg(CLOCK_RECOVERY_OVERSAMPLING_RATIO, 0x39);		// reg.20
	rf_write_reg(CLOCK_RECOVERY_OFFSET2, 0x20);					// reg.21
	rf_write_reg(CLOCK_RECOVERY_OFFSET1, 0x68);					// reg.22
	rf_write_reg(CLOCK_RECOVERY_OFFSET0, 0xDC);					// reg.23
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN1, 0x00);		// reg.24
	rf_write_reg(CLOCK_RECOVERY_TIMING_LOOP_GAIN0, 0x2C);		// reg.25
	rf_write_reg(AFC_LOOP_GEARSHIFT_OVERRIDE, 0x44);			// reg.1D
	rf_write_reg(AFC_TIMING_CONTROL, 0x0A);						// reg.1E
	rf_write_reg(ANTENNA_DIVERSITY_REGISTER2, 0x24);			// reg.2A
	rf_write_reg(CLOCK_RECOVERY_GEARSHIFT_OVERRIDE, 0x03);		// reg.1F
	rf_write_reg(AGC_OVERRIDE1, 0x60);                          // reg.69
#endif

	/***************set the packet structure and the modulation type***************/
	/* set local addr */
    extern uint16_t hal_board_get_device_short_addr(void);
	uint16_t id = hal_board_get_device_short_addr();
//    uint16_t id = 0xFFFF;
	rf_write_burst_reg(CHECK_HEADER3, (uint8_t *)&id, 2);

	/* set the preamble length to 5 bytes */
	rf_write_reg(PREAMBLE_LENGTH, 0x0A);						// reg.34

	/* set preamble detection threshold to 20bits*/
	rf_write_reg(PREAMBLE_DETECTION_CONTROL, 0x2A);				// reg.35

	/* Enable the TX & RX packet handler and CRC-16 (IBM) check with header */
	rf_write_reg(DATA_ACCESS_CONTROL, 0x8D);					// reg.30
	/* Broadcast Address (FFh) Check Enable, Received header check for byte 3.*/
	#if (NODE_TYPE == NODE_TYPE_DETECTOR)
	rf_write_reg(HEADER_CONTROL1, 0x88);						// reg.32
	#else
	rf_write_reg(HEADER_CONTROL1, 0x88);						// reg.32
	#endif
	/* 1 header byte; set variable packet length; synch word to two bytes*/
	rf_write_reg(HEADER_CONTROL2, 0x12);						// reg.33

	/* Set the sync word pattern to 0x2DD4*/
	rf_write_reg(SYNC_WORD3, 0x2D);								// reg.36
	rf_write_reg(SYNC_WORD2, 0xD4);								// reg.37

	/************************Some special Si4432 registers*************************/

	//    rf_write_reg(CHECK_HEADER3, (uint8_t)DEV_ADR_L);

	/* set Crystal Oscillator Load Capacitance register */
	/* 晶体振荡器调谐电容，会影响载波中心频�?*/
	rf_write_reg(CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE, 0xD7);	// reg.09

	/* Errata-01  Register modifications are required for operation */
	rf_write_reg(DIVIDER_CURRENT_TRIMMING, 0x03);
	rf_write_reg(VCO_CURRENT_TRIMMING, 0x02);

	//set GPIO0 & GPIO1 to control the TRX switch
	rf_write_reg(GPIO0_CONFIGURATION, 0x12);
	rf_write_reg(GPIO1_CONFIGURATION, 0x15);

	/*read interrupt status registers to release all pending interrupts */
	CLR_RF_MCU_INT_FLG();
}

/*
*  select power  { -1    2     5     8     11    14    17   20dbm }
*  power index = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07  };
*/
void rf_set_power(uint8_t index)
{
	DBG_ASSERT(index <= 7 __DBG_LINE);
	if (index <= 7)
	{
		uint8_t reg_val = rf_read_reg(TX_POWER);
		reg_val &= ~0x07;
		rf_write_reg(TX_POWER, index | reg_val);
	}
}


static void rf_spi_init(void)
{
	osel_int_status_t s;

	OSEL_ENTER_CRITICAL(s);

	UCB0CTL1 = UCSWRST;
	/*! SCLK常态低电平，上升沿采样，MSB在前�?位数据，主模式，3-pin，同步模�? */
	UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCB0CTL1 |= UCSSEL_2; ///  SMCLK
	UCB0BR0 = 2;
	UCB0BR1 = 0;	///  SCLK频率是SMCLK / 2
	UCB0IE &= ~(UCRXIE + UCTXIE);	  ///  禁止SPI中断
	UCB0CTL1 &= ~UCSWRST;
	P3SEL |= (BIT1 + BIT2 + BIT3);
	P3SEL &= ~BIT0;						//STE
	P3DIR |= (BIT0 + BIT1 + BIT3);
	P3DIR &= ~BIT2;
	P3OUT |= BIT0;

	OSEL_EXIT_CRITICAL(s);
}

bool_t rf_sw_reset(void)
{
	/* 拉低PWRDN(SDN)脚，使能RF */
	P2OUT &= ~BIT4;

	/* Wait at least 15ms befory any initialization SPI commands are sent to the radio
	(wait for the power on reset sequence)  */
	delay_ms(30);

	/* read interrupt status registers to clear the interrupt flags and release NIRQ pin */
	CLR_RF_MCU_INT_FLG();

	/* SW reset */
	rf_write_reg(OPERATING_FUNCTION_CONTROL1, 0x80);

	/* wait for POR interrupt from the radio (while the nIRQ pin is high) */
	while ((P8IN & BIT1)== BIT1);
	CLR_RF_MCU_INT_FLG();
	//wait until the chip ready interrupt is occured
	while ((P8IN & BIT1)== BIT1);
	CLR_RF_MCU_INT_FLG();

	return TRUE;
}

static void rf_port_init(void)
{
	//SLP
	P2SEL &= ~BIT4;
	P2DIR |= BIT4;
	//GPIO_2
	P2SEL &= ~BIT5;
	P2DIR &= ~BIT5;
	//nIRQ
	P8SEL |= BIT1;
	P8DIR &= ~BIT1;	//nIRQ captured by TA0CCR1
	TA0CCTL1 = CM_2 + CCIS_1 + SCS + CAP; ///  下降沿capture，CCI3B，同�?
}

static void rf_int_cfg(void)
{
	/* Cover the default value, IntEn1 reset value = 0,  IntEn2 reset value = 3*/
	rf_write_reg(INTERRUPT_ENABLE1, 0x00);
	rf_write_reg(INTERRUPT_ENABLE2, 0x00);
	TA0CCTL1 &= ~CCIFG;
	TA0CCTL1 |= CCIE;
}

void rf_cfg_int(uint8_t int_type, uint8_t ctrl_type)
{
	switch (int_type)
	{
		case RX_SFD_INT:		// 与tx sfd一�?
		case TX_SFD_INT:
			RF_CLEAR_SFD_FLAG();
			if (ctrl_type == INT_ENABLE)
			{
				RF_SFD_INT_ENABLE();
			}
			else
			{
				RF_SFD_INT_DISABLE();
			}
			break;
		case RX_OK_INT:
			RF_CLEAR_RXOK_FLAG();
			if (ctrl_type == INT_ENABLE)
			{
				RF_RXOK_INT_ENABLE();
			}
			else
			{
				RF_RXOK_INT_DISABLE();
			}
			break;
		case TX_OK_INT:
			RF_CLEAR_TXOK_FLAG();
			if (ctrl_type == INT_ENABLE)
			{
				RF_TXOK_INT_ENABLE();
			}
			else
			{
				RF_TXOK_INT_DISABLE();
			}
			break;
		case TX_UND_INT:
			RF_CLEAR_TXUND_FLAG();
			if (ctrl_type == INT_ENABLE)
			{
				RF_TXUND_INT_ENABLE();
			}
			else
			{
				RF_TXUND_INT_DISABLE();
			}
			break;
		case RX_OVR_INT:
			RF_CLEAR_RXOVR_FLAG();
			if (ctrl_type == INT_ENABLE)
			{
				RF_RXOVR_INT_ENABLE();
			}
			else
			{
				RF_RXOVR_INT_DISABLE();
			}
			break;
		default:
			break;
	}
}

bool_t rf_reg_int(uint16_t int_type, rf_int_reg_t cb_fun_ptr)
{
	if (rf_int_reg[int_type] == NULL)
	{
		rf_int_reg[int_type] = cb_fun_ptr;
		return TRUE;
	}
	return FALSE;
}

bool_t rf_unreg_int(uint8_t int_type)
{
	rf_int_reg[int_type] = NULL;
	return TRUE;
}

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

bool_t rf_init(void)
{
	for (uint8_t i = 0; i < RF_INT_MAX_NUM; i++)
    {
		rf_int_reg[i] = NULL;
    }

	rf_spi_init();
	rf_port_init();
	rf_sw_reset();

	do
	{
		rf_settings();
	} while (rf_read_reg(CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE) != 0xD7);

	rf_set_channel(CH_SN);
	rf_set_power(POWER_SN);
	rf_enter_sleep();
	rf_int_cfg();

	return TRUE;
}