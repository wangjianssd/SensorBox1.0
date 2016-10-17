/**
 * @brief       : 这个文件是os的任务创建部分
 *
 * @file        : wsnos_task.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng   first version
 * 2015/09/30  v0.0.2      gang.cheng   add pthread lib support
 */
#include "common/lib/lib.h"
#include "wsnos.h"

osel_task_t *g_task_list = NULL;
static osel_task_handler osel_idle_task_handler = NULL;

#if OSEL_DBG_EN > 0
osel_uint8_t osel_register_grp = 0;
osel_uint8_t *osel_register_tbl = NULL;
#endif

/**
 * @brief 创建一个任务，该版本里面，任务不进行实际任务操作，只是作为线程的载体，
 *        为线程调度的主体。
 * @param[in] entry 任务执行的函数指针
 * @param[in] prio  任务的优先级
 * @param[in] event_sto[] 任务的消息队列实际存储空间的起始地址
 * @param[in] event_len 任务的消息队列实际存储空间的长度
 */
osel_task_t *osel_task_create(void (*entry)(void *param),
                              osel_uint8_t prio,
                              osel_event_t event_sto[],
                              osel_uint8_t event_len)
{
    DBG_ASSERT(prio < OSEL_MAX_PRIO __DBG_LINE);

#if OSEL_DBG_EN > 0
    osel_uint8_t i = 0;
    if ((osel_register_grp & osel_map_tbl[prio >> 3]) == osel_map_tbl[prio >> 3]
            && ((osel_register_tbl[prio >> 3] & osel_map_tbl[prio & 0x07])
                == osel_map_tbl[prio & 0x07]))
    {
        return NULL;
    }

    osel_register_grp |= osel_map_tbl[prio >> 3];
    osel_register_tbl[prio >> 3] |= osel_map_tbl[prio & 0x07];

    for (i = 0; i < OSEL_MAX_PRIO; i++)
    {
        if ((entry != NULL) && (g_task_list[i].entry == entry))
        {
            return NULL;
        }
    }
#endif

    osel_task_t *tcb  = &g_task_list[prio];
    tcb->entry = entry;
    tcb->prio = prio;

    osel_equeue_init(&tcb->equeue, event_sto, event_len);

    tcb->process_list = NULL;

    return tcb;
}



void osel_idle_hook(osel_task_handler idle)
{
    if (idle != OSEL_NULL)
    {
        osel_idle_task_handler = idle;
    }
}

static void osel_onidle(void)
{
    if (osel_idle_task_handler != OSEL_NULL)
    {
        osel_idle_task_handler(OSEL_NULL);
    }
}

void osel_run(void)
{
    //delay_ms(1000);

	LED_CLOSE(BLUE);
	LED_CLOSE(GREEN);
	LED_CLOSE(RED);

    osel_start();
    
    OSEL_INT_LOCK();
    osel_schedule();
    OSEL_INT_UNLOCK();

    while (1)
    {
        osel_onidle();
        hal_wdt_clear(16000);
    }
}

