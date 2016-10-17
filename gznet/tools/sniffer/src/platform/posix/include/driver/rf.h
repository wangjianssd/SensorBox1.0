/**
 * @brief       : provides an abstraction for baseband.
 *
 * @file        : rf.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/8
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/8    v0.0.1      gang.cheng    some notes
 */


#ifndef __RF_H
#define __RF_H

/* RF中断类型宏定义 */
#define TX_SFD_INT		                            0u
#define RX_SFD_INT		                            1u
#define RX_OK_INT		                            2u
#define TX_OK_INT		                            3u
#define TX_UND_INT		                            4u
#define RX_OVR_INT		                            5u
#define RF_INT_MAX_NUM                              6u

#define INT_DISABLE                                 0u
#define INT_ENABLE                                  1u

/* 用于描述驱动层状态的宏定义，非芯片状态值 */
#define RF_IDLE_STATE			                    0x0F
#define RF_RX_STATE				                    0x1F
#define RF_TX_STATE					                0x2F
#define RF_SLEEP_STATE				                0x3F
#define RF_TXOK_STATE				                0x4F
#define RF_RXOK_STATE				                0x5F
#define RF_INVALID_STATE			                0xFF


typedef void (*rf_int_reg_t)( uint16_t time );
extern rf_int_reg_t rf_int_reg[RF_INT_MAX_NUM];

void rf_write_reg(uint8_t addr, uint8_t value);
uint8_t rf_read_reg(uint8_t addr);
uint8_t rf_write_fifo(uint8_t  *p_data, uint8_t count);
uint8_t rf_read_fifo(uint8_t *p_data, uint8_t count);

bool_t rf_enter_idle(void);
bool_t rf_enter_sleep(void);
bool_t rf_enter_rx(void);
bool_t rf_enter_tx(void);
void rf_wakeup(void);

void rf_flush_rxfifo(void);
void rf_flush_txfifo(void);

void rf_set_power(uint8_t index);
void rf_set_channel( uint8_t channel );
void rf_set_address(uint16_t id);

uint8_t rf_get_rxfifo_cnt(void);
uint8_t rf_get_txfifo_cnt(void);
int16_t rf_get_rssi(void);
/**
 * 获取在RX_SFD中读取的RSSI值，因为Si4432每次发送的数据较长时，
 * 比如48字节，在RX中断里读出来的RSSI就会失真
 *
 * @param: 无
 * @return: rssi的值
 */
int8_t rf_get_rx_rssi(void);

uint8_t rf_get_state(void);
bool_t rf_rxfifo_overflow(void);
bool_t rf_txfifo_underflow(void);


bool_t rf_sw_reset(void);
bool_t rf_init(void);

void rf_int_handler(uint16_t time);
void rf_int_cfg(void);

void rf_cfg_int(uint8_t int_type, uint8_t ctrl_type);
bool_t rf_reg_int(uint16_t int_type, rf_int_reg_t cb_fun_ptr);
bool_t rf_unreg_int(uint8_t int_type);
void rf_int_handler(uint16_t time);

#endif

/**
 * @}
 */