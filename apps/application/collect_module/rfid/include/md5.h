 /**
 * @brief       : md5算法
 *
 * @file        : md5.h
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/10/27
 *
 * Change Logs  : 源自mbed_md5源代码，修改代码规范及注释
 *
 * Date        Version      Author      Notes
 * 2015/10/27  v0.0.1    cuihongpeng    first version
 */
#ifndef __MD5_H
#define __MD5_H

#include <gznet.h> 
#include <stddef.h>
//#include <data_type_def.h>

/**
 * @brief 	MD5 context structure
 */
typedef struct
{
    uint32_t total[2];          /**< number of bytes processed  */
    uint32_t state[4];          /**< intermediate digest state  */
    uint8_t buffer[64];   		/**< data block being processed */
} md5_context_t;

/**
 * @brief		   Output = MD5(input buffer)
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   MD5 checksum result
 */
void md5(const uint8_t *input, size_t ilen, uint8_t output[16]);

#endif
