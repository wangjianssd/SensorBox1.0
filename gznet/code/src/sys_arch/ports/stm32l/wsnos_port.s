;/**
; * @brief       : 
; *
; * @file        : wsnos_port.s
; * @author      : gang.cheng
; * @version     : v0.0.1
; * @date        : 2015/6/29
; *
; * Change Logs  :
; *
; * Date        Version      Author      Notes
; * 2015/6/29    v0.0.1      gang.cheng    first version
; */

    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8
    
    EXTERN  osel_schedule                       ; external reference
    EXTERN  hard_fault_exception
;/*
; * rt_base_t interrupt_disable();
; */
    EXPORT interrupt_disable
interrupt_disable:
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR

;/*
; * void interrupt_enable(rt_base_t level);
; */
    EXPORT  interrupt_enable
interrupt_enable:
    MSR     PRIMASK, r0
    BX      LR

;/*
; * void osel_start(void);
; */
    EXPORT  osel_start
osel_start:
    MRS     r0, PRIMASK         ; store the state of the PRIMASK in r0
    CPSID   I                   ; disable interrupts (set PRIMASK)

    LDR     r1,=0xE000ED18      ; System Handler Priority Register
    
    LDR     r2,[r1,#8]          ; load the system 12-15 priority register
    MOVS    r3,#0xFF
    LSLS    r3,r3,#16
    ORRS    r2,r3               ; SET pri_14(PendSV) to 0xFF
    STR     r2,[r1,#8]          ; write the System 12-15 Priority Register
    
    LDR     r2,[r1,#4]          ; load the System 8-11 Priority Register
    LSLS    r3,r3,#8
    BICS    r2,r3               ; set PRI_11 (SVCall) to 0x00
    STR     r2,[r1,#4]          ; write the System 8-11 Priority Register
    
    MSR     PRIMASK,r0          ; restore the original PRIMASK
    BX      LR                  ; return to the caller
    
;*****************************************************************************
; The PendSV_Handler exception handler is used for handling asynchronous
; preemption in QK. The use of the PendSV exception is the recommended
; and most efficient method for performing context switches with ARM Cortex-M.
;
; The PendSV exception should have the lowest priority in the whole system
; (0xFF, see osel_start). All other exceptions and interrupts should have higher
; priority. For example, for NVIC with 2 priority bits all interrupts and
; exceptions must have numerical value of priority lower than 0xC0. In this
; case the interrupt priority levels available to your applications are (in
; the order from the lowest urgency to the highest urgency): 0x80, 0x40, 0x00.
;
; Also, *all* ISRs in the QK application must trigger the PendSV exception
; by calling the OSEL_ISR_EXIT() macro.
;
; Due to tail-chaining and its lowest priority, the PendSV exception will be
; entered immediately after the exit from the *last* nested interrupt (or
; exception). In QK, this is exactly the time when the QK scheduler needs to
; check for the asynchronous preemption.
;*****************************************************************************    
    EXPORT  PendSV_Handler
PendSV_Handler:
    PUSH    {lr}                ; push the exception lr (EXC_RETURN)
    
    MOVS    r0,#(0xFF>>2)       ; Keep in synch with BASEPRI in wsnos_port.h
    MSR     BASEPRI,r0          ; disable interrupts at processor level
    
    SUB     sp,sp,#4            ; align the stack to 8-byte boundary
    MOVS    r3,#1
    LSLS    r3,r3,#24           ; r3:=(1<<24),set the T bit     (new xpsr)
    LDR     r2,=osel_schedule   ; address of the wsnos schedule (new pc)
    LDR     r1,=svc_ret         ; return address after the call (new lr)
    PUSH    {r1-r3}             ; push xpsr,pc,lr
    SUB     sp,sp,#(4*4)        ; don't care for r12,r3,r2,r1
    PUSH    {r0}                ; push the prio argument        (new r0)
    MOVS    r0,#0x6
    MVNS    r0,r0               ; r0 := ~0x6 == 0xFFFFFFF9
    BX      r0                  ; exception-return to the scheduler

svc_ret:
    MOVS    r0,#0
    MSR     BASEPRI,r0          ; enable interrupts (remove BASEPRI)

    SVC     #0                  ; SV exception returns to the preempted task

;*****************************************************************************
; The SVC_Handler exception handler is used for returning back to the
; interrupted task. The SVCall exception simply removes its own interrupt
; stack frame from the stack and returns to the preempted task using the
; interrupt stack frame that must be at the top of the stack.
;*****************************************************************************
    EXPORT  SVC_Handler
SVC_Handler:
    ADD     sp,sp,#(9*4)      ; remove one 8-register exception frame
                              ; plus the "aligner" from the stack
    POP     {r0}              ; pop the original EXC_RETURN into r0
    BX      r0                ; return to the preempted task


    EXPORT HardFault_Handler
HardFault_Handler:

    ; get current context
    MRS     r0, msp                 ; get fault context from handler.
    TST     lr, #0x04               ; if(!EXC_RETURN[2])
    BEQ     _get_sp_done
    MRS     r0, psp                 ; get fault context from thread.
_get_sp_done

    STMFD   r0!, {r4 - r11}         ; push r4 - r11 register
    ;STMFD   r0!, {lr}               ; push exec_return register
    SUB     r0, r0, #0x04
    STR     lr, [r0]

    TST     lr, #0x04               ; if(!EXC_RETURN[2])
    BEQ     _update_msp
    MSR     psp, r0                 ; update stack pointer to PSP.
    B       _update_done
_update_msp
    MSR     msp, r0                 ; update stack pointer to MSP.
_update_done

    PUSH    {lr}
    BL      hard_fault_exception
    POP     {lr}

    ORR     lr, lr, #0x04
    BX      lr
    
    ALIGNROM 2,0xFF           ; make sure the END is properly aligned
    END