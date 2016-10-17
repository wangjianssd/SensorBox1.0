/**
 * @brief       : 
 *
 * @file        : hal_sd.h
 * @author      : shenghao.xu
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      shenghao.xu    some notes
 */
#pragma once
#include <data_type_def.h>
#if ((NODE_TYPE == NODE_TYPE_HOST) || (NODE_TYPE == NODE_TYPE_SMALL_HOST))
#include <ff.h>
#endif
/* 参数配置 */
#define FSYNC				(1u)	//冲刷开关
#define FSYNC_NUM			100		//写入100条数据后冲刷SD卡

/* 文件模式  */
#define F_READ				0x01	//指定对象的读权限
#define F_WRITE				0x02	//指定对象的写权限
#define F_CREATE			0x04	//创建一个新文件,如果存在创建失败
#define F_CREATE_ALWAYS    	0x08	//创建一个新文件,如果存在覆盖
#define F_OPEN				0x10	//打开一个文件,如果不存在创建一个新的文件


/**
 * 设备挂载
 *
 * @author xushenghao (2013-3-6)
 *
 * @param vol 		逻辑驱动器号被安装/卸载驱动器输入0
 * @param fs 		文件系统对象的指针(NULL为卸载)
 *
 * @return bool_t
 */
bool_t hal_fmount(uint8_t vol, FATFS *fs);

/**
 * 打开或创建一个文件
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param path 		文件路径
 * @param mode 		模式
 *
 * @return bool_t
 */
bool_t hal_fopen(FIL *fp, const char_t *path, uint8_t mode);

/**
 * 关闭并写入文件
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 *
 * @return bool_t
 */
bool_t hal_fclose(FIL *fp);

/**
 *
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param buf 		数据存储缓冲区
 * @param bufsize 	数据存储缓冲区大小
 * @param hr_num 	已读取的字节数
 *
 * @return bool_t
 */
bool_t hal_freadline(FIL *fp, void *buf ,uint16_t bufsize, uint16_t *hr_num);

/**
 * 写入文件+换行符
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param buf 		要写的数据源
 * @param w_num 	要写的字节数
 * @param hw_num 	已写的字节数
 *
 * @return bool_t
 */
bool_t hal_fwriteline(FIL *fp, const void *buf, uint16_t w_num, uint16_t *hw_num);

/**
 * 读取文件
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param buf 		数据存储缓冲区
 * @param r_num 	要读取的字节数
 * @param hr_num 	已读取的字节数
 *
 * @return bool_t
 */
bool_t hal_fread(FIL *fp, void *buf, uint16_t r_num, uint16_t *hr_num);

/**
 * 写入文件
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param buf 		要写的数据源
 * @param w_num 	要写的字节数
 * @param hw_num 	已写的字节数
 *
 * @return bool_t
 */
bool_t hal_fwrite(FIL *fp, const void *buf, uint16_t w_num, uint16_t *hw_num);

/**
 * 冲刷SD卡，即在SD卡中将数据真正的写入(断电保护)
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 *
 * @return bool_t
 */
bool_t hal_fsync(FIL *fp);

/**
 * 读/写指针移动
 *
 * @author xushenghao (2013-3-6)
 *
 * @param fp 		文件指针
 * @param ofs 		偏倚位(ofs=fp->fsize到文件末尾)
 *
 * @return bool_t
 */
bool_t hal_flseek(FIL *fp, uint32_t ofs);

/**
 * 磁盘可用容量（KB）
 *
 * @author xushenghao (2013-3-7)
 *
 * @param fs 		文件系统对象的指针
 * @param free 		磁盘可用容量
 *
 * @return bool_t
 */
bool_t hal_fgetfree(FATFS *fs, uint32_t *free);


/**
 * 删除文件/文件夹(禁止删除被打开的文件)
 *
 * @author xushenghao (2013-3-7)
 *
 * @param path 		文件路径
 *
 * @return bool_t
 */
bool_t hal_fremove(const char_t *path);

/**
 * 设置文件/文件夹时间戳
 *
 * @author xushenghao (2013-3-7)
 *
 * @param timestamp 时间戳
 *
 * @return bool_t
 */
bool_t hal_ftimestamp(sd_time_t *timestamp);

/**
 * 删除文件/目录
 *
 * @author xushenghao (2013-3-8)
 *
 * @param path 	文件路径/目录路径
 *
 * @return bool_t
 */
bool_t hal_fremovedir(const char_t *path);

bool_t hal_sd_init();
