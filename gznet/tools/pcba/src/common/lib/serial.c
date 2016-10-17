/**
 * @brief       : 
 *
 * @file        : serial.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <debug.h>
#include <serial.h>
#include <sqqueue.h>
#include <hal.h>
#include <debug.h>
#include <osel_arch.h>

#define SERIAL_BUF_RECV_SQQ_LEN         128u
#define SERIAL_BUF_SEND_SQQ_LEN         0u

typedef uint8_t                         serial_entry_t;
typedef void (*state_t) (uint8_t serial_id, uint8_t sig, uint8_t ch);

typedef enum _event  /* enumeration */
{
    SD_SIG,
    LD_SIG,
    CHAR_SIG,
    ED_SIG,
} sig_event;

typedef struct _fsm_t_
{
    state_t current_state;
} fsm_t;

typedef struct _serial_frm_t_
{
    uint8_t sd_index;

    struct
    {
        uint8_t data[SERIAL_LD_LEN_MAX];
        uint8_t index;
        uint16_t frm_len;
    } ld; //length describe

    uint16_t payload_len;       // actually len of recvived data
    uint8_t ed_index;

    uint16_t last_enter_q_num;  // record the num of frames has entered the queue
    uint8_t locked;
    uint8_t sig;
} serial_frm_t;

#define TRAN(state)                       (fsm[serial_id].current_state = (state_t)(state))
#define FSM_DISPATCH(serial_id, sig, ch)  (fsm[serial_id].current_state((serial_id), (sig), (ch)))

static fsm_t fsm[SERIAL_NUM] = {0};

static serial_frm_t serial_frm_array[SERIAL_NUM];
static serial_reg_t serial_reg_array[SERIAL_NUM];

static sqqueue_ctrl_t serial_recv_sqq[SERIAL_NUM];
//static sqqueue_ctrl_t serial_send_sqq[SERIAL_NUM];

static void wait_sd_state(uint8_t serial_id, uint8_t sig, uint8_t ch);
/************************************************************/
void lock_serial(uint8_t serial_id)
{
    hal_uart_recv_disable(serial_id);
    
    serial_frm_array[serial_id].locked = TRUE;
    for (uint8_t i = 0; i < serial_frm_array[serial_id].last_enter_q_num; i++)
    {
        serial_recv_sqq[serial_id].revoke(&serial_recv_sqq[serial_id]);
    }

    serial_frm_array[serial_id].last_enter_q_num = 0;
    serial_frm_array[serial_id].sd_index = 0;
    serial_frm_array[serial_id].ed_index = 0;
    serial_frm_array[serial_id].ld.frm_len = 0;
    serial_frm_array[serial_id].payload_len = 0;

    TRAN(wait_sd_state);
}

void unlock_serial(uint8_t serial_id)
{
    TRAN(wait_sd_state);
    
    serial_frm_array[serial_id].last_enter_q_num = 0;
    serial_frm_array[serial_id].sd_index = 0;
    serial_frm_array[serial_id].ed_index = 0;
    serial_frm_array[serial_id].ld.frm_len = 0;
    serial_frm_array[serial_id].payload_len = 0;
    
    serial_frm_array[serial_id].locked = FALSE;
    
    hal_uart_recv_enable(serial_id);
}

static void end_state_handle(uint8_t serial_id, uint8_t sig, uint8_t ch)
{
    TRAN(wait_sd_state);
    serial_frm_array[serial_id].ld.frm_len = 0;
    serial_frm_array[serial_id].payload_len = 0;
    serial_frm_array[serial_id].last_enter_q_num = 0;
    if (serial_reg_array[serial_id].func_ptr != NULL)
    {
        (*(serial_reg_array[serial_id].func_ptr))();
    }
}

static void wait_end_state(uint8_t serial_id, uint8_t sig, uint8_t ch)
{
    if (!serial_reg_array[serial_id].ed.valid)
    {
        if (serial_frm_array[serial_id].ld.frm_len == 0)
        {
            TRAN(wait_sd_state);

            end_state_handle(serial_id, sig, ch);

            for (uint8_t i = 0; i < serial_frm_array[serial_id].last_enter_q_num; i++)
            {
                serial_recv_sqq[serial_id].revoke(&serial_recv_sqq[serial_id]);
            }
            serial_frm_array[serial_id].last_enter_q_num = 0;
        }
        else
        {
            if (serial_recv_sqq[serial_id].enter(&serial_recv_sqq[serial_id], (void *)&ch))
            {
                serial_frm_array[serial_id].last_enter_q_num++;
                if (++serial_frm_array[serial_id].payload_len ==
                        serial_frm_array[serial_id].ld.frm_len)
                {
                    end_state_handle(serial_id, sig, ch);
                }
            }
            else
            {
                lock_serial(serial_id);
            }
        }
    }
    else
    {
        if (serial_frm_array[serial_id].ld.frm_len == 0)
        {
            if (serial_recv_sqq[serial_id].enter(&serial_recv_sqq[serial_id], (void *)&ch))
            {
                serial_frm_array[serial_id].last_enter_q_num++;
                if (serial_reg_array[serial_id].ed.data[0] == ch)
                {
                    end_state_handle(serial_id, sig, ch);
                }
            }
            else
            {
                lock_serial(serial_id);
            }
        }
        else
        {
            if (serial_recv_sqq[serial_id].enter(&serial_recv_sqq[serial_id], (void *)&ch))
            {
                serial_frm_array[serial_id].last_enter_q_num++;
                if (++serial_frm_array[serial_id].payload_len >=
                        serial_frm_array[serial_id].ld.frm_len)
                {
                    if (serial_reg_array[serial_id].ed.data[0] == ch)
                    {
                        end_state_handle(serial_id, sig, ch);
                    }
                }
            }
            else
            {
                lock_serial(serial_id);
            }
        }
    }
}

static void wait_ld_state(uint8_t serial_id, uint8_t sig, uint8_t ch)
{
    if (!serial_reg_array[serial_id].ld.valid)
    {
        TRAN(wait_end_state);
        FSM_DISPATCH(serial_id, serial_frm_array[serial_id].sig, ch);
        return;
    }

    serial_frm_array[serial_id].ld.data[serial_frm_array[serial_id].ld.index++] = ch;
    if (serial_recv_sqq[serial_id].enter(&serial_recv_sqq[serial_id], (void *)&ch))
    {
        serial_frm_array[serial_id].last_enter_q_num++;
        if (serial_frm_array[serial_id].ld.index == SERIAL_LD_LEN_MAX)
        {
            if (serial_reg_array[serial_id].ld.little_endian == TRUE)
            {
                serial_frm_array[serial_id].ld.frm_len =
                    serial_frm_array[serial_id].ld.data[SERIAL_LD_LEN_MAX - 1] * 256 + \
                    serial_frm_array[serial_id].ld.data[SERIAL_LD_LEN_MAX - 2];
            }
            else
            {
                serial_frm_array[serial_id].ld.frm_len =
                    serial_frm_array[serial_id].ld.data[SERIAL_LD_LEN_MAX - 2] * 256 + \
                    serial_frm_array[serial_id].ld.data[SERIAL_LD_LEN_MAX - 1];
            }

            if ((serial_frm_array[serial_id].ld.frm_len
                    > serial_reg_array[serial_id].argu.len_max)
                    || (serial_frm_array[serial_id].ld.frm_len
                        < serial_reg_array[serial_id].argu.len_min))
            {
                serial_frm_array[serial_id].ld.index = 0;
                TRAN(wait_sd_state);

                for (uint8_t i = 0; i < serial_frm_array[serial_id].last_enter_q_num; i++)
                {
                    serial_recv_sqq[serial_id].revoke(&serial_recv_sqq[serial_id]);
                }

                serial_frm_array[serial_id].ld.frm_len = 0;
                serial_frm_array[serial_id].last_enter_q_num = 0;
            }
            else
            {
                serial_frm_array[serial_id].ld.index = 0;
                TRAN(wait_end_state);
            }
        }
    }
    else
    {
        lock_serial(serial_id);
    }
}

static void wait_sd_state(uint8_t serial_id, uint8_t sig, uint8_t ch)
{
    if (!serial_reg_array[serial_id].sd.valid)
    {
        TRAN(wait_ld_state);
        FSM_DISPATCH(serial_id, serial_frm_array[serial_id].sig, ch);
        return;
    }
    if (serial_reg_array[serial_id].sd.data[serial_frm_array[serial_id].sd_index++] == ch)
    {
        if (serial_recv_sqq[serial_id].enter(&serial_recv_sqq[serial_id], (void *)&ch))
        {
            serial_frm_array[serial_id].last_enter_q_num++;
            if (serial_frm_array[serial_id].sd_index == SERIAL_SD_LEN_MAX)
            {
                serial_frm_array[serial_id].sd_index = 0;
                TRAN(wait_ld_state);
            }
        }
        else
        {
            lock_serial(serial_id);
        }
    }
    else
    {
        for (uint8_t i = 0; i < serial_frm_array[serial_id].last_enter_q_num; i++)
        {
            serial_recv_sqq[serial_id].revoke(&serial_recv_sqq[serial_id]);
        }

        serial_frm_array[serial_id].sd_index = 0;
        serial_frm_array[serial_id].last_enter_q_num = 0;
    }
}

static void serial_char_handle(uint8_t serial_id, uint8_t ch)
{
    if (serial_reg_array[serial_id].echo_en)
    {
        serial_write(serial_id, &ch, 1);
        if(ch == 0x0d)
        {
            uint8_t lf = 0x0a;
            serial_write(serial_id, &lf, 1);
        }
    }

    FSM_DISPATCH(serial_id, serial_frm_array[serial_id].sig, ch);
}

void serial_fsm_init(uint8_t serial_id)
{
    TRAN(wait_sd_state);

    serial_frm_array[serial_id].sig = CHAR_SIG;
    hal_uart_int_cb_reg(serial_char_handle);

    if (sqqueue_ctrl_init(&serial_recv_sqq[serial_id],
                          sizeof(serial_entry_t),
                          SERIAL_BUF_RECV_SQQ_LEN)
            == FALSE)
    {
        DBG_ASSERT(FALSE __DBG_LINE);
    }
}

uint8_t serial_read(uint8_t id, void *buffer, uint16_t len)
{
    uint8_t i = 0;
    serial_entry_t e;
    uint8_t *buf = (uint8_t *)buffer;

    if (serial_recv_sqq[id].get_len(&serial_recv_sqq[id]) >= len)
    {
        for (i = 0; i < len; i++)
        {
            e = *((serial_entry_t *)serial_recv_sqq[id].del(&serial_recv_sqq[id]));
            buf[i] = e;
        }
    }
    else
    {
        while ((serial_recv_sqq[id].get_len(&serial_recv_sqq[id]) != 0) && (i < len))
        {
            e = *((serial_entry_t *)serial_recv_sqq[id].del(&serial_recv_sqq[id]));
            buf[i++] = e;
        }
    }

    if (serial_frm_array[id].locked)
    {
        unlock_serial(id);
    }

    return i;
}

void serial_write(uint8_t id, uint8_t *const string, uint16_t len)
{
    hal_uart_send_string(id, string, len);
}

bool_t serial_reg(uint8_t id, serial_reg_t serial_reg)
{
    if (serial_reg_array[id].func_ptr == NULL)
    {
        osel_memcpy((void *)&serial_reg_array[id], (void *)&serial_reg, sizeof(serial_reg));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void serial_unreg(uint8_t id)
{
    osel_memset((void *)&serial_reg_array[id], 0, sizeof(serial_reg_t));
    serial_reg_array[id].func_ptr = NULL;
}




