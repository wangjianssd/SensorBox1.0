/**
 * @brief       : 
 *
 * @file        : m_sync.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#define SYNC_DBG                0u

#include <mac_module.h>
#include <driver.h>
#include "dev.h"
#include <portablelonglong.h>

#if SYNC_DBG
static void sync_test(void);
#endif

PROCESS_NAME(sync_event_process);

enum
{
    ENTRY_VALID_LIMIT    = 1,   // number of entries to become synchronized
    ENTRY_THROWOUT_LIMIT = 30,   // MICA and MICA2DOT have slower CPU
    MAX_ENTRIES          = 8,   // number of entries in the table
};

typedef struct
{
    list_head_t list;
    uint32_t    x;
    int32_t     y;
    uint64_t    x2;
    int64_t     xy;
} sync_entry_t;

static list_head_t  free_list;
static list_head_t  sync_points;
static int64_t      Exy;
static int64_t      Ey;
static uint64_t     Ex;
static uint64_t     Ex2;
static fp32_t       skew;
static uint32_t     local_avg;
static uint32_t     local_base;
static int32_t      offset_base;
static int32_t      offset_avg;
static uint8_t      num_sync_entries;
static sync_entry_t sync_entries[MAX_ENTRIES + 1];
static sync_entry_t *oldentry;
static sync_entry_t *newentry;

static volatile hal_time_t  rxsfdtime;
static sync_cfg_t   sync_cfg;
static hal_time_t   sync_txt;
static hal_time_t   sync_rxt;

static bool_t sync_enable = FALSE; //

static volatile hal_time_t sync_global_offset;
static uint8_t FaultPoint;
static bool_t FirstSync;

// 添加最大最小偏移量的误差统计
#if SYNC_DBG > 0
static int32_t max_sync_positive_offset = 0;    // 正数最大偏移
static int32_t max_sync_negative_offset = 0;    // 负数最大偏移
#endif

static void m_sync_add(hal_time_t *txtime, hal_time_t *rxtime);
static void m_sync_sfd_isr(uint16_t time);
static void (*m_sfd_isr)(uint16_t);
static void rx_sfd(uint16_t time);
static void tx_sfd(uint16_t time);

static void sync_reset(void)
{
    list_init(&free_list);
    list_init(&sync_points);

    for (uint8_t i = 0; i < MAX_ENTRIES + 1; i++)
    {
        list_add_to_tail(&(sync_entries[i].list), &free_list);
    }

    sync_enable = FALSE;

    num_sync_entries = 0;
    FaultPoint = 0;
    FirstSync = TRUE;
    LL_SET(Exy, 0, 0);
    LL_SET(Ey, 0, 0);
    LL_SET(Ex, 0, 0);
    LL_SET(Ex2, 0, 0);
    skew = 0;
    local_avg = 0;
    offset_avg = 0;

    newentry = list_entry_decap(&free_list, sync_entry_t, list);

    SSN_RADIO.int_cfg(RX_SFD_INT, m_sync_sfd_isr, INT_ENABLE);
    SSN_RADIO.int_cfg(RX_SFD_INT, m_sync_sfd_isr, INT_ENABLE);

    m_sfd_isr = rx_sfd;
}


static void sync_cfg_check(const sync_cfg_t *const cfg)
{
    DBG_ASSERT(cfg != NULL __DBG_LINE);

    DBG_ASSERT(cfg->stamp_len == sizeof(hal_time_t) __DBG_LINE);

    if (cfg->tx_sfd_cap == TRUE)
    {
        /* 使用硬件TxSFD中断 */
        DBG_ASSERT(cfg->len_modfiy == TRUE __DBG_LINE);
    }
    else
    {
        /* 模拟TxSFD中断 */
        DBG_ASSERT(cfg->tx_offset != 0 __DBG_LINE);
    }
}



void m_sync_cfg(const sync_cfg_t *const cfg)
{
    DBG_ASSERT(cfg != NULL __DBG_LINE);

    sync_cfg_check(cfg);

    sync_cfg = *cfg;

    sync_enable = TRUE;
}


void m_sync_enable(bool_t enable)
{
#if SYN_CODE_COMPRESS_EN == 0
    sync_enable = enable;
    if (enable == FALSE)
    {
        sync_reset();   // 关闭同步模块时，复位同步模块初始配置
    }
#endif
}

static void sync_send_msg(void *arg)
{
#if SYN_CODE_COMPRESS_EN == 0
    osel_event_t event;
    event.sig = M_SYNC_BACKGROUND;
    event.param = (osel_param_t)arg;
    osel_post(NULL, &sync_event_process, &event);
#endif
}


static void rx_sfd(uint16_t time)
{
    tran_rx_sfd_set(TRUE);
#if SYN_CODE_COMPRESS_EN == 0
    rxsfdtime = hal_timer_now();
    if (rxsfdtime.s.hw < time)
    {
        /*捕获发生到中断处理中获取时间过程中，定时器翻转，取消在获取当前时间
        函数中所做的软件时间修正*/
        rxsfdtime.s.sw--;
    }
    rxsfdtime.s.hw = time;
#endif
}


static void tx_sfd(uint16_t time)
{
#if SYN_CODE_COMPRESS_EN == 0

#if M_TRAN_DGB_EN > 0
    extern volatile tran_tracker_t m_tran_tracker;
    m_tran_tracker.tx_sfd_cb++;
#endif

    if (sync_cfg.tx_sfd_cap == TRUE)
    {
        // 1 capture current local time
        hal_time_t now = hal_timer_now();

        now.s.hw = time;

        // 2 convert to global time if needed
        if (!sync_cfg.sync_source)
        {
            m_sync_l2g(&now);
        }

        // 3 send the stamp to current FIFO
        SSN_RADIO.prepare((uint8_t *)&now.w, sizeof(now.w));
#if M_TRAN_DGB_EN > 0
        m_tran_tracker.tx_sfd_ok++;
#endif
        m_sfd_isr = rx_sfd;
    }
#endif
}


uint8_t m_sync_txfilter(pbuf_t *tpac)
{
    hal_time_t now;

    if (tpac == NULL)
    {
        return 0;
    }

    //if it is ok to add timestamp to the packet
    if (sync_enable)
    {
        if ((*(tpac->head + sync_cfg.flag_byte_pos) & sync_cfg.flag_byte_msk)
                == sync_cfg.flag_byte_val)
        {
            if (sync_cfg.tx_sfd_cap)
            {
                //enable tx sfd cap
                m_sfd_isr = tx_sfd;
            }
            else
            {
                now = hal_timer_now();
                if (!sync_cfg.sync_source)
                {
                    m_sync_l2g(&now);
                }

                now.w += sync_cfg.tx_offset;
                if (sync_cfg.stamp_byte_pos != 0)
                {
                    // 时间戳在指定的字节起始处；
                    osel_memcpy(tpac->head + sync_cfg.stamp_byte_pos,
                                &now.w,
                                sync_cfg.stamp_len);
                }
                else
                {
                    // 时间戳在帧尾，这时，在后面需要修改帧头长度和data_len的长度;
                    // 请检查sync_cfg.len_modfiy = TRUE；
                    osel_memcpy(tpac->data_p, &now.w, sync_cfg.stamp_len);
                }
            }

            return sync_cfg.stamp_len;
        }
    }
    else
    {
        ; // wheather check the pbuf len and frame len
    }

    return 0;
}

void m_sync_rxfilter(pbuf_t *rpac)
{
#if SYN_CODE_COMPRESS_EN == 0

    hal_time_t now;

    if (rpac == NULL)
    {
        return;
    }
    if (sync_cfg.sync_source)
    {
        return;
    }

    if (sync_enable)
    {
        if ((uint16_t)rpac->attri.src_id != sync_cfg.sync_target)
        {
            return;
        }
        if ((*(rpac->head + sync_cfg.flag_byte_pos) & sync_cfg.flag_byte_msk) == sync_cfg.flag_byte_val)
        {
            //txt
            if (sync_cfg.stamp_byte_pos != 0)
            {
                osel_memcpy(&sync_txt, rpac->head + sync_cfg.stamp_byte_pos, sync_cfg.stamp_len);
                //  TODO: 去掉载荷里面的时间戳
            }
            else
            {
                osel_memcpy(&sync_txt, rpac->data_p - sync_cfg.stamp_len, sync_cfg.stamp_len);
                rpac->data_len -= sync_cfg.stamp_len;
            }

            //rxt
            if (sync_cfg.rx_sfd_cap)
            {
                sync_rxt.w = rxsfdtime.w;
            }
            else
            {
                now = hal_timer_now();
                now.w += sync_cfg.rx_offset;
                sync_rxt.w = now.w;
            }

            //calculate
            if (sync_cfg.background_compute)
            {
                sync_send_msg(NULL);
            }
            else
            {
                m_sync_add(&sync_txt, &sync_rxt);
            }
        }
    }

#endif
}

#if PRINT_SYNC_PARA_EN == 1
uint32_t txtime_x;
uint32_t rxtime_y;
fp32_t k;
uint32_t x0;
uint32_t y0;
bool_t need_printf = FALSE;
#endif

static void m_sync_add(hal_time_t *txtime, hal_time_t *rxtime)
{
#if PRINT_SYNC_PARA_EN == 1
    txtime_x = txtime->w;
    rxtime_y = rxtime->w;
    need_printf = TRUE;
    k = skew;
    x0 = offset_base + offset_avg;
    y0 = local_base + local_avg;
#endif
    
#if SYN_CODE_COMPRESS_EN == 0
    //check if consistent
    hal_time_t templt;
    int32_t temperr;
    uint64_t templl;
    int64_t stempll;
    sync_entry_t *pos;
    double a;
    double b;

    if (FirstSync)
    {
        FirstSync = FALSE;
        local_base = rxtime->w;
        offset_base = txtime->w - rxtime->w;
    }

    if (num_sync_entries > 3)
    {
        templt = *rxtime;
        m_sync_l2g(&templt);
        temperr = txtime->w - templt.w;

        if ((temperr > ENTRY_THROWOUT_LIMIT) || (temperr < (0 - ENTRY_THROWOUT_LIMIT)))
        {
            //Fault point occur
            FaultPoint++;

#if SYNC_DBG > 0
            if (temperr > ENTRY_THROWOUT_LIMIT) // 正数最大偏移
            {
                if (temperr > max_sync_positive_offset)
                {
                    max_sync_positive_offset = temperr;
                }
            }
            if (temperr < (0 - ENTRY_THROWOUT_LIMIT)) // 负数最大偏移
            {
                if (temperr < max_sync_negative_offset)
                {
                    max_sync_negative_offset = temperr;
                }
            }
#endif

            if (FaultPoint > 3)
            {
                //err sync 3times, start from beginning
                DBG_ASSERT(FALSE __DBG_LINE);
                FaultPoint = 0;
                return;
            }
            else
            {
                //Some sw timer error, skip it, return entry dropped
                return;
            }
        }
        else
        {
            //Good point
            FaultPoint = 0;
        }
    }

    //add to the table & update parameters  --29
    // -31
    if (num_sync_entries < MAX_ENTRIES)
    {
        newentry->x = rxtime->w - local_base;
        newentry->y = txtime->w - rxtime->w - offset_base;
        if ((newentry->y < -10000) || (newentry->y > 10000))
        {
            _NOP();
            //DBG_ASSERT(FALSE);
        }
        //newentry->x2 = (uint64_t)newentry->x * newentry->x;
        LL_UI2L(templl, newentry->x);
        LL_MUL(newentry->x2, templl, templl);
        //newentry->xy = (int64_t)newentry->y * newentry->x;
        LL_I2L(stempll, newentry->y);
        LL_MUL(newentry->xy, stempll, templl);

        //queue not full, get space from &free_list, and fill up the new entry
        num_sync_entries++;

        //then append the new entry to sync_points
        list_add_to_tail(&newentry->list, &sync_points);

        //now, update the parameters
        //Ex += newentry->x;
        LL_UI2L(templl, newentry->x);
        LL_ADD(Ex, Ex, templl);
        //Ey += newentry->y;
        LL_I2L(templl, newentry->y);
        LL_ADD(Ey, Ey, templl);
        //Exy += newentry->xy;
        LL_ADD(Exy, Exy, newentry->xy);
        //Ex2 += newentry->x2;
        LL_ADD(Ex2, Ex2, newentry->x2);

        newentry = list_entry_decap(&free_list, sync_entry_t, list);

        switch (num_sync_entries)
        {
        case 1:
            //local_avg = Ex;
            LL_L2UI(local_avg, Ex);
            //offset_avg = Ey;
            LL_L2I(offset_avg, Ey);
            skew = 0;
            return;

        case 2:
            //local_avg = Ex>>1;
            LL_USHR(templl, Ex, 1);
            LL_L2UI(local_avg, templl);
            //offset_avg = Ey>>1;
            LL_SHR(templl, Ey, 1);
            LL_L2I(offset_avg, templl);
            break;

        case 4:
            //local_avg = Ex>>2;
            LL_USHR(templl, Ex, 2);
            LL_L2UI(local_avg, templl);
            //offset_avg = Ey>>2;
            LL_SHR(templl, Ey, 2);
            LL_L2I(offset_avg, templl);
            break;

        case 8:
            //local_avg = Ex>>3;
            LL_USHR(templl, Ex, 3);
            LL_L2UI(local_avg, templl);
            //offset_avg = Ey>>3;
            LL_SHR(templl, Ey, 3);
            LL_L2I(offset_avg, templl);
            break;

        default:
            //local_avg = Ex /num_sync_entries; //toooooo~ sloooooowz~~~
            //return;
            LL_UI2L(templl, num_sync_entries);
            LL_DIV(templl, Ex, templl);
            LL_L2UI(local_avg, templl);

#if (((__TID__ >> 8) & 0x7F) == 0x2b)     /* 0x2b = 43 dec */
            LL_DIV(offset_avg, Ey, num_sync_entries);
#else
            int64_t result = {0};
            int64_t temp = {0};
            result.lo = offset_avg;
            temp.lo = num_sync_entries;
            LL_DIV(result, Ey, temp);

#endif
            LL_UI2L(stempll, num_sync_entries);
            LL_DIV(stempll, Ey, stempll);
            LL_L2I(offset_avg, stempll);
            break;
        }

        //      skew = (double)(Exy - local_avg*Ey)/
        //          (double)(Ex2 - local_avg*Ex);

        LL_UI2L(templl, local_avg);
        LL_MUL(stempll, templl, Ey);
        LL_SUB(stempll, Exy, stempll);
        LL_L2D(a, stempll);
        LL_MUL(templl, templl, Ex);
        LL_SUB(templl, Ex2, templl);
        LL_L2D(b, templl);

        skew = a / b;
    }
    else
    {
        //get the oldest entry
        oldentry = list_entry_decap(&sync_points, sync_entry_t, list);

        //check
        newentry->x = rxtime->w - local_base;
        newentry->y = txtime->w - rxtime->w - offset_base;
        if ((newentry->x < oldentry->x) || ((newentry->y ^ oldentry->y) < 0))
        {
            LL_SET(Exy, 0, 0);
            LL_SET(Ey, 0, 0);
            LL_SET(Ex, 0, 0);
            LL_SET(Ex2, 0, 0);
            //should be 7 entries
            list_entry_for_each(pos, &sync_points, sync_entry_t, list)
            {
                pos->x = pos->x - oldentry->x;
                pos->y = pos->y - oldentry->y;
                //pos->x2 = (uint64_t)pos->x * pos->x;
                LL_UI2L(templl, pos->x);
                LL_MUL(pos->x2, templl, templl);
                //pos->xy = (int64_t)pos->y * pos->x;
                LL_I2L(stempll, pos->y);
                LL_MUL(pos->xy, stempll, templl);
                //Ex += pos->x;
                LL_UI2L(templl, pos->x);
                LL_ADD(Ex, Ex, templl);
                //Ey += pos->y;
                LL_I2L(templl, pos->y);
                LL_ADD(Ey, Ey, templl);
                //Exy += pos->xy;
                LL_ADD(Exy, Exy, pos->xy);
                //Ex2 += pos->x2;
                LL_ADD(Ex2, Ex2, pos->x2);
            }

            //now, add the last one
            local_base += oldentry->x;
            offset_base += oldentry->y;
            newentry->x = rxtime->w - local_base;
            newentry->y = txtime->w - rxtime->w - offset_base;
            //newentry->x2 = (uint64_t)newentry->x * newentry->x;
            LL_UI2L(templl, newentry->x);
            LL_MUL(newentry->x2, templl, templl);
            //newentry->xy = (int64_t)newentry->y * newentry->x;
            LL_I2L(stempll, newentry->y);
            LL_MUL(newentry->xy, stempll, templl);

            //Ex += newentry->x;
            LL_UI2L(templl, newentry->x);
            LL_ADD(Ex, Ex, templl);
            //Ey += newentry->y;
            LL_I2L(templl, newentry->y);
            LL_ADD(Ey, Ey, templl);
            //Exy += newentry->xy;
            LL_ADD(Exy, Exy, newentry->xy);
            //Ex2 += newentry->x2;
            LL_ADD(Ex2, Ex2, newentry->x2);
            // print delay
        }
        else
        {
            //newentry->x2 = (uint64_t)newentry->x * newentry->x;
            LL_UI2L(templl, newentry->x);
            LL_MUL(newentry->x2, templl, templl);

            //newentry->xy = (int64_t)newentry->y * newentry->x;
            LL_I2L(stempll, newentry->y);
            LL_MUL(newentry->xy, stempll, templl);

            //now, update the parameters --195
            //Ex += newentry->x - oldentry->x;
            LL_UI2L(templl, newentry->x - oldentry->x);
            LL_ADD(Ex, Ex, templl);

            //Ey += newentry->y - oldentry->y;
            LL_I2L(templl, newentry->y - oldentry->y);
            LL_ADD(Ey, Ey, templl);

            //Exy += newentry->xy - oldentry->xy;
            LL_ADD(Exy, Exy, newentry->xy);
            LL_SUB(Exy, Exy, oldentry->xy);

            //Ex2 += newentry->x2 - oldentry->x2;
            LL_ADD(Ex2, Ex2, newentry->x2);
            LL_SUB(Ex2, Ex2, oldentry->x2);
        }
        //local_avg = Ex>>3;
        LL_USHR(templl, Ex, 3);
        LL_L2UI(local_avg, templl);

        //offset_avg = Ey>>3;
        LL_SHR(templl, Ey, 3);
        LL_L2I(offset_avg, templl);

        list_add_to_tail(&newentry->list, &sync_points);
        newentry = oldentry;

        //      skew = (double)(Exy - local_avg*Ey)/
        //          (double)(Ex2 - local_avg*Ex);
        LL_UI2L(templl, local_avg);
        LL_MUL(stempll, templl, Ey);
        LL_SUB(stempll, Exy, stempll);
        LL_L2D(a, stempll);
        LL_MUL(templl, templl, Ex);
        LL_SUB(templl, Ex2, templl);
        LL_L2D(b, templl);

        skew = a / b;
    }
#endif
}


void m_sync_g2l(hal_time_t *gtime)
{
#if SYN_CODE_COMPRESS_EN == 0
    if (gtime == NULL)
    {
        _NOP();
        return;
    }

    int32_t approxLocalTime = gtime->w - offset_avg - offset_base;
    gtime->w = approxLocalTime - (int32_t)(skew * (int32_t)(approxLocalTime - local_avg - local_base));
#endif
}
//
void m_sync_l2g(hal_time_t *ltime)
{
#if SYN_CODE_COMPRESS_EN == 0
    if (ltime == NULL)
    {
        return;
    }

    ltime->w += (offset_base + offset_avg +
                 (int32_t)(skew * (int32_t)(ltime->w - local_avg - local_base)));
#endif
}


static void m_sync_sfd_isr(uint16_t time)
{
#if SYN_CODE_COMPRESS_EN == 0
    /* ewan: need to timestamp the rx sfd time, or read it from time cap*/
    if (m_sfd_isr)
    {
        m_sfd_isr(time);
    }
#endif
}

PROCESS(sync_event_process, "sync_event_process");
PROCESS_THREAD(sync_event_process, ev, data)
{
    PROCESS_BEGIN();
    
    while(1)
    {   
#if SYN_CODE_COMPRESS_EN == 0
        if(ev == M_SYNC_BACKGROUND) {

            m_sync_add(&sync_txt, &sync_rxt);
        }
#endif

        PROCESS_YIELD();
    }

    PROCESS_END();
}


void m_sync_init(osel_task_t *task)
{
    /* 绑定同步模块的消息及其相应函数 */
    osel_pthread_create(task, &sync_event_process, NULL);
    sync_reset();

#if SYNC_DBG
    sync_test();
    sync_reset();
#endif
}



#if SYNC_DBG
static hal_time_t xt;
static hal_time_t yt;

static void sync_test (void)
{
    for (uint8_t i = 0; i < 25; i++)
    {
        //300000000,300006000  //20007
        xt.w = 0x00000000 + (uint32_t)i * 100000000;
        yt.w = 0x00000000 + (uint32_t)i * 100001000;

        m_sync_add(&yt, &xt);

        if (i % 2)
        {
            m_sync_g2l(&yt);
        }
        else
        {
            m_sync_g2l(&yt);
        }
    }
}
#endif

