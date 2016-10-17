/**
 * @brief       : 
 *
 * @file        : wsnos_init.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/9/30
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/9/30    v0.0.1      gang.cheng    first version
 */
#include "lib.h"
#include "wsnos.h"


void osel_env_init(osel_uint8_t *buf, 
                   osel_uint16_t size,
                   osel_uint8_t max_prio)
{
    osel_uint8_t tbl_size = ((max_prio - 1) / 8) + 1;
    
    /* wsnos mem module init */
    osel_mem_init(buf, size);
    
    /** wsnos task module init */
    g_task_list = osel_mem_alloc(sizeof(osel_task_t) * max_prio);
    DBG_ASSERT(g_task_list != NULL __DBG_LINE);
    
#if OSEL_DBG_EN
    osel_register_grp = 0;
    osel_register_tbl = osel_mem_alloc(sizeof(osel_uint8_t) * tbl_size);
    DBG_ASSERT(osel_register_tbl != NULL __DBG_LINE);
#endif
    
    /** wsnos sched module init */
    osel_rdy_grp = 0;
    osel_rdy_tbl = osel_mem_alloc(sizeof(osel_uint8_t) * tbl_size);
    DBG_ASSERT(osel_rdy_tbl != NULL __DBG_LINE);
    
    osel_pthread_init();
}







                   




