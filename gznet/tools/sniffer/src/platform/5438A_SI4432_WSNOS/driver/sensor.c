/**
 * @brief       : 
 *
 * @file        : sensor.c
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <driver.h>
#include <wsnos.h>

#define HMC5983_ADDRESS	(0x3C>>1)

#define HMC5983_WAIT_DRDY_BY_INT()                  \
    do                                              \
    {                                               \
        uint32_t i = 0;                             \
        while(!hmc5983_drdy)                        \
        {                                           \
            if (i++ > 70000)                       	\
            {                                       \
                DBG_ASSERT(FALSE __DBG_LINE);       \
            }                                       \
            LPM3;                                   \
        }                                           \
		hmc5983_drdy = FALSE;                       \
    } while(__LINE__ == -1)

#define HMC5983_WAIT_DRDY()                         \
    do                                              \
    {                                               \
		WHILE(P8IN & BIT2);                         \
        WHILE(!(P8IN & BIT2));						\
    } while(__LINE__ == -1)

#define I2C_WRITE_START() 					        \
    do                                              \
    {                                               \
        WHILE(UCB3CTL1 & UCTXSTP);					\
    	UCB3CTL1 |= UCTR;							\
    	UCB3CTL1 |= UCTXSTT;						\
    } while(__LINE__ == -1)

#define I2C_READ_START() 					        \
    do                                              \
    {                                               \
		uint8_t recv = 0;							\
        WHILE(UCB3CTL1 & UCTXSTP);					\
    	UCB3CTL1 &= ~UCTR;							\
    	UCB3CTL1 |= UCTXSTT;						\
		recv = UCB3RXBUF;							\
		recv = recv;								\
    } while(__LINE__ == -1)

#define I2C_WAIT_ADDR_ACK()						    \
	{												\
		WHILE(UCB3CTL1 & UCTXSTT);					\
	}

#define I2C_SEND_STOP_BIT()			UCB3CTL1 |= UCTXSTP

#define I2C_WAIT_STOP()				while(UCB3CTL1 & UCTXSTP)

#define I2C_SEND_CHAR(x)                            \
    do                                              \
    {                                               \
		WHILE(!(UCB3IFG & UCTXIFG));				\
    	UCB3TXBUF = (x);							\
    } while(__LINE__ == -1)

#define I2C_RECCEIVE_CHAR(x)                        \
    do                                              \
    {                                               \
		WHILE(!(UCB3IFG & UCRXIFG));				\
		(x) = UCB3RXBUF;							\
    } while(__LINE__ == -1)

#define I2C_IS_NO_ACK()					(UCB3IFG & UCNACKIFG)

static volatile bool_t hmc5983_drdy = FALSE;
static uint8_t reg_a_v = REG_A_DEFAULT_VALUE;
static uint8_t reg_b_v = REG_B_DEFAULT_VALUE;

bool_t compass_sensor_self_test(int16_t *x, int16_t *y, int16_t *z);

static void hmc5983_i2c_init(void)
{
	P10SEL |= BIT1;
    P10SEL |= BIT2;

	UCB3CTL1 |= UCSWRST;
    UCB3CTL0 = UCMST + UCMODE_3 + UCSYNC ; // I2C Master, synchronous mode
    UCB3CTL1 |= UCSSEL_2; // Use SMCLK, keep SW reset
    UCB3BR0 = 20; // fSCL = SMCLK/20 = 400kHz
    UCB3BR1 = 0;
	UCB3CTL0 &= ~UCSLA10; //7位地址
    UCB3I2CSA = HMC5983_ADDRESS;
    UCB3CTL1 &= ~UCSWRST;
}

static void hmc5983_port_init(void)
{
    P8SEL |= BIT2;
    P8DIR &= ~BIT2;
    TA0CCTL2 = CM_2 + CCIS_1 + SCS + CAP; ///  下降沿capture，CCI3B，同步

    TA0CCTL2 &= ~CCIFG;
    TA0CCTL2 |= CCIE;
    hmc5983_drdy = FALSE;
}

static bool_t hmc5983_reg_write(uint8_t addr, uint8_t val)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

	I2C_WRITE_START();
	I2C_SEND_CHAR(addr);
	I2C_WAIT_ADDR_ACK();
	I2C_SEND_CHAR(val);
	while(!(UCB3IFG & UCTXIFG));
	I2C_SEND_STOP_BIT();
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        OSEL_EXIT_CRITICAL(status);
		return FALSE;
	}

    OSEL_EXIT_CRITICAL(status);
	return TRUE;
}

static bool_t hmc5983_reg_read(uint8_t addr, uint8_t *val)
{
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

	I2C_WRITE_START();
	I2C_SEND_CHAR(addr);
	I2C_WAIT_ADDR_ACK();
	while(!(UCB3IFG & UCTXIFG));
	I2C_SEND_STOP_BIT();
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        OSEL_EXIT_CRITICAL(status);
		return FALSE;
	}

	I2C_READ_START();
	I2C_WAIT_ADDR_ACK();
	I2C_SEND_STOP_BIT();
	I2C_RECCEIVE_CHAR(*val);
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        OSEL_EXIT_CRITICAL(status);
		return FALSE;
	}

    OSEL_EXIT_CRITICAL(status);
	return TRUE;
}

static bool_t hmc5983_reg_config(uint8_t reg_a, uint8_t reg_b)
{
	static uint8_t reg_a_r;
	static uint8_t reg_b_r;

	if(FALSE == hmc5983_reg_write(CONFIG_REG_A_ADDR, reg_a))
		return FALSE;
	if(FALSE == hmc5983_reg_write(CONFIG_REG_B_ADDR, reg_b))
		return FALSE;
	if(FALSE == hmc5983_reg_read(CONFIG_REG_A_ADDR, &reg_a_r))
		return FALSE;
	if(FALSE == hmc5983_reg_read(CONFIG_REG_B_ADDR, &reg_b_r))
		return FALSE;

	if(reg_a != reg_a_r || reg_b != reg_b_r)
	{
		return FALSE;
	}
    return TRUE;
}
/* I2C死锁以后的解决方法 */
static void hmc5983_i2c_lock_init(void)
{
    //P10.2 作为模拟scl，输出9个信号
    P10SEL &= ~BIT2;
	P10DIR |= BIT2;
    P10OUT |= BIT2;
    // 主设备模拟SCL，从高到低，输出9次，使得从设备释放SDA
    for(uint8_t i=0;i<9;i++)
    {
        P10OUT |= BIT2;
        delay_ms(1);
        P10OUT &= ~BIT2;
        delay_ms(1);
    }
}

bool_t compass_sensor_init(void)
{
    hmc5983_i2c_lock_init();

    hmc5983_i2c_init();
    hmc5983_port_init();
    reg_a_v = REG_A_DEFAULT_VALUE;
    reg_b_v = REG_B_DEFAULT_VALUE;
	if (FALSE == hmc5983_reg_config(REG_A_DEFAULT_VALUE, REG_B_DEFAULT_VALUE))
    {
        return FALSE;
    }
    int16_t x, y, z;
    if (FALSE == compass_sensor_self_test(&x, &y, &z))
    {
        return FALSE;
    }
    if (   (x < 243 || x > 575)
        || (y < 243 || y > 575)
        || (z < 243 || z > 575))
    {
        return FALSE;
    }
    
    return TRUE;
}

bool_t compass_sensor_detect(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6] = {0, };

    if((reg_a_v != REG_A_VALUE_FOR_DETECT)
       || (reg_b_v != REG_B_VALUE_FOR_DETECT))
    {
        reg_a_v = REG_A_VALUE_FOR_DETECT;
        reg_b_v = REG_B_VALUE_FOR_DETECT;
        if(FALSE == hmc5983_reg_config(REG_A_VALUE_FOR_DETECT, REG_B_VALUE_FOR_DETECT))
        {
            return FALSE;
        }
    }
	if(FALSE == hmc5983_reg_write(MODE_REG_ADDR, MODE_REG_VALUE_FOR_SINGLE))
	{
		return FALSE;
	}

    HMC5983_WAIT_DRDY_BY_INT();
        

	for(int i=0; i<6; i++)
	{
		if(FALSE == hmc5983_reg_read(X_MSB_REG_ADDR + i, &buf[i]))
		{
			return FALSE;
		}
	}

	*x = BUILD_UINT16(buf[1], buf[0]);
	*z = BUILD_UINT16(buf[3], buf[2]);
	*y = BUILD_UINT16(buf[5], buf[4]);

	return TRUE;
}

bool_t compass_sensor_self_test(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6] = {0, };
    osel_int_status_t status = 0;
    OSEL_ENTER_CRITICAL(status);

    if((reg_a_v != REG_A_VALUE_FOR_SELF_TEST)
       || (reg_b_v != REG_B_VALUE_FOR_SELF_TEST))
    {
        reg_a_v = REG_A_VALUE_FOR_SELF_TEST;
        reg_b_v = REG_B_VALUE_FOR_SELF_TEST;
        if(FALSE == hmc5983_reg_config(REG_A_VALUE_FOR_SELF_TEST, REG_B_VALUE_FOR_SELF_TEST))
        {
            LED_CLOSE(BLUE);
            OSEL_EXIT_CRITICAL(status);
            return FALSE;
        }
    }

	if(FALSE == hmc5983_reg_write(MODE_REG_ADDR, MODE_REG_VALUE_FOR_SINGLE))
	{
        OSEL_EXIT_CRITICAL(status);
		return FALSE;
	}

	HMC5983_WAIT_DRDY();

	for(int i=0; i<6; i++)
	{
		if(FALSE == hmc5983_reg_read(X_MSB_REG_ADDR + i, &buf[i]))
		{
            OSEL_EXIT_CRITICAL(status);
			return FALSE;
		}
	}
	*x = BUILD_UINT16(buf[1], buf[0]);
	*z = BUILD_UINT16(buf[3], buf[2]);
	*y = BUILD_UINT16(buf[5], buf[4]);
    OSEL_EXIT_CRITICAL(status);
	return TRUE;
}

bool_t sensor_init(void)
{
    return compass_sensor_init();
}

int16_t sensor_detect(int16_t *max_data)
{
	int16_t x = 0;
	int16_t y = 0;
	int16_t z = 0;

	int16_t x1 = 0;
	int16_t y1 = 0;
	int16_t z1 = 0;

	compass_sensor_detect(&x, &y, &z);
	x1=ABS(x);
	y1=ABS(y);
	z1=ABS(z);
	*max_data = (x1>=y1)?x1:y1;
	*max_data = (*max_data>=z1)?*max_data:z1;
	return z;
}

void sensor_drdy_state_handler(void)
{
    hmc5983_drdy = TRUE;
}







