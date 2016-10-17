#include <gznet.h> 
//#include <driver.h>
//#include <wsnos.h>
#include <sensor_humiture.h>

#define USE_SI7021

#ifdef USE_SI7021
#define DEV_I2C_ADDR 								0x40//0x80
#endif

#ifdef USE_SI7021
#define SHT20_ADDRESS   							DEV_I2C_ADDR
#else
#define SHT20_ADDRESS   							0x40
#endif


#define TRIGGER_T_MEASUREMENT_HOLD_COMMAND          0xE3
#define TRIGGER_RH_MEASUREMENT_HOLD_COMMAND         0xE5
#define TRIGGER_T_MEASUREMENT_NO_HOLD_COMMAND       0xF3
#define TRIGGER_RH_MEASUREMENT_NO_HOLD_COMMAND      0xF5
#define WRITE_REG_COMMAND                           0xE6
#define READ_REG_COMMAND                            0xE7
#define SOFTWARE_RESET                              0xFE
#ifdef USE_SI7021
#define CMD_WRITE_REGISTER_2						0x50
#define CMD_READ_REGISTER_2							0x10
#define CMD_WRITE_REGISTER_3						0x51
#define CMD_READ_REGISTER_3							0x11
#define CMD_WRITE_COEFFICIENT						0xC5
#define CMD_READ_COEFFICIENT						0x84
#define DEV_ID 										0x15
#endif


#define HUMITURE_RESOLUTION_AT_12_BIT               40
#define HUMITURE_RESOLUTION_AT_8_BIT                700
#define T_RESOLUTION_AT_14_BIT                      10
#define T_RESOLUTION_AT_12_BIT                      40

#define HUMITURE_HIGH_RESOLUTION                    0
#define HUMITURE_LOW_RESOLUTION                     1

#ifndef HUMITURE_RESOLUTION
#define HUMITURE_RESOLUTION                         HUMITURE_HIGH_RESOLUTION
#endif

#define HUMITURE_I2C_WRITE_START()                  \
    do                                              \
    {                                               \
        UCB2I2CSA = SHT20_ADDRESS;                  \
        while(UCB2CTL1 & UCTXSTP);                  \
        UCB2CTL1 |= UCTR;                           \
        UCB2CTL1 |= UCTXSTT;                        \
    } while(__LINE__ == -1)

#define HUMITURE_I2C_READ_START()                   \
    do                                              \
    {                                               \
        uint8_t i;                                  \
        while(UCB2CTL1 & UCTXSTP);                  \
        UCB2CTL1 &= ~UCTR;                          \
        UCB2CTL1 |= UCTXSTT;                        \
        i = UCB2RXBUF;                              \
        i = i;                                      \
    } while(__LINE__ == -1)

#define HUMITURE_I2C_WAIT_ADDR_ACK()                while(UCB2CTL1 & UCTXSTT)
#define HUMITURE_I2C_SEND_STOP_BIT()                UCB2CTL1 |= UCTXSTP
#define HUMITURE_I2C_WAIT_STOP()                    while(UCB2CTL1 & UCTXSTP)

#define HUMITURE_I2C_SEND_CHAR(x)                   \
    do                                              \
    {                                               \
        while(!(UCB2IFG & UCTXIFG));                \
        UCB2TXBUF = x;                              \
    } while(__LINE__ == -1)

#define HUMITURE_I2C_RECCEIVE_CHAR(x)               \
    do                                              \
    {                                               \
        while(!(UCB2IFG & UCRXIFG));                \
        x = UCB2RXBUF;                              \
    } while(__LINE__ == -1)

#define HUMITURE_I2C_IS_NO_ACK()                    (UCB2IFG & UCNACKIFG)

static void sht20_i2c_lock_init(void)
{
    //P9.6 作为模拟scl，输出9个信号
    P9SEL &= ~BIT6;
	P9DIR |= BIT6;
    P9OUT |= BIT6;
    // 主设备模拟SCL，从高到低，输出9次，使得从设备释放SDA
    for(uint8_t i=0;i<9;i++)
    {
        P9OUT |= BIT6;
        delay_ms(1);
        P9OUT &= ~BIT6;
        delay_ms(1);
    }
}

static void sht20_i2c_init(void)
{
    sht20_i2c_lock_init();
	
    //sda
    P9SEL |= BIT5; 

    P9SEL |= BIT6;
    P9DIR |= BIT6;

    UCB2CTL1 |= UCSWRST;
    UCB2CTL0 = UCMST + UCMODE_3 + UCSYNC ; // I2C Master, synchronous mode
    UCB2CTL1 |= UCSSEL_2; // Use SMCLK, keep SW reset
    UCB2BR0 = 40; // fSCL = SMCLK/40 = 200kHz
    UCB2BR1 = 0;
    UCB2CTL0 &= ~UCSLA10; //7 bits
    //UCB2I2CSA = SHT20_ADDRESS;      // IIC address write when iic start
    UCB2CTL1 &= ~UCSWRST;
}

static bool_t sht20_read_user_reg(uint8_t *val)
{
    HUMITURE_I2C_WRITE_START();
    HUMITURE_I2C_SEND_CHAR(READ_REG_COMMAND);
    HUMITURE_I2C_WAIT_ADDR_ACK();

    HUMITURE_I2C_READ_START();
    HUMITURE_I2C_WAIT_ADDR_ACK();
    HUMITURE_I2C_SEND_STOP_BIT();
    HUMITURE_I2C_RECCEIVE_CHAR(*val);
    HUMITURE_I2C_WAIT_STOP();
    if (HUMITURE_I2C_IS_NO_ACK())
    {
        return FALSE;
    }
    return TRUE;
}

static bool_t sht20_write_user_reg(uint8_t val)
{
    HUMITURE_I2C_WRITE_START();
    HUMITURE_I2C_SEND_CHAR(WRITE_REG_COMMAND);
    HUMITURE_I2C_WAIT_ADDR_ACK();

    HUMITURE_I2C_SEND_CHAR(val);
    HUMITURE_I2C_SEND_STOP_BIT();
    HUMITURE_I2C_WAIT_STOP();
    if (HUMITURE_I2C_IS_NO_ACK())
    {
        return FALSE;
    }
    return TRUE;
}



/**************************************************************************/
bool_t humiture_sensor_init(void)
{
    DBG_ASSERT((HUMITURE_RESOLUTION == HUMITURE_HIGH_RESOLUTION)
               || (HUMITURE_RESOLUTION == HUMITURE_LOW_RESOLUTION) __DBG_LINE);
    uint8_t val;
    sht20_i2c_init();
    delay_ms(15);
    if (HUMITURE_RESOLUTION == HUMITURE_LOW_RESOLUTION)
    {
        if (FALSE == sht20_read_user_reg(&val))
        {
            return FALSE;
        }
        val &= ~BIT7;
        val |= BIT0;
        return sht20_write_user_reg(val);
    }
    else
    {
        if (FALSE == sht20_read_user_reg(&val))
        {
            return FALSE;
        }
        val &= ~(BIT7 + BIT0);
        return sht20_write_user_reg(val);
    }
}

bool_t humiture_sensor_read_h(uint16_t *pval)
{
    uint8_t rh_hi;
    uint8_t rh_lo;
    uint8_t check;

    HUMITURE_I2C_WRITE_START();
    HUMITURE_I2C_SEND_CHAR(TRIGGER_RH_MEASUREMENT_HOLD_COMMAND);
    HUMITURE_I2C_WAIT_ADDR_ACK();
    HUMITURE_I2C_READ_START();
    HUMITURE_I2C_WAIT_ADDR_ACK();
    HUMITURE_I2C_RECCEIVE_CHAR(rh_hi);
    HUMITURE_I2C_RECCEIVE_CHAR(rh_lo);
    HUMITURE_I2C_SEND_STOP_BIT();
    HUMITURE_I2C_RECCEIVE_CHAR(check);
    HUMITURE_I2C_WAIT_STOP();
    if (HUMITURE_I2C_IS_NO_ACK())
    {
        return FALSE;
    }
    check = check;
    *pval = (((uint16_t)rh_hi) << 8) | rh_lo;
    *pval &= ~0x0003;
    return TRUE;
}

bool_t humiture_sensor_read_t(uint16_t *pval)
{
    uint8_t t_hi;
    uint8_t t_lo;
    uint8_t check;

    HUMITURE_I2C_WRITE_START();
    HUMITURE_I2C_SEND_CHAR(TRIGGER_T_MEASUREMENT_HOLD_COMMAND);
    HUMITURE_I2C_WAIT_ADDR_ACK();
    HUMITURE_I2C_READ_START();
    HUMITURE_I2C_WAIT_ADDR_ACK();
    HUMITURE_I2C_RECCEIVE_CHAR(t_hi);
    HUMITURE_I2C_RECCEIVE_CHAR(t_lo);
    HUMITURE_I2C_SEND_STOP_BIT();
    HUMITURE_I2C_RECCEIVE_CHAR(check);
    HUMITURE_I2C_WAIT_STOP();
    if (HUMITURE_I2C_IS_NO_ACK())
    {
        return FALSE;
    }
    check = check;
    *pval = (((uint16_t)t_hi) << 8) | t_lo;
    *pval &= ~0x0003;
    return TRUE;
}

