/**
 * @brief       : 
 *
 * @file        : clock.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"

#define XT2_OPEN            1u
#define XT2_CLOSE           0u

static volatile uint16_t wdt_cnt = 0;
static volatile uint8_t xt2_status = XT2_CLOSE;
static volatile uint16_t backvalue_wdt_cnt = 0;

void wdt_clear(uint32_t time_ms)
{
    if (time_ms == 16000)
    {
        WDTCTL = WDTPW + WDTCNTCL + WDTSSEL0 + WDTIS0 + WDTIS1;
    }
    else if(time_ms == 1000)
    {
        WDTCTL = WDTPW + WDTCNTCL + WDTSSEL0 + WDTIS2;
    }
    else
    {
        wdt_cnt = time_ms/1000;
        TBCCR0 = TB0R + 0x8000;  // 1s
        TBCCTL0 |= CCIE;
    }
}

static bool_t backvalue_wdt_start = FALSE;
void wdt_start(uint32_t time_ms)
{
    if (!backvalue_wdt_start)
    {
        backvalue_wdt_start = TRUE;
        backvalue_wdt_cnt = time_ms/1000;
        TBCCR3 = TB0R + 0x8000;  // 1s
        TBCCTL3 |= CCIE;
    }
}

void wdt_stop(uint32_t time_ms)
{
    if(time_ms == 300000ul) // 5分钟看门狗，用于保护背景值更新错误
    {
        backvalue_wdt_start = FALSE;
        TBCCTL3 &= ~CCIE;
    }
    else
    {
        TBCCTL0 &= ~CCIE;  //由于有周期性定时器存在，TimerB不允许停止
    }
}


void clk_xt2_open_without_wait(void)
{

}


void clk_xt2_open(void)
{

}

void clk_xt2_close(void)
{

}

void clk_protect_int(void)
{
    TBCCR1 = TBR + 0x6666; //820ms
    TBCCTL1 |= CCIE;
}

static void  get_system_clock_settings(uint8_t  system_clock_speed,
                                           uint8_t  *set_dco_range,
                                           uint8_t  *set_vcore,
                                           uint16_t *set_multiplier)
{
	switch (system_clock_speed)
	{
    case SYSCLK_1MHZ:
        *set_dco_range  = (uint8_t)DCORSEL_1MHZ;
        *set_vcore      = (uint8_t)VCORE_1MHZ;
        *set_multiplier = (uint16_t)DCO_MULT_1MHZ;
        break;

    case SYSCLK_4MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_4MHZ;
	    *set_vcore      = (uint8_t)VCORE_4MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_4MHZ;
	    break;

    case SYSCLK_8MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_8MHZ;
	    *set_vcore      = (uint8_t)VCORE_8MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_8MHZ;
	    break;

    case SYSCLK_12MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_12MHZ;
	    *set_vcore      = (uint8_t)VCORE_12MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_12MHZ;
	    break;

    case SYSCLK_16MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_16MHZ;
	    *set_vcore      = (uint8_t)VCORE_16MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_16MHZ;
	    break;

    case SYSCLK_20MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_20MHZ;
	    *set_vcore      = (uint8_t)VCORE_20MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_20MHZ;
	    break;

    case SYSCLK_25MHZ:
	    *set_dco_range  = (uint8_t)DCORSEL_25MHZ;
	    *set_vcore      = (uint8_t)VCORE_25MHZ;
	    *set_multiplier = (uint16_t)DCO_MULT_25MHZ;
	    break;
	}
}

/**
 * set the core voltage down
 * 1.Program the SVML to the new level and wait for (SVSMLDLYIFG) to be set.
 * 2.Program PMMCOREV to the new VCORE level.
 * 3.Wait for the voltage level reached (SVMLVLRIFG) interrupt.
 *
 * vcore:the value of core voltage
 */
static void set_vcore_down(uint8_t VCore)
{
    PMMCTL0_H = 0xA5;  /* Open PMM registers for write access */
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * VCore; /* Set SVM low side to new level */
    while ((PMMIFG & SVSMLDLYIFG) == 0);  /* Wait till SVM is settled */
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);    /* Clear already set flags */
    PMMCTL0_L = PMMCOREV0 * VCore;        /* Set VCore to new level */
    if (PMMIFG & SVMLIFG)
    {
        while ((PMMIFG & SVMLVLRIFG) == 0);    /* Wait till new level reached */
    }
    PMMCTL0_H = 0x00;  /* Lock PMM registers for write access */
}


/**
 * set the core voltage up
 * 1.Program the SVMH and SVSH to the next level to ensure DVCC is high enough
 *   for the next VCORE level.Program the SVML to the next level and wait for
 *   (SVSMLDLYIFG) to be set.
 * 2.Program PMMCOREV to the next VCORE level.
 * 3.Wait for the voltage level reached (SVMLVLRIFG) flag.
 * 4.Program the SVSL to the next level.
 * vcore:the value of core voltage
 */
static void set_vcore_up(uint8_t VCore)
{
    PMMCTL0_H = 0xA5; /* Open PMM registers for write access*/
    /* Set SVS/SVM high side new level */
    SVSMHCTL = SVSHE + SVSHRVL0 * VCore + SVMHE + SVSMHRRL0 * VCore;
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * VCore; /* Set SVM low side to new level */
    while ((PMMIFG & SVSMLDLYIFG) == 0);    /* Wait till SVM is settled */
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);      /* Clear already set flags */
    PMMCTL0_L = PMMCOREV0 *  VCore;         /* Set VCore to new level */
    if ((PMMIFG & SVMLIFG))
    {
        while ((PMMIFG & SVMLVLRIFG) == 0);     /* Wait till new level reached */
    }
    /* Set SVS/SVM low side to new level */
    SVSMLCTL = SVSLE + SVSLRVL0 * VCore + SVMLE + SVSMLRRL0 * VCore;
    PMMCTL0_H = 0x00;  /* Lock PMM registers for write access */
}


/**
 * set the core voltage
 * vcore: the value of core voltage to be set
 */
static void set_vcore(uint8_t VCore)
{
	uint16_t currentVCore;
	currentVCore = PMMCTL0 & PMMCOREV_3;      /* Get actual VCore  */
	while (VCore != currentVCore)
	{
        if (VCore > currentVCore)
        {
            set_vcore_up(++currentVCore);
        }
        else
        {
            set_vcore_down(--currentVCore);
        }
  	}
}

/**
 * set system clock
 * system_clock_speed:the speed of the system clock to be set
 */
static void set_system_clock(uint8_t system_clock_speed)
{
    uint8_t   set_vco_range;
    uint8_t   set_vcore_value;
	uint16_t  set_multiplier;

	get_system_clock_settings( system_clock_speed, &set_vco_range,
                                  &set_vcore_value, &set_multiplier);

	if(set_vcore_value != (PMMCTL0&PMMCOREV_3))	/* Only change VCore if necessary */
    {
		set_vcore(set_vcore_value);
	}

    /* During FLL operation with REFO as the reference clock; if two subsequent
    writes occur to the UCSCTL0-2 registers within one cycle of the DCO,
    the results do not get updated within the FLL.Similarly, if two subsequent
    writes are performed to the UCSCTL3 register within one cycle of FLLREFCLK
    the results do not get updated. */
    __bis_SR_register(SCG0);                /* Disable the FLL control loop */

    /* During FLL operation with REFO as the reference clock; if two subsequent
    writes occur to the UCSCTL0-2 registers within one cycle of the DCO,
    the results do not get updated within the FLL.  Similarly, if two subsequent
    writes are performed to the UCSCTL3 register within one cycle of FLLREFCLK
    the results do not get updated. */
    UCSCTL1 |= DISMOD;
    UCSCTL0 = 0x00;                         /* Set lowest possible DCOx, MODx */
    UCSCTL1 &= ~DISMOD;
	UCSCTL1 = set_vco_range;                /* Select suitable range */
	UCSCTL2 = set_multiplier + FLLD_0;      /* Set DCO Multiplier */
    UCSCTL3 = SELREF__XT1CLK;
    UCSCTL4 = SELA__XT1CLK  | SELS__DCOCLKDIV  | SELM__DCOCLKDIV;
    __bic_SR_register(SCG0);                /* Enable the FLL control loop */

    /* Worst-case settling time for the DCO when the DCO range bits have been */
    /* changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx */
    /* UG for optimization. */
    /* 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle */
    __delay_cycles(250000);

    /*  Loop until XT1, DCO fault flag is cleared  */
    do
    {
        /* Clear XT1,DCO fault flags */
        UCSCTL7 &= ~(XT1LFOFFG + XT1HFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;                      /* Clear fault flags */
    }while (SFRIFG1&OFIFG);                     /* Test oscillator fault flag */
}


static void board_start_xt1(void)
{
    /* Initialize LFXT1 */

    P7SEL |= BIT0 + BIT1;                            /* Select XT1 */
	P7DIR &= ~BIT0;
	P7DIR |= BIT1;
//	P7OUT &= ~BIT1;

    UCSCTL6 &= ~(XT1OFF);                     /* XT1 On */
//    UCSCTL6 |= XCAP_3;                        /* Set internal cap values */

    /*  Set up XT1 Pins to analog function, and to lowest drive	*/
    do
    {
        UCSCTL7 &= ~XT1LFOFFG;                 /* Clear XT1 fault flags */
    }while (UCSCTL7&XT1LFOFFG);                /* Test XT1 fault flag */

/*    UCSCTL6 &= ~(XT1DRIVE_3); */             /* Xtal is now stable, reduce drive */
}

void clk_init(uint8_t system_clock_speed)
{
    board_start_xt1();
	set_system_clock(system_clock_speed);   /* Set board to operate Frequncy */
    TB0CTL |= TBSSEL_1 + MC_2 + TBCLR;  // 启动TimerB定时器
}

#pragma vector = TIMERB0_VECTOR
__interrupt void timer_b0_ccrx( void )
{
	OSEL_ISR_ENTRY();
    TBCCR0 += 0x147; // 10ms
	wsnos_ticks();
	wdt_clear(16000);
	OSEL_ISR_EXIT();
}


#pragma vector = TIMERB1_VECTOR
__interrupt void timer_b1_ccrx( void )
{
    OSEL_ISR_ENTRY();

    switch(TBIV)
    {
    case 2:
        break;

    case 4:
        break;

    case 6:
        TBCCR3 += 0x8000; // 1s
        break;

    default:
        break;
    }

    OSEL_ISR_EXIT();
    LPM3_EXIT;
}
