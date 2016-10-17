/**
 * @brief       : the port mapping controller allows a flexible mapping of digital
 *              : functions to port pins. the chapter describes the port mapping
 *              : controller
 *
 * @file        : pmap.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/10/13
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/13    v0.0.1      gang.cheng    first version
 */

#include <msp430.h>
#include <lib.h>
#include "osel_arch.h"

bool_t pmap_cfg(uint8_t pin, uint8_t mode);

void pmap_init(void)
{
    osel_int_status_t s = 0;
    
    //*< disable interrupt before altering port mapping registers
    OSEL_ENTER_CRITICAL(s); 
    //*< Enable Write-access to modify port mapping registers 
    PMAPPWD = 0x02d52;
    //*< Allow reconfiguration during runtime
    PMAPCTL = PMAPRECFG;
    
    OSEL_EXIT_CRITICAL(s); 
}


bool_t pmap_cfg(uint8_t pin, uint8_t mode)
{
    osel_int_status_t s = 0;
    
    //*< disable interrupt before altering port mapping registers
    OSEL_ENTER_CRITICAL(s); 
    //*< Enable Write-access to modify port mapping registers 
    PMAPPWD = 0x02d52;
    
    volatile uint8_t *mapp;
    mapp = &P2MAP0;
    
    if(pin > 8)
    {
        return FALSE;
    }
    
    *(mapp+pin) = mode;
    
    OSEL_EXIT_CRITICAL(s);
    
    return TRUE;
}
     



