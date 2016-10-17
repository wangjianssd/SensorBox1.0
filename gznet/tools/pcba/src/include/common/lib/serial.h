/**
 * @brief       : 
 *
 * @file        : serial.h
 * @author      : gang.cheng
 *
 * Version      : v0.0.1
 * Date         : 2015/5/7
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <data_type_def.h>
#include <hal_uart.h>

#define SERIAL_NUM                      UART_NUM


#define SERIAL_1                        HAL_UART_1
#define SERIAL_2                        HAL_UART_2
#define SERIAL_3                        HAL_UART_3

#define SERIAL_SD_LEN_MAX               2
#define SERIAL_LD_LEN_MAX               2
#define SERIAL_ED_LEN_MAX               1

typedef struct _serial_reg_t_
{
    struct
    {
        uint8_t len;
        uint8_t pos;
        uint8_t data[SERIAL_SD_LEN_MAX];
        bool_t  valid;
    } sd; //start delimiter

    struct
    {
        uint8_t len;
        uint8_t pos;
        uint8_t little_endian;
        bool_t  valid;
    } ld; //length describe

    struct
    {
        uint16_t len_max;
        uint16_t len_min;
    } argu;

    struct
    {
        uint8_t len;
        uint8_t data[SERIAL_ED_LEN_MAX];
        bool_t  valid;
    } ed;

    bool_t echo_en;
    void (*func_ptr)(void);
} serial_reg_t;

uint8_t serial_read(uint8_t id, void *buffer, uint16_t len);

void serial_write(uint8_t id, uint8_t *const string, uint16_t len);

void lock_serial(uint8_t serial_id);

void unlock_serial(uint8_t serial_id);

void serial_fsm_init(uint8_t serial_id);

bool_t serial_reg(uint8_t id, serial_reg_t serial_reg);

void serial_unreg(uint8_t id);
#endif

/**@}*/

