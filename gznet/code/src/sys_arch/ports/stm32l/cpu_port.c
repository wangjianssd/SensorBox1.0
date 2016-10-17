/**
 * @brief       : 
 *
 * @file        : cpu_port.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/7/1
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/7/1    v0.0.1      gang.cheng    first version
 */
#include "common/lib/lib.h"
#include "platform/platform.h"

#define CPUPORT_PRINTF(fmt, ...)   __asm("NOP");

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28) /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C) /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34) /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38) /* Bus Fault Address Register */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */



struct exception_stack_frame
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

struct stack_frame
{
    /* r4 ~ r11 register */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

struct exception_info
{
    uint32_t exc_return;
    struct stack_frame stack_frame;
};

static struct stack_frame* context = NULL;

static void usage_fault_track(void)
{
    CPUPORT_PRINTF("usage fault:\n");
    CPUPORT_PRINTF("SCB_CFSR_UFSR:0x%02X ", SCB_CFSR_UFSR);

    if(SCB_CFSR_UFSR & (1<<0))
    {
        /* [0]:UNDEFINSTR */
        CPUPORT_PRINTF("UNDEFINSTR ");
    }

    if(SCB_CFSR_UFSR & (1<<1))
    {
        /* [1]:INVSTATE */
        CPUPORT_PRINTF("INVSTATE ");
    }

    if(SCB_CFSR_UFSR & (1<<2))
    {
        /* [2]:INVPC */
        CPUPORT_PRINTF("INVPC ");
    }

    if(SCB_CFSR_UFSR & (1<<3))
    {
        /* [3]:NOCP */
        CPUPORT_PRINTF("NOCP ");
    }

    if(SCB_CFSR_UFSR & (1<<8))
    {
        /* [8]:UNALIGNED */
        CPUPORT_PRINTF("UNALIGNED ");
    }

    if(SCB_CFSR_UFSR & (1<<9))
    {
        /* [9]:DIVBYZERO */
        CPUPORT_PRINTF("DIVBYZERO ");
    }

    CPUPORT_PRINTF("\n");
}

static void bus_fault_track(void)
{
    CPUPORT_PRINTF("bus fault:\n");
    CPUPORT_PRINTF("SCB_CFSR_BFSR:0x%02X ", SCB_CFSR_BFSR);

    if(SCB_CFSR_BFSR & (1<<0))
    {
        /* [0]:IBUSERR */
        CPUPORT_PRINTF("IBUSERR ");
    }

    if(SCB_CFSR_BFSR & (1<<1))
    {
        /* [1]:PRECISERR */
        CPUPORT_PRINTF("PRECISERR ");
    }

    if(SCB_CFSR_BFSR & (1<<2))
    {
        /* [2]:IMPRECISERR */
        CPUPORT_PRINTF("IMPRECISERR ");
    }

    if(SCB_CFSR_BFSR & (1<<3))
    {
        /* [3]:UNSTKERR */
        CPUPORT_PRINTF("UNSTKERR ");
    }

    if(SCB_CFSR_BFSR & (1<<4))
    {
        /* [4]:STKERR */
        CPUPORT_PRINTF("STKERR ");
    }

    if(SCB_CFSR_BFSR & (1<<7))
    {
        CPUPORT_PRINTF("SCB->BFAR:%08X\n", SCB_BFAR);
    }
    else
    {
        CPUPORT_PRINTF("\n");
    }
}

static void mem_manage_fault_track(void)
{
    CPUPORT_PRINTF("mem manage fault:\n");
    CPUPORT_PRINTF("SCB_CFSR_MFSR:0x%02X ", SCB_CFSR_MFSR);

    if(SCB_CFSR_MFSR & (1<<0))
    {
        /* [0]:IACCVIOL */
        CPUPORT_PRINTF("IACCVIOL ");
    }

    if(SCB_CFSR_MFSR & (1<<1))
    {
        /* [1]:DACCVIOL */
        CPUPORT_PRINTF("DACCVIOL ");
    }

    if(SCB_CFSR_MFSR & (1<<3))
    {
        /* [3]:MUNSTKERR */
        CPUPORT_PRINTF("MUNSTKERR ");
    }

    if(SCB_CFSR_MFSR & (1<<4))
    {
        /* [4]:MSTKERR */
        CPUPORT_PRINTF("MSTKERR ");
    }

    if(SCB_CFSR_MFSR & (1<<7))
    {
        /* [7]:MMARVALID */
        CPUPORT_PRINTF("SCB->MMAR:%08X\n", SCB_MMAR);
    }
    else
    {
        CPUPORT_PRINTF("\n");
    }
}

static void hard_fault_track(void)
{
    if(SCB_HFSR & (1UL<<1))
    {
        /* [1]:VECTBL, Indicates hard fault is caused by failed vector fetch. */
        CPUPORT_PRINTF("failed vector fetch\n");
    }

    if(SCB_HFSR & (1UL<<30))
    {
        /* [30]:FORCED, Indicates hard fault is taken because of bus fault,
                        memory management fault, or usage fault. */
        if(SCB_CFSR_BFSR)
        {
            bus_fault_track();
        }

        if(SCB_CFSR_MFSR)
        {
            mem_manage_fault_track();
        }

        if(SCB_CFSR_UFSR)
        {
            usage_fault_track();
        }
    }

    if(SCB_HFSR & (1UL<<31))
    {
        /* [31]:DEBUGEVT, Indicates hard fault is triggered by debug event. */
        CPUPORT_PRINTF("debug event\n");
    }
}

/*
 * fault exception handler
 */
void hard_fault_exception(struct exception_info * exception_info)
{
    context = &exception_info->stack_frame;

    CPUPORT_PRINTF("psr: 0x%08x\n", context->exception_stack_frame.psr);

    CPUPORT_PRINTF("r00: 0x%08x\n", context->exception_stack_frame.r0);
    CPUPORT_PRINTF("r01: 0x%08x\n", context->exception_stack_frame.r1);
    CPUPORT_PRINTF("r02: 0x%08x\n", context->exception_stack_frame.r2);
    CPUPORT_PRINTF("r03: 0x%08x\n", context->exception_stack_frame.r3);
    CPUPORT_PRINTF("r04: 0x%08x\n", context->r4);
    CPUPORT_PRINTF("r05: 0x%08x\n", context->r5);
    CPUPORT_PRINTF("r06: 0x%08x\n", context->r6);
    CPUPORT_PRINTF("r07: 0x%08x\n", context->r7);
    CPUPORT_PRINTF("r08: 0x%08x\n", context->r8);
    CPUPORT_PRINTF("r09: 0x%08x\n", context->r9);
    CPUPORT_PRINTF("r10: 0x%08x\n", context->r10);
    CPUPORT_PRINTF("r11: 0x%08x\n", context->r11);
    CPUPORT_PRINTF("r12: 0x%08x\n", context->exception_stack_frame.r12);
    CPUPORT_PRINTF(" lr: 0x%08x\n", context->exception_stack_frame.lr);
    CPUPORT_PRINTF(" pc: 0x%08x\n", context->exception_stack_frame.pc);

    if(exception_info->exc_return & (1 << 2) )
    {
        CPUPORT_PRINTF("hard fault on thread\r\n\r\n");
    }
    else
    {
        CPUPORT_PRINTF("hard fault on handler\r\n\r\n");
    }

    hard_fault_track();

    while (1)
    {
        context = context;
    }
}


