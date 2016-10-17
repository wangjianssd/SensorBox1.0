/**
 * @brief       : 
 *
 * @file        : m24lr64e_arch.c
 * @author      : WangJifang
 * @version     : v0.0.1
 * @date        : 2015/10/23
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/10/23    v0.0.1      WangJifang    some notes
 */

#include <driver.h>
#include <debug.h>
#include <osel_arch.h>
#include <i2c.h>

void m24lr64e_port_init(void)
{
 	TB0CCTL0 &= ~CCIE; 
  	P4SEL |= BIT0;   //config P8.0 as input capture io
	P4DIR &= ~BIT0;  //config P8.0 as input
    TB0CCTL0 = CM_1 + CCIS_0 + SCS + CAP;  //CM_1上升沿捕获，CCIS_1选择CCIxB,
                                            //SCS同步捕获，CAP捕获模式  
}

void m24lr64e_int_cfg(void)
{
  	//设置中断P8.0
  	TB0CCTL0 &= ~CCIFG;//初始为中断未挂起
	TB0CCTL0 |= CCIE;// 开启中断使能
}

void m24lr64e_vcc_open(void)
{
    P3SEL &= ~BIT7;
	P3DIR |= BIT7;
	P3OUT |= BIT7;
}

void m24lr64e_vcc_close(void)
{
    P3SEL &= ~BIT7;
	P3DIR |= BIT7;
	P3OUT &= ~BIT7;
}
