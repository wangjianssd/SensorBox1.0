/**
 * @brief       : PHY state control
 *
 * @file        : phy_state.c
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
#include <hal.h>
#include <dev.h>
#include <phy_state.h>

uint8_t phy_get_state(void)
{
    uint8_t state = RF_INVALID_STATE;
    SSN_RADIO.get_value(RF_STATE, &state);
    
    uint8_t phy_rf_state = PHY_INVALID_STATE;

    switch(state)
    {
    case RF_IDLE_STATE:
        phy_rf_state = PHY_IDLE_STATE;
        break;

    case RF_RX_STATE:
        phy_rf_state = PHY_RX_STATE;
        break;

    case RF_TX_STATE:
        phy_rf_state = PHY_TX_STATE;
        break;

    case RF_SLEEP_STATE:
        phy_rf_state = PHY_SLEEP_STATE;
        break;

    case RF_INVALID_STATE:
        phy_rf_state = PHY_INVALID_STATE;
        break;

    default:
        break;
    }

    return phy_rf_state;
}


bool_t phy_set_state(uint8_t phy_state)
{
    rf_state_t rf_state_wanted = RF_INVALID_STATE;
    switch(phy_state)
    {
    case PHY_RX_STATE:
        rf_state_wanted = RF_RX_STATE;
        break;

    case PHY_TX_STATE:
        rf_state_wanted = RF_TX_STATE;
        break;

    case PHY_IDLE_STATE:
        rf_state_wanted = RF_IDLE_STATE;
        break;

    case PHY_SLEEP_STATE:
        rf_state_wanted = RF_SLEEP_STATE;
        break;

    default:
      break;
    }
    
    if(SSN_RADIO.set_value(RF_STATE, rf_state_wanted) == RF_RESULT_OK)
    {
        return TRUE;
    }
    
    return FALSE;
}


bool_t phy_set_channel(uint8_t channel_index)
{
    if( SSN_RADIO.set_value(RF_CHANNEL, channel_index) == RF_RESULT_OK)
    {
        return TRUE;
    }
    
    return FALSE;
}


uint8_t phy_get_power(void)
{
    uint8_t power = 0;
    SSN_RADIO.get_value(RF_POWER, &power);
    return power;
}

bool_t phy_set_power(uint8_t power_index)
{
    if( SSN_RADIO.set_value(RF_POWER, power_index) == RF_RESULT_OK)
    {
        return TRUE;
    }
    
    return FALSE;
}
