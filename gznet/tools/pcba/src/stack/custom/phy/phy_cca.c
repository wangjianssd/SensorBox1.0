/**
 * @brief       : Clear Channel Assessment and relative functions
 *
 * @file        : phy_cca.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <driver.h>
//#include <hal.h>
#include <dev.h>
#include <phy_cca.h>
#include <phy_state.h>

bool_t phy_cca(void)
{
    /* CCA期间不接收数据，接收时间较短，无溢出危险 */
    SSN_RADIO.int_cfg(RX_SFD_INT, NULL, INT_DISABLE);
    SSN_RADIO.int_cfg(RX_OK_INT, NULL, INT_DISABLE);

    phy_set_state(PHY_RX_STATE);
    delay_us(600); //等待RSSI寄存器值有效
    
    int8_t rssi_dbm0 = 0;
    int8_t rssi_dbm1 = 0;
    SSN_RADIO.get_value(RF_CCA, &rssi_dbm0);
    delay_us(200);
    SSN_RADIO.get_value(RF_CCA, &rssi_dbm1);
    
    int16_t rssi_dbm = 0;
    rssi_dbm = rssi_dbm0 + rssi_dbm1;
    
    rssi_dbm = (rssi_dbm >> 1);

    phy_set_state(PHY_IDLE_STATE);

    SSN_RADIO.int_cfg(RX_SFD_INT, NULL, INT_ENABLE);
    SSN_RADIO.int_cfg(RX_OK_INT, NULL, INT_ENABLE);

    if (rssi_dbm >= CCA_RSSI_THRESHOLD)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

bool_t phy_cca_stop(void)
{
	return TRUE;
}

int8_t phy_get_rssi_largest(void)
{
//    return rf_get_rssi();
    return 0;
}

uint8_t phy_get_rssi(void)
{
//    return (rf_get_rssi() >> 1);
    return 0;
}

int8_t phy_rssi_average(uint8_t num)
{
    return 0;
}
