#include "ad5252.h"
#include <msp430.h>
#include "driver.h"

#define I2C_WRITE_START() 					        \
    do                                              \
    {                                               \
        UCB0I2CSA = AD5252_ADDRESS;                 \
        WHILE(UCB0CTL1 & UCTXSTP);					\
    	UCB0CTL1 |= UCTR;							\
    	UCB0CTL1 |= UCTXSTT;						\
    } while(__LINE__ == -1)

#define I2C_READ_START() 					        \
    do                                              \
    {                                               \
		uint8_t recv = 0;							\
        WHILE(UCB0CTL1 & UCTXSTP);					\
    	UCB0CTL1 &= ~UCTR;							\
    	UCB0CTL1 |= UCTXSTT;						\
		recv = UCB0RXBUF;							\
		recv = recv;								\
    } while(__LINE__ == -1)

#define I2C_WAIT_ADDR_ACK()						    \
	{												\
		WHILE(UCB0CTL1 & UCTXSTT);					\
	}

#define I2C_SEND_STOP_BIT()			UCB0CTL1 |= UCTXSTP

#define I2C_WAIT_STOP()				while(UCB0CTL1 & UCTXSTP)

#define I2C_SEND_CHAR(x)                            \
    do                                              \
    {                                               \
		WHILE(!(UCB0IFG & UCTXIFG));				\
    	UCB0TXBUF = (x);							\
    } while(__LINE__ == -1)

#define I2C_RECCEIVE_CHAR(x)                        \
    do                                              \
    {                                               \
		WHILE(!(UCB0IFG & UCRXIFG));				\
		(x) = UCB0RXBUF;							\
    } while(__LINE__ == -1)

#define I2C_IS_NO_ACK()					(UCB0IFG & UCNACKIFG)


#define I2C_WRITE_EN()                  (P3OUT |= BIT3)
#define I2C_WRITE_DIS()                 (P3OUT &= ~BIT3)
        
static fp32_t ab_res_actual = AB_RES;
/******************************************************************************/
static void iic_lock_init(void)
{
    //P3.2 作为模拟scl，输出9个信号
    P3SEL &= ~BIT2;
    P3DIR |= BIT2;
    P3OUT |= BIT2;
    // 主设备模拟SCL，从高到低，输出9次，使得从设备释放SDA
    for(uint8_t i=0;i<9;i++)
    {
        P3OUT |= BIT2;
        delay_ms(1);
        P3OUT &= ~BIT2;
        delay_ms(1);
    }
}

static void iic_mode_init(void)
{
    P3SEL |= BIT1;
    P3SEL |= BIT2;

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC ; // I2C Master, synchronous mode
    UCB0CTL1 |= UCSSEL_2; // Use SMCLK, keep SW reset
    UCB0BR0 = 40; // fSCL = SMCLK/40 = 200kHz
    UCB0BR1 = 0;
    UCB0CTL0 &= ~UCSLA10; //7位地址
    UCB0CTL1 &= ~UCSWRST;
}

static void iic_port_init(void)
{
    P3SEL &= ~BIT3;
    P3DIR |= BIT3;
    P3OUT &= ~BIT3;    // wp == 0 ,写保护
}


static bool_t i2c_write(uint8_t addr, uint8_t val)
{
    I2C_WRITE_EN();
    
	I2C_WRITE_START();
	I2C_SEND_CHAR(addr);
	I2C_WAIT_ADDR_ACK();
	I2C_SEND_CHAR(val);
	while(!(UCB0IFG & UCTXIFG));
	I2C_SEND_STOP_BIT();
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        I2C_WRITE_DIS();
		return FALSE;
	}
    
    I2C_WRITE_DIS();
	return TRUE;
}

static bool_t i2c_read(uint8_t addr, uint8_t *val)
{    
    I2C_WRITE_EN();
    
	I2C_WRITE_START();
	I2C_SEND_CHAR(addr);
	I2C_WAIT_ADDR_ACK();
	while(!(UCB0IFG & UCTXIFG));
	I2C_SEND_STOP_BIT();
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        I2C_WRITE_DIS();
		return FALSE;
	}

	I2C_READ_START();
	I2C_WAIT_ADDR_ACK();
	I2C_SEND_STOP_BIT();
	I2C_RECCEIVE_CHAR(*val);
	I2C_WAIT_STOP();
	if(I2C_IS_NO_ACK())
	{
        I2C_WRITE_DIS();
		return FALSE;
	}
    
    I2C_WRITE_DIS();
	return TRUE;
}
        
bool_t ad5252_init(void)
{
    iic_lock_init();
    iic_mode_init();
    iic_port_init();
    
    // power 作为LTC3115的RUN启动
    P3SEL &= ~BIT0;
	P3DIR |= BIT0;
    P3OUT &= ~BIT0;             // 低电平表示不工作
    
//    ad5252_quick_cmd(RES1_RELOAD);
    return 0;
}


bool_t AD5252_WaitForDevice(void)
{
    uint8_t address = AD5252_CMD_REG | AD5252_INSTRUCTION(AD5252_NOP);
    uint8_t data = 0;             
    return i2c_write(address, data); 
}
      
bool_t AD5252_WriteRDAC(uint8_t rDacAddress, uint8_t data)
{   
    uint8_t address = rDacAddress;
    return i2c_write(address,data);
}

bool_t AD5252_ReadRDAC(uint8_t rDacAddress,uint8_t *data)
{
    uint8_t address = AD5252_INSTRUCTION(AD5252_NOP) | rDacAddress;         
    return i2c_read(address,data); 
}

bool_t AD5252_WriteEEMEM(uint8_t memAddress, uint8_t data)
{
    uint8_t delay = 0;
    uint8_t delay1 = 0;
    uint8_t address = AD5252_EE_RDAC | memAddress;
    bool_t rval;
    rval = i2c_write(address, data);
    
    if (rval)
    {
        for(delay1 = 0; delay1 < 100; delay1++)
        {
            for(delay = 0; delay < 255; delay++)
            {
                _NOP();_NOP();_NOP();
                _NOP();_NOP();_NOP();
            }
        }
        rval = AD5252_WaitForDevice();
        return rval;
    }
    else
    {
        return rval;
    }	
}


bool_t AD5252_ReadEEMEM( uint8_t memAddress,uint8_t *data)
{
     uint8_t address = AD5252_EE_RDAC | memAddress;
     return i2c_read(address,data);
}

void AD5252_SendCommand(unsigned char instruction, unsigned char rDacAddress)
{
    uint8_t address = AD5252_CMD_REG | AD5252_INSTRUCTION(instruction) | rDacAddress;    
    i2c_write(address, 0);
    //The AD5252_RESTORE command leaves the device in the EEMEM read power state 
    //, which consumes power. Issuing the NOP command will return the device to 
    //its idle state.
    if(instruction == AD5252_RESTORE)
    {
        address = AD5252_CMD_REG | 
                     AD5252_INSTRUCTION(AD5252_NOP) |
                     rDacAddress;    
        i2c_write(address, 0);
    }
}


/*****************************************************************************/
static fp32_t ad5252_get_tolerance(void)
{
    uint8_t integer = 0;
    uint8_t decimal = 0;
    AD5252_ReadEEMEM(AD5252_TOLR1INT, &integer);
    AD5252_ReadEEMEM(AD5252_TOLR1DEC, &decimal);
    
    uint8_t sign = integer >> 7;
    uint8_t inte_val = integer & 0x7F;
    fp32_t dec_val = ((fp32_t)(decimal))/(2^8);
    
    if(sign == 0)
    {
        ab_res_actual = AB_RES + (AB_RES/100 * (inte_val+dec_val));
    }
    else
    {
        ab_res_actual = AB_RES - (AB_RES/100 * (inte_val+dec_val));
    }

    return ab_res_actual;
}

/*
*          D
*   Rwb = --- * Rab + 75
*         256
*/
void set_ad5252_wb_res(fp32_t res)
{
    fp32_t tes_temp = res - CONST_RES_OFFSET;
    tes_temp = tes_temp*DIV_CONST;
    tes_temp = (uint32_t)(tes_temp/ad5252_get_tolerance());
    if(tes_temp >255)
    {
        tes_temp = 255;
    }
    
    bool_t result = FALSE;
//    AD5252_WriteRDAC(AD5252_RDAC1, 0);
    result = AD5252_WriteRDAC(AD5252_RDAC1, (uint8_t)tes_temp);
    result = result;

    AD5252_SendCommand(AD5252_STORE, AD5252_RDAC1);
    result = AD5252_ReadRDAC(AD5252_RDAC1, (uint8_t *)&tes_temp);
    
    tes_temp = tes_temp;
}
/**
*                  R169
*   Vccout = ------------- * Vf + 1V
*               Ru + R171
*
* Ru 最大为100K，
*/
void vout_set(uint8_t vccout)
{
    if(vccout < 3)
    {
        return;
    }
    
    if( vccout > 25)
    {
        return;
    }
    
    fp32_t res = 0;
    res = R169/(vccout-1) - R171;
    
    set_ad5252_wb_res(res);
    
    P3OUT |= BIT0;      //!< LTC3115启动 
}




