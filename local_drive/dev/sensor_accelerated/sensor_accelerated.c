/**
 * @brief       : 加速度传感器模块驱动
 *
 * @file        : sensor_accelerated.c
 * @author      : zhangzhan
 * @version     : v0.1
 * @date        : 2015/9/15
 *
 * Change Logs  : 
 *
 * Date           Version      Author      Notes
 * - 2015/9/15    v0.0.1      zhangzhan    文件初始版本
 */
#include <gznet.h> 
//#include <driver.h>
//#include <wsnos.h>
#include <sensor_accelerated.h>
#include <hal_acc_sensor.h>

//extern acc_config_t acc_config_1;

#define ADXL345_DATA_OUT_REG_NUM    6 //X Y Z三轴6个寄存器

/**
 *
 * @brief 加速度传感器读寄存器值的操作
 *
 * @param[in]  uint8_t reg_add    寄存器地址
 * @param[in]  uint8_t *pvalue    寄存器内容
 *
 * @return 读成功与否
 *   - 0 读成功
 *   - 1 读失败
 */
bool_t adxl345_read_register( uint8_t reg_add , uint8_t *pvalue)
{
    UCB2I2CSA = ADXL345_ADDRESS;
    UCB2CTL1 |= UCTR;                 // 写模式
    UCB2CTL1 |= UCTXSTT;              // 发送启动位和写控制字节

    UCB2TXBUF = reg_add;              // 发送数据，必须要先填充TXBUF
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB2IFG & UCTXIFG))
    {
        if( UCB2IFG & UCNACKIFG )       // 若无应答 UCNACKIFG=1
        {
            return I2C_FAIL;
        }
    }

    UCB2CTL1 &= ~UCTR;                // 读模式
    UCB2CTL1 |= UCTXSTT;              // 发送启动位和读控制字节

    while(UCB2CTL1 & UCTXSTT);        // 等待UCTXSTT=0
    // 若无应答 UCNACKIFG = 1

    UCB2CTL1 |= UCTXSTP;              // 先发送停止位

    while(!(UCB2IFG & UCRXIFG));      // 读取数据，读取数据在发送停止位之后
    *pvalue = UCB2RXBUF;

    while( UCB2CTL1 & UCTXSTP );

    return I2C_OK ;
}

/**
 *
 * @brief 向寄存器中写入单个字节数据
 *
 * @param[in]  uint8_t reg_add    寄存器地址
 * @param[in]  uint8_t reg_value   寄存器内容
 *
 * @return 写成功与否
 *   - 0 写成功
 *   - 1 写失败
 */
bool_t adxl345_write_register( uint8_t reg_add , uint8_t reg_value)
{
    UCB2I2CSA = ADXL345_ADDRESS;
    while( UCB2CTL1 & UCTXSTP );
    UCB2CTL1 |= UCTR;                 // 写模式
    UCB2CTL1 |= UCTXSTT;              // 发送启动位

    UCB2TXBUF = reg_add;             // 发送寄存器地址
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB2IFG & UCTXIFG))
    {
        if( UCB2IFG & UCNACKIFG )       // 若无应答 UCNACKIFG=1
        {
            return I2C_FAIL;
        }
    }

    UCB2TXBUF = reg_value;            // 发送寄存器内容
    while(!(UCB2IFG & UCTXIFG));      // 等待UCTXIFG=1

    UCB2CTL1 |= UCTXSTP;
    while(UCB2CTL1 & UCTXSTP);        // 等待发送完成

    return I2C_OK ;
}

/**
 *
 * @brief 从传感器寄存器连续读取多个字节
 *
 * @param[in]  uint8_t RegAddr    寄存器地址
 * @param[in]  uint8_t Len     读取字节长度
 * @param[out] uint8_t *pregbuf    寄存器内容
 *
 * @return 读成功与否
 *   - 0 读成功
 *   - 1 读失败
 */
bool_t adxl345_read_buff(uint8_t reg_add , uint8_t *pregbuf , uint8_t  len)
{
    UCB2I2CSA = ADXL345_ADDRESS;
    while( UCB2CTL1 & UCTXSTP );
    UCB2CTL1 |= UCTR;                 // 写模式
    UCB2CTL1 |= UCTXSTT;              // 发送启动位和写控制字节

    UCB2TXBUF = reg_add;             // 发送数据
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB2IFG & UCTXIFG))
    {
        if( UCB2IFG & UCNACKIFG )       // 若无应答 UCNACKIFG=1
        {
            return I2C_FAIL;
        }
    }

    UCB2CTL1 &= ~UCTR;                // 读模式
    UCB2CTL1 |= UCTXSTT;              // 发送启动位和读控制字节

    while(UCB2CTL1 & UCTXSTT);        // 等待UCTXSTT=0
    // 若无应答 UCNACKIFG = 1

    for(uint8_t i = 0; i < (len - 1); i++)
    {
        while(!(UCB2IFG & UCRXIFG));    // 读取数据
        *pregbuf++ = UCB2RXBUF;
    }

    UCB2CTL1 |= UCTXSTP;              // 在接收最后一个字节之前发送停止位

    while(!(UCB2IFG & UCRXIFG));      // 读取数据
    *pregbuf = UCB2RXBUF;

    while( UCB2CTL1 & UCTXSTP );

    return I2C_OK;
}

/**
 *
 * @brief 获得三轴加速度传感器各轴检测结果，转换结果为mg
 *
 * @param[out]  int16_t *pacc_x      X轴结果
 * @param[out]  int16_t *pacc_y      Y轴结果
 * @param[out] int16_t *pacc_z      Z轴结果
 *
 * @return 读成功与否
 *   - 0 读成功
 *   - 1 读失败
 */
bool_t adxl345_get_xyz( int16_t *pacc_x , int16_t *pacc_y , int16_t *pacc_z)
{
    uint8_t accbuf[ADXL345_DATA_OUT_REG_NUM] = {0};
    uint8_t ret = I2C_FAIL;               // 读写返回值
    
    ret = adxl345_read_buff( 0x32 , accbuf , ADXL345_DATA_OUT_REG_NUM );
    DBG_ASSERT(I2C_OK == ret __DBG_LINE);

    *pacc_x = (accbuf[1] << 8 ) | accbuf[0];
    *pacc_y = (accbuf[3] << 8 ) | accbuf[2];
    *pacc_z = (accbuf[5] << 8 ) | accbuf[4];
    return I2C_OK;
}

/**
 *
 * @brief 使用前I2C的释放操作,即防止I2C死锁
 *
 *
 * @return 无
 */
static void accelerated_i2c_lock_init(void)
{    
    //P9.6 作为模拟scl，输出9个信号
    P9SEL &= ~BIT6;// P9.6置成IO口模式
	P9DIR |= BIT6; //P9.6做为输出
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

/**
 *
 * @brief I2C模块的初始化:包括使用前I2C的释放操作和寄存器的配置
 *
 *
 * @return 无
 */
static void accelerated_iic_init(void)
{
    accelerated_i2c_lock_init();
    
    P9SEL |= BIT5;
    P9SEL |= BIT6;
    P9DIR |= BIT6;

    UCB2CTL1 |= UCSWRST;                    //软件复位使能
    UCB2CTL0 = UCMST + UCMODE_3 + UCSYNC ;  // I2C主机模式
    UCB2CTL1 |= UCSSEL_2;                   // 选择SMCLK
    UCB2BR0 = 80;                // I2C时钟约 100hz，F(i2c)=F(smclk)/BR,smclk=8m
    UCB2BR1 = 0;
    UCB2CTL0 &= ~UCSLA10;                   // 7位地址模式
    //UCB2I2CSA = ADXL345_ADDRESS;            // ADXL345
    UCB2CTL1 &= ~UCSWRST;       //软件清除UCSWRST可以释放USCI,使其进入操作状态
}

/**
 *
 * @brief 该函数在系统启动后传感器的寄存器初始配置操作
 *
 * @return 无
 */
static void accelerated_settings(void)
{ 
    uint8_t device_id = 0;
    adxl345_read_register(ADXL345_DEVID,&device_id);
    DBG_ASSERT(ADXL345_ID == device_id __DBG_LINE);
    // ADXL345_REG_POWER_CTL[3]=0设置成待机模式,即清除测试位
    adxl345_write_register(ADXL345_POWER_CTL,0x00); 
    //BW_RATE[4]=1；即设置LOW_POWER位低功耗
    //BW_RATE[3][2][1][0]=0x07，即设置输出速率12.5HZ，Idd=34uA
    //普通，100hz
    adxl345_write_register(ADXL345_BW_RATE,0x07);                                                     
    //adxl345_write_register(ADXL345_BW_RATE,0x0A);  
   //THRESH_TAP: 比例因子为62.5mg/lsb  建议大于3g 
    //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff   3.5g=0x38
    adxl345_write_register(ADXL345_THRESH_TAP,0x38); 
                                                     
    adxl345_write_register(ADXL345_OFSX,0x00);       // Z轴偏移量
    adxl345_write_register(ADXL345_OFSY,0x00);       // Y轴偏移量
    adxl345_write_register(ADXL345_OFSZ,0x00);       // Z轴偏移量
    //DUR:比例因子为625us/LSB，建议大于10ms
    //6.25ms=0x0A //12.5ms=0x14。
    adxl345_write_register(ADXL345_DUR,0x14); 
    //Latent:比例因子为1.25ms/LSB，建议大于20ms
    //2.5ms=0x02，，20ms=0x10，，，25ms=0x14
    adxl345_write_register(ADXL345_LATENT,0x14);    
    //window:比例因子为1.25ms/LSB，建议大于80ms 
    //10ms=0x08，，80ms=0x40
    adxl345_write_register(ADXL345_WINDOW,0x41);    
    //THRESH_ACT:比例因子为62.5mg/LSB，
    //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff,,,//1.5g=0x18
    adxl345_write_register(ADXL345_THRESH_ACT,0x18);
    //THRESH_INACT:比例因子为62.5mg/LSB   
    //1g=0x10  //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff
    adxl345_write_register(ADXL345_THRESH_INACT,0x10);
                                                    
    //TIME_INACT:比例因子为1sec/LSB      //1s=0x01                                           
    adxl345_write_register(ADXL345_TIME_INACT,0x05);
    //设置为直流耦合：当前加速度值直接与门限值比较，以确定是否运动或静止  
    //x,y,z参与检测活动或静止
    adxl345_write_register(ADXL345_ACT_INACT_CTL,0x77);
    //用于自由落地检测，比例因子为62.5mg/LSB   
    //建议设置成300mg~600mg（0x05~0x09）
    adxl345_write_register(ADXL345_THRESH_FF,0x06);       
    //所有轴的值必须小于此设置值，才会触发中断;比例因子5ms/LSB   
    //建议设成100ms到350ms(0x14~~0x46),200ms=0x28
    adxl345_write_register(ADXL345_TIME_FF,0x28);         
    //TAP_AXES:单击/双击轴控制寄存器； 
    //1）不抑制双击  2）使能x.y,z进行敲击检查
    adxl345_write_register(ADXL345_TAP_AXES,0x07);  
    // 中断使能   
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0]
    adxl345_write_register(ADXL345_INT_ENABLE,0xfc); 
                                                    
    //INT_MAC中断映射：任意位设为0发送到INT1位，，设为1发送到INT2位
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0] 
    adxl345_write_register(ADXL345_INT_MAP,0x40);   
    
    //1）SELF_TEST[7];2)SPI[6]; 3)INT_INVERT[5]：设置为0中断高电平有效，
    // 数据输出格式  高电平触发
    adxl345_write_register(ADXL345_DATA_FORMAT,0x0B);
    //adxl345_write_register(ADXL345_DATA_FORMAT,0x2B);// 数据输出格式  低电平触发         
                                                    //反之设为1低电平有效    rang[1][0]
    //设置 FIFO模式
    adxl345_write_register(ADXL345_FIFO_CTL,0xBF);
    // 进入测量模式
    //1)链接位[5]    2)AUTO_SLEEP[4]   3)测量位[3]  4)休眠位[2]  5)唤醒位[1][0]
    adxl345_write_register(ADXL345_POWER_CTL,0x28);
    uint8_t int_source; 
    adxl345_read_register(ADXL345_INT_SOURCE, &int_source);                                                 
}

/**
 *
 * @brief 电源打开
 *
 * @return 无
 */
void acc_power_open()
{
//    P1SEL &= ~(BIT6);
//    P1DIR |= (BIT6);
//    P1OUT |= (BIT6);    
}

/**
 *
 * @brief 加速度传感器芯片初始化接口
 *
 * @return 无
 */
void accelerated_sensor_init(void)
{    
    acc_power_open();//打开电源    
    accelerated_iic_init();//I2C模块的初始化:包括使用前I2C的释放操作和寄存器的配置
    accelerated_settings();//寄存器配置
}


void buzzer_init(void)
{
    P1SEL &= ~BIT6;
	P1DIR |= BIT6;                       		// P1.6  output    
    P1OUT &= ~BIT6;
}


void buzzer_on(void)
{
    P1OUT |= BIT6;
}


void buzzer_off(void)
{
    P1OUT &= ~BIT6;
}
