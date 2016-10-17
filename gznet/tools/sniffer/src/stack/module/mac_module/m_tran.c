/**
 * @brief       : Transceiving control of RF
 *  -   1.不能加入tran_state.cb.tx_finish = NULL;以防txok回调函数中发送数据
 *  -   2.默认ACK帧不需要回复，否则RXOK事件处理需要修改
 * @file        : m_tran.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <mac_module.h>
#include <stdlib.h>
#include <phy_cca.h>
#include <phy_packet.h>
#include <phy_state.h>
#include <hal.h>
#include "dev.h"

/* 以下宏定义为临时定义，调试完毕需从PIB通过接口获取 */
#define  BE_INIT                1
#define  MAX_BE                 4
#define  MAX_CSMA_BACKOFFS      3

#define  PRE_SYNC_CODE_LEN      (7u)

#if DATA_RATE == 250u
/* 250K:32us/B; 4M:2us/B; 50us处理时延 */
#define  LEN_TO_US(len)         (((len+PRE_SYNC_CODE_LEN)*(32+2)) + 50)
/* 250K速率，20*(8*4)=640 增加160的抖动时间（经验值）*/
#define  BE_TO_US(be)           ((be)*800u)
/* 250K速率，等待ACK的时间 */
#define  MS_FOR_ACK             (3.5)
#endif

#if DATA_RATE == 100u
#define  LEN_TO_US(len)         (((len+PRE_SYNC_CODE_LEN)*(80+2)) + 100u)
/* 100K速率，50*(8*4)=1600 增加160的抖动时间（经验值）*/
#define  BE_TO_US(be)           ((be)*1760u)
#define  MS_FOR_ACK             (7u)
#endif

#if DATA_RATE == 10u
#define  LEN_TO_US(len)         (((len+PRE_SYNC_CODE_LEN)*(800+2)) + 50u)
/* 10K速率，500*(8*4)=1600 增加160的抖动时间（经验值）*/
#define  BE_TO_US(be)           ((be)*16160)
#define  BE_TO_MS(be)           ((be)*16)
#define  MS_FOR_ACK             (30u)
#endif

typedef struct _tran_state_t
{
    bool_t can_send;            // 用于标志本模块是否正在等待ACK或重发
    sbuf_t *sbuf;               // 用于保存等待ACK或重传的sbuf
    volatile bool_t ack_sent;   // 当前ack发送成功
    bool_t ack_needed;          // 当前帧需要ack
    bool_t ack_received;        // 接收到ack
    tran_cfg_t cb;              // 保存回调函数
    uint8_t need_resend_times;  // 本帧需要传输模块重传的次数
    bool_t  ack_timeout_happened; // 记录超时消息是否已经被处理，接收到ACK时使用
    bool_t  tx_und_happend;     //记录下溢是否已经处理，在TXOK处理中判断
} tran_state_t;


typedef struct _csma_state
{
    uint8_t nb;
    uint8_t be;
    uint32_t backoff_time;
} csma_state_t;

static hal_timer_t *csma_timer          = NULL;
static hal_timer_t *ack_timer           = NULL;
static hal_timer_t *resend_timer        = NULL;

#if TXOK_INT_SIMU_EN > 0
static hal_timer_t *txok_timer          = NULL;
#endif

static tran_state_t tran_state;
static csma_state_t csma_state;

static uint8_t already_send_times = 0;
static bool_t  rx_sfd_happed = FALSE; //!< 定义RX_SFD是否已经产生，在RXSFD中处理

/* 以下为调试用变量, 待删除*/
static uint32_t tx_und_cnt = 0;
static uint32_t ack_timeout_cnt = 0;

#if M_TRAN_DGB_EN > 0
volatile tran_tracker_t m_tran_tracker;
volatile tran_recv_tracker_t m_tran_recv_tracker;
#endif

static void tran_send_intra(void);
static void tran_send(pbuf_t *pbuf);

PROCESS_NAME(tran_event_process);

void tran_rx_sfd_set(bool_t res)
{
    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    rx_sfd_happed = res;
    HAL_EXIT_CRITICAL(s);
}

bool_t tran_rx_sfd_get(void)
{
    return rx_sfd_happed;
}

/*the function uses to send message to mac itself for calling back*/
static void tran_cb_send_msg(osel_signal_t sig)
{
    osel_event_t event;
    event.sig = sig;
    event.param = (osel_param_t)NULL;
    osel_post(NULL, &tran_event_process, &event);
}

/*call back for receiving Successfully*/
static void rx_ok_cb(uint16_t int_time)
{  
    extern volatile uint32_t current_time_stamp;
    hal_time_t now = hal_timer_now();

    current_time_stamp = now.w; 
    
    tran_cb_send_msg(M_TRAN_RXOK_EVENT);
}

/*call back for receiving Successfully*/
#if RF_INT_DEAL_FLOW > 0u
static void rx_ovr_cb(uint16_t int_time)
{
    tran_cb_send_msg(M_TRAN_RXOVR_EVENT);
}
#endif

/*call back for sending Successfully*/
static void tx_ok_cb(uint16_t int_time)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.txok_cb++;
    if (tran_state.can_send)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
#endif

#if TXOK_INT_SIMU_EN > 0
    //DBG_ASSERT(txok_timer != NULL __DBG_LINE);
    if (txok_timer != NULL)             // 确保该timer未被free
    {
        hal_timer_cancel(&txok_timer);
        tran_cb_send_msg(M_TRAN_TXOK_EVENT, MSG_LOW_PRIO);
    }
#else
    tran_cb_send_msg(M_TRAN_TXOK_EVENT);
#endif

#if M_TRAN_DGB_EN > 0
    m_tran_recv_tracker.txok_int_cb_tick = hal_timer_now().w;
#endif
}


static void tx_und_cb(uint16_t int_time)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.tx_und_cb++;
#endif

#if TXOK_INT_SIMU_EN > 0
    if (txok_timer != NULL)             // 确保该timer未被free
    {
        hal_timer_cancel(&txok_timer);
    }
#endif

    if (ack_timer != NULL)
    {
        hal_timer_cancel(&ack_timer);
    }
    tran_cb_send_msg(M_TRAN_TXUND_EVENT);

}

#if TXOK_INT_SIMU_EN > 0
/*call back for timeout of waiting tx ok*/
static void tx_ok_error_timer(void *arg)
{
    txok_timer = NULL;
    if (ack_timer != NULL)
    {
        hal_timer_cancel(&ack_timer);
    }
    tran_cb_send_msg(M_TRAN_TX_ERROR_EVENT, MSG_LOW_PRIO);
}
#endif

/*call back for timeout of waiting ack*/
static void ack_timeout_cb(void *arg)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.ack_tout_cb++;
#endif
    ack_timer = NULL;

    if (tran_state.ack_received == FALSE)
    {
        // 此时ACK有可能真在解析过程中,故消息处理需判断是否ACK已收到
        tran_cb_send_msg(M_TRAN_ACK_TIMEOUT_EVENT);
    }
    else
    {
        ;                                                   // 已经收到ACK
    }
}


/*create a random number between min and max*/
static uint8_t random(uint8_t min, uint8_t max)         //添加stdlib.h 头文件
{
    DBG_ASSERT(max >= min __DBG_LINE);
    srand(TA0R);
    return (min + rand() % (max - min + 1));
}

/*judge whether the channel is idle */
static bool_t channel_is_idle(void)
{
    return phy_cca();
}

/*call back for the time of tick of csma*/
static void csma_tick_cb(void *arg)
{
    csma_timer = NULL;
    tran_cb_send_msg(M_TRAN_CSMA_TIMEOUT_EVENT);
}

/*set a random time for backing off of csma*/
static void csma_backoff_tick(uint32_t time_us)
{
    if (csma_timer == NULL)
    {
        HAL_TIMER_SET_REL(US_TO_TICK(time_us - 80), csma_tick_cb, NULL, csma_timer);
        DBG_ASSERT(csma_timer != NULL __DBG_LINE);
    }
}

static void csma_fail_deal(void)
{
    m_tran_stop();

    DBG_ASSERT(tran_state.sbuf != NULL __DBG_LINE);
    if (tran_state.sbuf != NULL)
    {
        tran_send_intra();
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}

/*the function  uses to accomplish Features of csma*/
static void csma_backoff(void)
{
    /* 取得 2^be-1 到 2^MAX_BE-1之间的随机数 */
    uint8_t min = (1 << csma_state.be) - 1;
    uint8_t max = (1 << MAX_BE) - 1;
    uint8_t rand_be = random(min, max);
    uint32_t current_backoff_need_us = BE_TO_US(rand_be);

    if (
        (csma_state.backoff_time < current_backoff_need_us) ||
        (csma_state.nb >= MAX_CSMA_BACKOFFS)
    )
    {
        csma_fail_deal();
    }
    else
    {
        csma_state.backoff_time -= current_backoff_need_us;
        csma_backoff_tick(current_backoff_need_us);
    }

}

/*the part of Logic about the back_off*/
static void tran_operate_cca(void)
{
    if (channel_is_idle())
    {
        tran_send(tran_state.sbuf->primargs.pbuf);
    }
    else
    {
        csma_state.nb++;
        csma_state.be = ((csma_state.be + 1) < MAX_BE) ? (csma_state.be + 1) : MAX_BE;

        csma_backoff();
    }
}

/*Initialization of csma*/
static void tran_csma_start(uint32_t backoff_max_time_us)
{
    csma_state.nb = 0;
    csma_state.be = BE_INIT;
    csma_state.backoff_time = backoff_max_time_us;
    csma_backoff();
}

/*transmit by csma*/
static void tran_csma_send(pbuf_t *pbuf)
{
    uint8_t frm_len = 0;
    uint32_t send_need_time = 0;
    uint32_t slot_remain_time = 0;

    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    if (pbuf != NULL)
    {
        frm_len =  pbuf->data_len;
        DBG_ASSERT(frm_len != 0 __DBG_LINE);

#if M_SLOT_EN > 0
        send_need_time = LEN_TO_US(frm_len);
        slot_remain_time = TICK_TO_US(m_slot_get_remain_time());
#else
        send_need_time = 0;
        slot_remain_time = 0xFFFFFFFF;
#endif
        if (slot_remain_time > send_need_time)
        {
            tran_csma_start(slot_remain_time - send_need_time); // 开始退避
        }
        else
        {
            csma_fail_deal();
        }
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}

static void tran_resend_event(void)
{
    pbuf_t *pbuf = tran_state.sbuf->primargs.pbuf;
    if (pbuf->attri.send_mode == CSMA_SEND_MODE)
    {
        tran_csma_send(pbuf);
    }
    else
    {
        tran_send(pbuf);
    }
}

/*the founction uses to wait ack*/
static void tran_wait_ack(void)
{
    tran_state.ack_received = FALSE;
    tran_state.ack_timeout_happened = FALSE;

    if (ack_timer == NULL)
    {
        m_tran_recv();

        HAL_TIMER_SET_REL(MS_TO_TICK(MS_FOR_ACK), ack_timeout_cb, NULL, ack_timer);
        DBG_ASSERT(ack_timer != NULL __DBG_LINE);

#if M_TRAN_DGB_EN > 0
        m_tran_tracker.waitting_ack++;
#endif
    }
    else
    {
        /* a very strange state*/
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}

//tran_send失败
static void tran_send_failed(void)
{
    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    tran_state.ack_needed = FALSE;
    tran_state.can_send = TRUE;
    sbuf_t *sbuf_tmp = tran_state.sbuf;
    tran_state.sbuf = NULL;
    HAL_EXIT_CRITICAL(s);

    phy_set_state(PHY_SLEEP_STATE);

    DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
    tran_state.cb.tx_finish(sbuf_tmp, FAILE);
}

static void tran_send(pbuf_t *pbuf)
{
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

#if M_SLOT_EN > 0
    uint32_t send_need_time = LEN_TO_US(pbuf->data_len);
    uint32_t slot_remain_time = TICK_TO_US(m_slot_get_remain_time());
#else
    uint32_t send_need_time = 0;
    uint32_t slot_remain_time = 0xFFFFFFFF;
#endif
    if ((pbuf != NULL) && (slot_remain_time > send_need_time))
    {
        uint8_t stamp_size = 0;
#if M_SYNC_EN > 0
        stamp_size = m_sync_txfilter(pbuf);
#endif
        if (!phy_write_buf(pbuf, stamp_size))
        {
#if M_TRAN_DGB_EN > 0
            static uint32_t tx_write_fifo_fail_cnt = 0;
            tx_write_fifo_fail_cnt++;
#endif
            tran_send_failed();

            return;
        }

        tran_state.tx_und_happend = FALSE;

        if (!phy_send_data())
        {
            tran_send_failed();
            return;
        }

#if M_TRAN_DGB_EN > 0
        m_tran_tracker.data_send_real++;
#endif

#if TXOK_INT_SIMU_EN > 0
        HAL_TIMER_SET_REL(US_TO_TICK(LEN_TO_US(pbuf->data_len) + 10000),
                          tx_ok_error_timer,
                          NULL,
                          txok_timer);
        DBG_ASSERT(txok_timer != NULL __DBG_LINE);
#endif

        if (pbuf->attri.need_ack == TRUE)
        {
            tran_state.ack_needed = TRUE;
        }
    }
    else
    {
        hal_int_state_t s;
        HAL_ENTER_CRITICAL(s);
        if (ack_timer != NULL)
        {
            hal_timer_cancel(&ack_timer);
        }
        tran_send_failed();
        HAL_EXIT_CRITICAL(s);
    }
}

/*return state of can_send*/
bool_t m_tran_can_send(void)
{
    return (tran_state.can_send == TRUE);
}


static void resend_timeout_cb(void *p)
{
    resend_timer = NULL;
    
    osel_event_t event;
    event.sig = M_TRAN_RESEND_TIMEOUT_EVENT;
    event.param = (osel_param_t)NULL;
    osel_post(NULL, &tran_event_process, &event);
}

static void tran_send_intra(void)
{
    pbuf_t *pbuf = tran_state.sbuf->primargs.pbuf;
    uint32_t tran_resend_times = 0;

    if (++already_send_times <= tran_state.need_resend_times)
    {
        tran_state.can_send = FALSE;                    // 标志进入发送数据阶段
        /* 重传退避，如果重传时间大于0，就退避，否则直接进入CSMA */
        tran_resend_times = TRAN_RESEND_TIMES * (already_send_times - 1) * (already_send_times - 1);
        if (tran_resend_times > 0)
        {
            if (resend_timer == NULL)
            {
                HAL_TIMER_SET_REL(MS_TO_TICK(tran_resend_times),
                                  resend_timeout_cb,
                                  pbuf,
                                  resend_timer);
                DBG_ASSERT(resend_timer != NULL __DBG_LINE);
            }
        }
        else
        {
            if (pbuf->attri.send_mode == CSMA_SEND_MODE)
            {
                tran_csma_send(pbuf);
            }
            else
            {
                tran_send(pbuf);
            }
        }
    }
    else
    {
        hal_int_state_t s;
        HAL_ENTER_CRITICAL(s);
        if (ack_timer != NULL)
        {
            hal_timer_cancel(&ack_timer);
        }

        tran_state.can_send = TRUE;
        tran_state.ack_needed = FALSE;
        sbuf_t *sbuf_tmp = tran_state.sbuf;
        tran_state.sbuf = NULL;
        HAL_EXIT_CRITICAL(s);
        phy_set_state(PHY_SLEEP_STATE);

#if M_TRAN_DGB_EN > 0
        m_tran_tracker.tx_finish_fail++;
#endif
        DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
        tran_state.cb.tx_finish(sbuf_tmp, FAILE);
    }
}

/*send data about considering ACK and late send completely*/
void m_tran_send(sbuf_t *const sbuf, tran_tx_finish_cb_t tx_finish, uint8_t resend_times)
{
    DBG_ASSERT(sbuf != NULL __DBG_LINE);
    DBG_ASSERT(sbuf->primargs.pbuf != NULL __DBG_LINE);
    DBG_ASSERT(tx_finish != NULL __DBG_LINE);

    pbuf_t *pbuf = NULL;

    /* 如果当前正在等待ACK或重发则不接受新数据 */
    if (tran_state.can_send == FALSE)
    {
        asm("NOP");
        tx_finish(sbuf, FALSE);
        return;
    }

    m_tran_stop();

    if (tx_finish != NULL)
    {
        tran_state.cb.tx_finish = tx_finish;
    }
    else
    {
        return;
    }

    if (sbuf != NULL)
    {
        pbuf = sbuf->primargs.pbuf;
        if (pbuf != NULL)
        {
            tran_state.sbuf = sbuf;
            tran_state.ack_received   = FALSE;
            tran_state.need_resend_times  = resend_times;
            already_send_times = 0;

#if M_TRAN_DGB_EN > 0
            osel_memset((uint8_t *)&m_tran_tracker, 0, sizeof(m_tran_tracker));
            m_tran_tracker.data_send_start++;
#endif
            tran_send_intra();
        }
        else
        {
            hal_int_state_t s;
            HAL_ENTER_CRITICAL(s);

            tran_state.ack_needed   = FALSE;
            tran_state.can_send     = TRUE;
            sbuf_t *sbuf_tmp        = tran_state.sbuf;
            tran_state.sbuf         = NULL;
            HAL_EXIT_CRITICAL(s);
            phy_set_state(PHY_SLEEP_STATE);

            DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
            tran_state.cb.tx_finish(sbuf_tmp, FAILE);
        }
    }
}

/*recevice data */
void m_tran_recv(void)
{
    /* 如果当前正在发送帧 */
    if (phy_get_state() == PHY_TX_STATE)
    {
        phy_set_state(PHY_IDLE_STATE);
        if (tran_state.sbuf != NULL)
        {
            hal_int_state_t s;
            HAL_ENTER_CRITICAL(s);
            if (ack_timer != NULL)
            {
                hal_timer_cancel(&ack_timer);
            }

            tran_state.can_send = TRUE;
            tran_state.ack_needed = FALSE;
            sbuf_t *sbuf_tmp = tran_state.sbuf;
            tran_state.sbuf = NULL;
            HAL_EXIT_CRITICAL(s);

            DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
            tran_state.cb.tx_finish(sbuf_tmp, FAILE);

            sbuf_tmp = NULL;
        }
    }
    
    tran_rx_sfd_set(FALSE);
    phy_set_state(PHY_RX_STATE);

#if M_TRAN_DGB_EN > 0
    osel_memset((void *)&m_tran_recv_tracker, 0, sizeof(m_tran_recv_tracker));
#endif
}

void m_tran_stop(void)
{
    phy_set_state(PHY_IDLE_STATE);
}

void m_tran_sleep(void)
{
    /* 如果当前正在发送帧 */
    if (phy_get_state() == PHY_TX_STATE)
    {
        phy_set_state(PHY_IDLE_STATE);
        if (tran_state.sbuf != NULL)
        {
            hal_int_state_t s;
            HAL_ENTER_CRITICAL(s);
            if (ack_timer != NULL)
            {
                hal_timer_cancel(&ack_timer);
            }

            tran_state.can_send = TRUE;
            tran_state.ack_needed = FALSE;
            sbuf_t *sbuf_tmp = tran_state.sbuf;
            tran_state.sbuf = NULL;
            HAL_EXIT_CRITICAL(s);

            DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
            tran_state.cb.tx_finish(sbuf_tmp, FAILE);

            sbuf_tmp = NULL;
        }
    }
    
    phy_set_state(PHY_SLEEP_STATE);
}

/*function uses to deal event when RX interrupt come*/
static void tran_deal_rxok_event(void)
{
    sbuf_t *sbuf_tmp = NULL;
    bool_t is_valid_frm = FALSE;

    pbuf_t *pbuf = tran_state.cb.frm_get();

#if M_TRAN_DGB_EN > 0
    m_tran_recv_tracker.rxok_msg_deal_tick = hal_timer_now().w;
#endif

    if ((pbuf == NULL) || (pbuf->data_len == 0))
    {
        if (pbuf != NULL)
        {
            pbuf_free(&pbuf __PLINE2);
        }
        return;
    }

    is_valid_frm = tran_state.cb.frm_head_parse(pbuf);

    if (is_valid_frm == TRUE)
    {
        /* 等待ACK时收到其他有效帧，释放该帧，继续等待 */
        if ((tran_state.ack_needed == TRUE) && (pbuf->attri.is_ack == FALSE))
        {
            pbuf_free(&pbuf __PLINE2);
            return;
        }

#if M_TRAN_DGB_EN > 0
        m_tran_tracker.recv_data++;
#endif

#if M_SYNC_EN > 0
        m_sync_rxfilter(pbuf);
#endif

        if (pbuf->attri.need_ack == TRUE)
        {
            tran_state.cb.send_ack(pbuf->attri.seq);//发送结束为IDLE状态
        }
        else if (pbuf->attri.is_ack == TRUE)
        {
#if M_TRAN_DGB_EN > 0
            m_tran_tracker.recv_ack++;
#endif
            if (tran_state.ack_needed == TRUE)              //状态为正在等待ACK
            {
                if (tran_state.sbuf->primargs.pbuf->attri.seq == pbuf->attri.seq)
                {
                    hal_int_state_t s;
                    HAL_ENTER_CRITICAL(s);

                    if (ack_timer != NULL)
                    {
                        hal_timer_cancel(&ack_timer);
                    }
                    else // 在处理ack期间等待ACK定时器已然触发，超时消息已发出
                    {
                        if (tran_state.ack_timeout_happened)
                        {
                            //重传数据已经发出,TXOK消息已经发出，交给后面的TXOK消息处理
                            tran_state.ack_timeout_happened = FALSE;
                            ack_timeout_cnt++;
                            HAL_EXIT_CRITICAL(s);
                            return;
                        }
                        else
                        {
                            ; // 超时消息处理在排队，本流程继续处理
                        }
                    }

                    tran_state.ack_needed = FALSE;
                    tran_state.can_send = TRUE;
                    tran_state.ack_received = TRUE;
                    sbuf_tmp = tran_state.sbuf;
                    tran_state.sbuf = NULL;
                    HAL_EXIT_CRITICAL(s);

#if M_TRAN_DGB_EN > 0
                    m_tran_tracker.tx_success_ack++;
#endif
                    DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
                    tran_state.cb.tx_finish(sbuf_tmp, SUCCESS);
                }
                else
                {
                    ;   //收到错误的ACK
                }
            }
            else
            {
                ;//不处理不在等待ACK期间收到的ACK
            }
        }

        tran_state.cb.frm_payload_parse(pbuf);
    }
    else
    {// is valid == FALSE
        if(pbuf->used == TRUE)
        {
            pbuf_free(&pbuf __PLINE2);
        }
        phy_set_state(PHY_RX_STATE);
    }
}

/*function uses to deal event when RX interrupt come*/
static void tran_deal_rxovr_event(void)
{
//    hal_rf_flush_rxfifo();
    SSN_RADIO.set_value(RF_RXFIFO_FLUSH, 0);
    phy_set_state(PHY_SLEEP_STATE);        //PHY_RX_STATE
}

/*function uses to deal tx event when the interrupt of tx ok */
static void tran_deal_txok_event(void)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.txok_msg_deal++;
#endif

    /* 当无法在驱动中断中给出下溢处理判断时，在txok消息处理中判断*/
#if RF_INT_DEAL_FLOW == 0u
    if (hal_rf_txfifo_underflow())
    {
        tx_und_cb(0);
        DBG_PRINT_INFO(0x47);
        return;
    }
#endif

    if (!tran_state.tx_und_happend)
    {
        if (tran_state.ack_needed == FALSE)
        {
            if (tran_state.can_send == FALSE)  /* txok event haven't been dealed */
            {
                hal_int_state_t s;
                HAL_ENTER_CRITICAL(s);
                sbuf_t *sbuf_tmp = tran_state.sbuf;
                tran_state.sbuf = NULL;
                tran_state.can_send = TRUE;
                HAL_EXIT_CRITICAL(s);

                phy_set_state(PHY_SLEEP_STATE);
                DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
                tran_state.cb.tx_finish(sbuf_tmp, SUCCESS);
#if M_TRAN_DGB_EN > 0
                m_tran_tracker.tx_success_without_ack++;
#endif
            }
            else
            {
                // ACK定时器超时-超时消息响应之前收到ACK，ACK等待处理，先进入重传流程
                // 重传数据发送成功后,发送txok消息,再处理之前的ACK消息，变量复位
                // 接着处理重传的txok消息，变量异步，会进入这里。已在RXOK事件中处理。
                DBG_ASSERT(FALSE __DBG_LINE);
            }
        }
        else
        {
            tran_wait_ack();
        }
    }
    else
    {
        tx_und_cnt++;
        return;//下溢中断已经发生并处理过，不操作
    }

#if M_TRAN_DGB_EN > 0
    m_tran_recv_tracker.txok_msg_deal_tick = hal_timer_now().w;
#endif
}

#if TXOK_INT_SIMU_EN > 0
static uint32_t simu_int_cnt = 0;
/*function uses to deal tx event when the interrupt of tx ok */
static void tran_deal_tx_error_event(void)
{
    simu_int_cnt++;
    if (tran_state.can_send == FALSE)  /* txok event haven't been dealed */
    {
        hal_int_state_t s;
        HAL_ENTER_CRITICAL(s);

        sbuf_t *sbuf_tmp = tran_state.sbuf;
        tran_state.sbuf = NULL;
        tran_state.can_send = TRUE;

        HAL_EXIT_CRITICAL(s);

        extern void rf_enter_sleep_force(void);
        rf_enter_sleep_force();

        DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
        tran_state.cb.tx_finish(sbuf_tmp, FAILE);
    }
    else
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}
#endif


static void tran_deal_tx_und_event(void)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.tx_und_msg_deal++;
#endif

    if (tran_state.can_send == FALSE)  /* txok event haven't been dealed */
    {
        hal_int_state_t s;
        HAL_ENTER_CRITICAL(s);
        tran_state.tx_und_happend = TRUE;

        sbuf_t *sbuf_tmp = tran_state.sbuf;
        tran_state.sbuf = NULL;
        tran_state.can_send = TRUE;

        HAL_EXIT_CRITICAL(s);

        phy_set_state(PHY_SLEEP_STATE);

        DBG_ASSERT(tran_state.cb.tx_finish != NULL __DBG_LINE);
        tran_state.cb.tx_finish(sbuf_tmp, FAILE);
    }
    else
    {
#if TXOK_INT_SIMU_EN > 0
        ; //模拟TXOK中断已经处理过下溢
#else
        DBG_ASSERT(FALSE __DBG_LINE);
#endif
    }
}

/*deal event when waiting ack timeout*/
static void tran_deal_ack_timeout_event(void)
{
#if M_TRAN_DGB_EN > 0
    m_tran_tracker.ack_tout_msg_deal++;
#endif
    if (tran_state.ack_received == FALSE)
    {
        m_tran_stop();
        // 在停止接收状态之前，有可能接收到ACK，消息已发出但尚未处理
        // 在RXOK中处理该情况
        tran_state.ack_timeout_happened = TRUE;
        DBG_ASSERT(tran_state.sbuf != NULL __DBG_LINE);
        if (tran_state.sbuf != NULL)
        {
#if M_TRAN_DGB_EN > 0
            m_tran_tracker.tx_resend++;
#endif
            tran_send_intra();
        }
        else
        {
            DBG_ASSERT(FALSE __DBG_LINE);
        }
    }
    else
    {
        // 解析ACK数据期间超时，ACK已经成功接收到
#if M_TRAN_DGB_EN > 0
        m_tran_tracker.ack_tout_recved_ack++;
#endif
        ;
    }
}

PROCESS(tran_event_process, "tran_event_process");
PROCESS_THREAD(tran_event_process, ev, data)
{
    PROCESS_BEGIN();
    
    while(1)
    {   
        if(ev == M_TRAN_RESEND_TIMEOUT_EVENT) {
            tran_resend_event();
        }
        else if(ev == M_TRAN_CSMA_TIMEOUT_EVENT) {
            tran_operate_cca();
        }
        else if(ev == M_TRAN_RXOK_EVENT) {
            tran_deal_rxok_event();
        }
        else if(ev == M_TRAN_RXOVR_EVENT) {
            tran_deal_rxovr_event();
        }
        else if(ev == M_TRAN_TXOK_EVENT) {
            tran_deal_txok_event();
        }
#if TXOK_INT_SIMU_EN > 0
        else if(ev == M_TRAN_TX_ERROR_EVENT) {
            tran_deal_tx_error_event();
        }
#endif
        else if(ev == M_TRAN_ACK_TIMEOUT_EVENT) {
            tran_deal_ack_timeout_event();
        }
        else if(ev == M_TRAN_TXUND_EVENT) {
            tran_deal_tx_und_event();
        }

        PROCESS_YIELD();
    }

    PROCESS_END();
}


void m_tran_cfg(const tran_cfg_t *const tran_cb_reg)
{
    /* 注册传输模块的回调函数 */
    DBG_ASSERT(tran_cb_reg != NULL __DBG_LINE);
    DBG_ASSERT(tran_cb_reg->frm_head_parse != NULL __DBG_LINE);
    DBG_ASSERT(tran_cb_reg->frm_payload_parse != NULL __DBG_LINE);
    DBG_ASSERT(tran_cb_reg->frm_get != NULL __DBG_LINE);
    DBG_ASSERT(tran_cb_reg->send_ack != NULL __DBG_LINE);

    tran_state.cb.frm_get           = tran_cb_reg->frm_get;
    tran_state.cb.frm_head_parse    = tran_cb_reg->frm_head_parse;
    tran_state.cb.frm_payload_parse = tran_cb_reg->frm_payload_parse;
    tran_state.cb.send_ack          = tran_cb_reg->send_ack;
}

void m_tran_init(osel_task_t *task)
{    
    osel_pthread_create(task, &tran_event_process, NULL);

    /* 注册各种中断回调函数, 使能各种中断 */
    
    SSN_RADIO.int_cfg(RX_OK_INT, rx_ok_cb, INT_ENABLE);
    SSN_RADIO.int_cfg(TX_OK_INT, tx_ok_cb, INT_ENABLE);

#if RF_INT_DEAL_FLOW > 0u
    
    SSN_RADIO.int_cfg(TX_UND_INT, tx_und_cb, INT_ENABLE);
    SSN_RADIO.int_cfg(RX_OVR_INT, rx_ovr_cb, INT_ENABLE);
#endif

    phy_set_state(PHY_SLEEP_STATE);

    /* 传输模块的初始状态 */
    tran_state.can_send     = TRUE;
    tran_state.sbuf         = NULL;
    tran_state.ack_needed   = FALSE;
    tran_state.ack_received = FALSE;
}

