/*************************************************************************************************************
*                                              INCLUDE FILES                                                 *
*************************************************************************************************************/
#include <gznet.h>
#include <gprs_tx.h>
#include <gprs_rx.h>
#include "gps.h"
#include "fifo.h"
//#include <sqqueue.h>
//#include <coap.h>
#include <stdio.h>

/*************************************************************************************************************
*                                            FUNCTION PROTOTYPES                                             *
*************************************************************************************************************/
void FIFOInit (FIFODataTypeDef *pfifo, uint16_t fifo_size)
{
    pfifo->info.front = 0;
    pfifo->info.rear = 0;
    pfifo->info.flag = FIFO_STATUS_EMPTY;
    pfifo->info.fifo_size = fifo_size;
}

uint8_t FIFOIn (FIFODataTypeDef *pfifo, uint8_t *byte)
{   
    uint8_t *p;
    
    if (pfifo->info.flag == FIFO_STATUS_FULL)
    {
        return FIFO_ERROR_FULL;
    }
    
    p = (uint8_t *)((uint32_t)(&pfifo->pdata) + pfifo->info.rear);

    *p = *byte;
    
    pfifo->info.rear = (pfifo->info.rear + 1) % pfifo->info.fifo_size;
    
    if (pfifo->info.rear == pfifo->info.front)
    {
        pfifo->info.flag = FIFO_STATUS_FULL;
    }
    else
    {
        pfifo->info.flag = FIFO_STATUS_OTHER;
    }
    
    return FIFO_OK;
}

uint8_t FIFOOut (FIFODataTypeDef *pfifo, uint8_t *byte)
{
    uint8_t *p;
    
    if (pfifo->info.flag == FIFO_STATUS_EMPTY)
    {
        return FIFO_ERROR_EMPTY;
    }

    p = (uint8_t *)((uint32_t)(&pfifo->pdata) + pfifo->info.front);
    
    *byte = *p;
    
    pfifo->info.front = (pfifo->info.front + 1) % pfifo->info.fifo_size;
    
    if (pfifo->info.rear == pfifo->info.front)
    {
        pfifo->info.flag = FIFO_STATUS_EMPTY;
    }
    else
    {
        pfifo->info.flag = FIFO_STATUS_OTHER;
    }
    
    return FIFO_OK;
}

uint8_t FIFOIsEmpty (FIFODataTypeDef *pfifo)
{
    if (pfifo->info.flag == FIFO_STATUS_EMPTY)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t FIFOIsFull (FIFODataTypeDef *pfifo)
{
    if (pfifo->info.flag == FIFO_STATUS_FULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t GetFIFOCount (FIFODataTypeDef *pfifo)
{
   uint16_t count;
    
    if (pfifo->info.rear < pfifo->info.front)
    {
        count = pfifo->info.rear + pfifo->info.fifo_size - pfifo->info.front;
    }
    else if (pfifo->info.rear == pfifo->info.front)
    {
        if (pfifo->info.flag == FIFO_STATUS_FULL)
        {
            count = pfifo->info.fifo_size;
        }
        else
        {
            count = 0;
        }
    }
    else
    {
        count = pfifo->info.rear - pfifo->info.front;
    }
     
    return count;
}


/*************************************************************************************************************
*                                                  END FILE                                                  *
*************************************************************************************************************/
