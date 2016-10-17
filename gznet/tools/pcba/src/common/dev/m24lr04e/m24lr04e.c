/**
 * @brief       : 
 *
 * @file        : m24lr64e.c
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
#include <inc/m24lr04e.h>
#include <i2c.h>
#include <m24lr64e_arch.h>

#define SECTION_SIZE        0x80            //一个扇区大小128byte
#define SECTION_AREA_SIZE   0x20            //一个扇区大小32个区域
#define AREA_SIZE           0x04            //一个区域大小4byte
#define NFC_MAX_ADDR        0x2000          //NFC最大地址

DBG_THIS_MODULE("m24lr64e")

static rfid_interupt_cb_t rfid_interrupt_cb = NULL;

static bool_t m24lr64e_read(uint8_t mode,
                      uint8_t *const data_buf,
                       uint8_t const data_len,
                       uint16_t const word_addr)
{
    uint8_t read_size = 0;
    read_size = I2C_DRIVER.recv(mode, data_buf, data_len, word_addr);

    return read_size;
}

//连续写nfc一块扇区内存储区的数据
static bool_t write_nfc_area_data(uint8_t mode,uint8_t *const data_buf, 
                                    uint8_t const data_len,
                                    uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    uint8_t *data_temp = data_buf;
    uint16_t word_addr_temp = word_addr;
    //该起始地址所在区域还剩多少字节可写
    uint8_t remain_data_pro = AREA_SIZE - (word_addr_temp % AREA_SIZE);
    uint8_t remain_data_end = 0;
    if(data_len >= remain_data_pro)
    {
        //最后一个区域
        remain_data_end = (data_len-remain_data_pro)%AREA_SIZE;
    }
    if(remain_data_pro != 0)
    {
        if(data_len <= remain_data_pro)
        {
            remain_data_pro = data_len;
        }
        if(RIGHT == I2C_DRIVER.send(mode, data_temp, remain_data_pro, word_addr_temp))
        {
            data_temp += remain_data_pro;
            word_addr_temp += remain_data_pro;
            delay_ms(5);
        }
    }
    //写完开始的后还剩几个完整区域
    uint8_t data_area_numb = (data_len-remain_data_pro)/AREA_SIZE;
    for(uint8_t i=0; i<data_area_numb; i++)
    {
        if(RIGHT == I2C_DRIVER.send(mode, data_temp, AREA_SIZE, word_addr_temp))
        {
            data_temp += AREA_SIZE;
            word_addr_temp += AREA_SIZE;
            delay_ms(5);
        }
        else
        {
            return ERROR;
        }
    }
	
    if(remain_data_end != 0)                                      //写最后的区域
    {
        I2C_DRIVER.send(mode, data_temp, remain_data_end, word_addr_temp);
        delay_ms(5);
    }
	
    return RIGHT;
}

static bool_t m24lr64e_write(uint8_t mode,
                      uint8_t *const data_buf,
                       uint8_t const data_len,
                       uint16_t const word_addr)
{
    DBG_ASSERT(word_addr <= NFC_MAX_ADDR __DBG_LINE);
    DBG_ASSERT(data_buf != NULL __DBG_LINE);
    DBG_ASSERT(data_len != 0 __DBG_LINE);
    
    uint8_t *data_temp = data_buf;
    uint16_t word_addr_temp = word_addr;
    uint8_t remain_data_area_pro = SECTION_SIZE - (word_addr_temp % SECTION_SIZE);  
    uint8_t remain_data_area_end = 0;
    if(data_len >= remain_data_area_pro)
    {
        remain_data_area_end = (data_len-remain_data_area_pro)%SECTION_SIZE;
    }
    if(remain_data_area_pro != 0)
    {
        if(data_len <= remain_data_area_pro)
        {
            remain_data_area_pro = data_len;
        }
        if(RIGHT == write_nfc_area_data(mode, data_temp, remain_data_area_pro,
                                        word_addr_temp))
        {
            data_temp  += remain_data_area_pro;
            word_addr_temp += remain_data_area_pro;
        }
    }
    //写完开始的后还剩几个整扇区
    uint8_t data_section_numb = (data_len-remain_data_area_pro)/SECTION_SIZE;
    for(uint8_t i=0;i<data_section_numb; i++)
    {
        if(RIGHT == write_nfc_area_data(mode, data_temp, SECTION_SIZE, word_addr_temp))
        {
            data_temp += SECTION_SIZE;
            word_addr_temp += SECTION_SIZE;
        }
        else
        {
            return ERROR;
        }
    }
	
    if(remain_data_area_end != 0)
    {
        if(ERROR == write_nfc_area_data(mode, data_temp, remain_data_area_end, word_addr_temp))
        {
            return ERROR;
        }   
    }
	
    return RIGHT;
}

static void m24lr64e_reg_cfg(void)
{
  	uint8_t oldreg = 0x00;
	uint8_t newreg = 0x00;
  	I2C_DRIVER.recv(NFC_ADDRESS_E2_1,&oldreg, 1, NFC_INT_BUSY_ADDR);
	newreg = oldreg | 0x08;//RF WIP/BUSY 设置为1，即WRITE模式
	I2C_DRIVER.send(NFC_ADDRESS_E2_1,&newreg, 1, NFC_INT_BUSY_ADDR);
}

bool_t m24lr64e_deinit(void)
{
 	return TRUE;
}

bool_t m24lr64e_init(void)
{	
    m24lr64e_vcc_open();
    I2C_DRIVER.init();//I2C模块的初始化:包括使用前I2C的释放操作和寄存器的配置

    m24lr64e_port_init();//配置硬件接口
	
	uint8_t i2c_add = NFC_ADDRESS_E2_1;
	m24lr64e_reg_cfg();//rfid寄存器配置	
	i2c_add = NFC_ADDRESS_E2_0;

    m24lr64e_int_cfg();//中断配置
    return TRUE;
}

static void m24lr64e_int_cb_reg(rfid_interupt_cb_t cb)
{
    if (cb != NULL)
    {
        rfid_interrupt_cb = cb;
    }
}

static bool_t m24lr04e_get_value(uint8_t type, void *value)
{
    switch(type)
    {
    case NFC_ID:
        m24lr64e_read(NFC_ADDRESS_E2_1, value, 4, NFC_ID_ADDR);
        break;
        
    defaut:
        break;
    }
    return TRUE;
}

const extern rfid_driver_t rfid_device_driver = 
{
    m24lr64e_init,
    m24lr64e_deinit,
    m24lr64e_write,
    m24lr64e_read,
    m24lr04e_get_value,
    m24lr64e_int_cb_reg,
};
