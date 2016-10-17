/**
 * @brief       : 主要是加速度传感器驱动层和控制层之间的接口处理 
 *
 * @file        : hal_acc_sensor.c
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
#include <sensor_accelerated.h>
#include <hal_acc_sensor.h>
//#include <hal_board.h>

extern device_info_t device_info;

/**
 *
 * @brief 加速度芯片初始化, 完成引脚设置，中断配置
 *
 * @return 初始化成功与否
 *   - 1 初始化成功
 *   - 0 初始化失败
 */
bool_t hal_acc_init(void)
{
	buzzer_init();
    
    accelerated_sensor_init();
    return TRUE;
}

/**
 *
 * @brief 读取加速度数据 
 *
 * @param[in]  acc_data_t acc_data_buf[]  用来存放读取的加速度数据
 * @param[in]  max_size 在加速度传感器fifo中最多可以存放的数据条数
 *
 * @return 读取数据的个数
 */
uint8_t hal_acc_get_detect_data(acc_data_t acc_data_buf[],
                            uint8_t      max_size)
{
    int16_t pacc_x = 0 ;
    int16_t pacc_y = 0 ;
    int16_t pacc_z = 0 ;
    int16_t offset_z = 0;
    uint8_t retvalue = 0;
    uint8_t fifo_status_reg = 0;
    uint8_t entries = 0;//采样点个数

    //首先获得当前FIFO中有多少个采样点，读取寄存器0x39  
    // ADXL345_FIFO_STATUS  [D5~D0]    
    retvalue = adxl345_read_register(ADXL345_FIFO_STATUS,&fifo_status_reg);
    DBG_ASSERT(I2C_OK == retvalue __DBG_LINE);

    entries = fifo_status_reg&0x3f;
    if (entries > max_size)
    {
        entries = max_size;
    }

    offset_z = device_info.offset_z;
    for (uint8_t i = 0;i < entries;i++)
    {
        adxl345_get_xyz( &pacc_x , &pacc_y , &pacc_z);
        acc_data_buf[i].x = pacc_x;
        acc_data_buf[i].y = pacc_y;
        acc_data_buf[i].z = (pacc_z + offset_z);
    }

    return entries;   
}