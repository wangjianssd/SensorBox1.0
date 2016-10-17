/**
 * @brief       : this
 * @file        : radio.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2016-01-09
 * change logs  :
 * Date       Version     Author        Note
 * 2016-01-09  v0.0.1  gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "sys_arch/osel_arch.h"

#include "../radio_defs.h"
#include "radio.h"

#include <stdio.h>


DBG_THIS_MODULE("SimRadio")

#define  TIMEOUT(i)                                 \
    i = 0;                                          \
    do                                              \
    {                                               \
        if((i)++ > 70000)                           \
        {                                           \
            DBG_ASSERT(FALSE __DBG_LINE);           \
        }                                           \
    } while(__LINE__ == -1)

static rf_int_reg_t rf_int_reg[RF_INT_MAX_NUM];

static bool_t sim_radio_init(void)
{
    DBG_LOG(DBG_LEVEL_INFO, "sim_raido_init");
    return TRUE;
}

static int8_t sim_radio_prepare(uint8_t const *p_data, uint8_t count)
{
    DBG_LOG(DBG_LEVEL_INFO, p);
    return TRUE;
}

int8_t sim_radio_transmit(uint8_t len)
{
    return len;
}

int8_t sim_radio_send(uint8_t const *p_data, uint8_t count)
{
    return count;
}

int8_t sim_radio_recv(uint8_t *p_data, uint8_t count)
{
    return count;
}

rf_result_t sim_radio_set_value(rf_cmd_t cmd, uint8_t value)
{
    rf_result_t ret = RF_RESULT_OK;

    return ret;
}

rf_result_t sim_radio_get_value(rf_cmd_t cmd, void *value)
{
    rf_result_t ret = RF_RESULT_OK;

    return ret;
}

static bool_t sim_radio_int_cfg(rf_int_t state,
                                rf_int_reg_t int_cb,
                                uint8_t type)
{
    (type == INT_ENABLE) ? ((int_cb != NULL) ? (rf_int_reg[state] = int_cb) : (NULL)) : (NULL);

    return TRUE;
}

const struct radio_driver sim_radio_driver =
{
    sim_radio_init,

    sim_radio_prepare,
    sim_radio_transmit,

    sim_radio_send,
    sim_radio_recv,

    sim_radio_get_value,
    sim_radio_set_value,

    sim_radio_int_cfg,
};






























