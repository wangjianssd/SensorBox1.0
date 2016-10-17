#ifndef __HAL_ACC_SENSOR_H
#define __HAL_ACC_SENSOR_H

/**
 *
 * @brief 加速度芯片初始化, 完成引脚设置，中断配置
 *
 * @return 初始化成功与否
 *   - 1 初始化成功
 *   - 0 初始化失败
 */
bool_t hal_acc_init(void);

/**
 * @breif 加速度数据的结构定义 
 *
 */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
}acc_data_t;
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
                                uint8_t    max_size);

#endif