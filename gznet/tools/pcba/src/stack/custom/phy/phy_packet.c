/**
 * @brief       : 
 *
 * @file        : phy_packet.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <hal.h>
#include <pbuf.h>
#include <phy_packet.h>
#include <phy_state.h>
#include <osel_arch.h>
#include <dev.h>

pbuf_t *phy_get_packet(void)
{
#if RF_INT_DEAL_FLOW == 0u
    if (hal_rf_rxfifo_overflow())
    {
        hal_rf_flush_rxfifo();
        phy_set_state(PHY_RX_STATE);
        DBG_PRINT_INFO(0x45);
        return NULL;
    }
#endif

    /* FIFO包括RSSI和LQI，不算入有效帧长 */
    static int8_t frm_len = 0;
    SSN_RADIO.get_value(RF_RXFIFO_CNT, &frm_len);
    frm_len = frm_len - (int8_t)PHY_FCS_SIZE;

    /* 地址校验失败、帧长过滤失败处理*/
    if (frm_len < (int8_t)PKT_LEN_MIN)
    {
        phy_set_state(PHY_RX_STATE);
        return NULL;
    }

    if (frm_len > (int8_t)PKT_LEN_MAX)
    {
        phy_set_state(PHY_RX_STATE);
        return NULL;
    }

    frm_len += PHY_FCS_SIZE;

    pbuf_t *frm_buf = pbuf_alloc(frm_len __PLINE1);
    if (frm_buf == NULL)
    {
        SSN_RADIO.set_value(RF_RXFIFO_FLUSH, 0);
        phy_set_state(PHY_RX_STATE);
        return NULL;
    }

    SSN_RADIO.recv(frm_buf->head, PHY_LEN_FEILD_SIZE);
    /* FIFO中字节数需与物理帧长度域数字匹配 */
    if (*(frm_buf->head) + PHY_LEN_FEILD_SIZE == frm_len - PHY_FCS_SIZE)
    {
        SSN_RADIO.recv(frm_buf->head + PHY_LEN_FEILD_SIZE, 
                       frm_len - PHY_LEN_FEILD_SIZE);

        frm_buf->data_len = frm_len;
        frm_buf->data_p = frm_buf->head + frm_buf->data_len;
        // rf已经把rssi值换算过
        SSN_RADIO.get_value(RF_RXRSSI, &frm_buf->attri.rssi_dbm);
    }
    else
    {
        pbuf_free(&frm_buf __PLINE2);
        phy_set_state(PHY_RX_STATE);
    }

    return frm_buf;
}


bool_t phy_write_buf(pbuf_t *pbuf, uint8_t stamp_size)
{
    DBG_ASSERT(pbuf != NULL __DBG_LINE);
    if (pbuf != NULL)
    {
        pbuf->data_len += PHY_HEAD_SIZE;
        DBG_ASSERT((pbuf->data_len >= PKT_LEN_MIN) && (pbuf->data_len <= PKT_LEN_MAX) __DBG_LINE);

        pbuf->head[0] = pbuf->data_len - PHY_LEN_FEILD_SIZE + stamp_size;
        osel_memcpy(&pbuf->head[1], &pbuf->attri.dst_id, PHY_HEAD_SIZE - 1);

        SSN_RADIO.set_value(RF_TXFIFO_FLUSH, 0);
        //  len， addr
        SSN_RADIO.set_value(RF_TXFIFO_CNT, pbuf->data_len + stamp_size);
        SSN_RADIO.set_value(RF_DADDR3, LO_UINT16(pbuf->attri.dst_id));
        SSN_RADIO.set_value(RF_DADDR2, HI_UINT16(pbuf->attri.dst_id));

        if (!SSN_RADIO.prepare(pbuf->head,  pbuf->data_len + stamp_size))
        {
            return FALSE;
        }
        /* 这里需要还原到MAC的长度，因为发送失败以后重传，长度会累积往上加 */
        pbuf->data_len -= PHY_HEAD_SIZE;
        return TRUE;
    }
    return FALSE;
}


bool_t phy_send_data(void)
{
    // return (phy_set_state(PHY_TX_STATE));
    
    SSN_RADIO.transmit(0);
    
    return TRUE;
}
