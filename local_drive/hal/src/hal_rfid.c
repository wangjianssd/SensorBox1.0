 /**
 * @brief       : RFID 中间层代码
 *
 * @file        : hal_rfid.c
 * @author      : cuihongpeng
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1    cuihongpeng    first version
 */

#include "hal_rfid.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief rfid初始化
 * @param[in] 空
 * @return TRUE or FALSE
 */
bool_t hal_rfid_init(void)
{
	m24lr64e_init();
	return TRUE;
}

/**
* @brief 写入rfid数据
*
* @param addr:     需要写入数据的地址
* @param buff:     需要写入数据的长度
* @param buff:     需要写入数据的内容
*
* @return TRUE or FALSE
*/
bool_t hal_rfid_write_info(uint16_t addr, uint8_t len, uint8_t *buff)
{
	if(write_rfid_data(buff, len, addr) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

/**
* @brief 读取rfid数据
*
* @param addr:     需要读取数据的地址
* @param len:      需要读取数据的长度
* @param buff:     需要读取数据的内容
*
* @return TRUE or FALSE
*/
bool_t hal_rfid_read_info(uint16_t addr, uint8_t len, uint8_t *buff)
{
	if(read_rfid_data(buff, len, addr) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

/**
* @brief 读取rfid更改索引
*
* @param rfid_modify_flag: rfid更改索引结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_read_modify_flag(rfid_modify_flag_t* rfid_modify_flag)
{
	hal_rfid_read_info(ADDRESS_OPERATION_MODIFY_INFO, 4,(uint8_t*)(&rfid_modify_flag->operation_modify_flag));
	//hal_rfid_read_info(ADDRESS_APPLICATION_MODIFY_INFO, 4, (uint8_t*)(&rfid_modify_flag->application_modify_flag));
	return TRUE;
}

/**
* @brief 清除rfid更改索引
*
* @param rfid_modify_flag: rfid更改索引结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_clear_modify_flag(void)
{
	uint32_t temp = 0;
	hal_rfid_write_info(ADDRESS_OPERATION_MODIFY_INFO, 4, (uint8_t*)&temp);
	//hal_rfid_write_info(ADDRESS_APPLICATION_MODIFY_INFO, 4, (uint8_t*)&temp);
	return TRUE;
}

/**
* @brief 更新货物信息
*
* @param operation_info_t: rfid货物信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_cargo_info(cargo_info_t* cargo_info)
{
	hal_rfid_read_info(ADDRESS_CARGO_LEN_INFO, 4, (uint8_t *)&cargo_info->len);
	cargo_info->info = malloc(cargo_info->len);
	memset(cargo_info->info, 0, cargo_info->len);
	hal_rfid_read_info(ADDRESS_CARGO_INFO, cargo_info->len, (uint8_t *)cargo_info->info);
	free(cargo_info->info);
	return TRUE;
}


/**
* @brief 更新操作信息
*
* @param operation_info_t: rfid操作信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_operation_info(operation_info_t* operation_info)
{
	rfid_modify_flag_t rfid_modify_flag;
	memset(&rfid_modify_flag, 0, sizeof(rfid_modify_flag));
	hal_rfid_read_modify_flag(&rfid_modify_flag);
	if(rfid_modify_flag.operation_modify_flag != 0)
	{
		for(uint8_t index = 0; index < 4; index++)
		{
			if(rfid_modify_flag.operation_modify_flag & 0x01)
			{
				if(index <= 2)
				{
					hal_rfid_read_info(ADDRESS_LOCK_STATUS_INFO + (ADDRESS_EACH_BLOCK_SIZE*index), 4, (uint8_t*)(operation_info) + (index*4));	
				}
				else if(index == 3)
				{
					hal_rfid_read_info(ADDRESS_LOCK_STATUS_INFO + (ADDRESS_EACH_BLOCK_SIZE*index), 1, (uint8_t*)(operation_info) + (index*4));
				}
				else
				{
					
				}
			}
			rfid_modify_flag.operation_modify_flag = rfid_modify_flag.operation_modify_flag >> 1;
		}
	}
	return TRUE;
}

/**
* @brief 更新profile信息
*
* @param profile_info_t: profile信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_profile_info(profile_info_t* profile_info)
{
	hal_rfid_read_info(ADDRESS_PROFILE_LEN_INFO, 2, (uint8_t*)profile_info->length);
	hal_rfid_read_info(ADDRESS_PROFILE_ID_INFO, 20 + profile_info->length, (uint8_t*)profile_info);
	return TRUE;
}


/**
* @brief 更新应用信息
*
* @param profile_info_t: 应用信息结构体
* @return TRUE/FALSE
*/
bool_t hal_rfid_updata_application_info(application_info_t* application_info)
{
	rfid_modify_flag_t rfid_modify_flag;
	memset(&rfid_modify_flag, 0, sizeof(rfid_modify_flag));
	hal_rfid_read_modify_flag(&rfid_modify_flag);
	if(rfid_modify_flag.application_modify_flag != 0)
	{
		for(uint8_t index = 0; index < 4; index++)
		{
			if(rfid_modify_flag.application_modify_flag & 0x01)
			{
				if(index <= 2)
				{
					hal_rfid_read_info(ADDRESS_ACTIVATION_INFO + (ADDRESS_EACH_BLOCK_SIZE*index), 4, (uint8_t*)application_info + (index*4));
				}
				else
				{
					hal_rfid_updata_profile_info(&(application_info->profile_info));
				}
			}
			rfid_modify_flag.application_modify_flag = rfid_modify_flag.application_modify_flag >> 1;
		}
	}
	return TRUE;
}

/**
* @brief 读取rfid读取操作用户ID
*
* @param type: 读取类型
* @param id: 用户ID
*/
bool_t hal_rfid_read_user_id(user_type_e type, uint32_t* id)
{
	switch(type)
	{
		case OPERATION_USER:
		hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4, (uint8_t*)id);
		break;
		
		case APPLICATION_USER:
		hal_rfid_read_info(ADDRESS_APPLICATION_USER_ID_INFO, 4, (uint8_t*)id);
		break;
		
		default :
		break;
	}
	return TRUE;
}

/**
* @brief 查看感知箱是否被激活
*
* @return TRUE:已经被激活 FALSE:未被激活
*/
bool_t hal_rfid_access_status(void)
{
	uint32_t temp = 0;
	hal_rfid_read_info(ADDRESS_ACCESS_INFO, 4, (uint8_t*)&temp);
	if(0 == temp)
	{
		return FALSE;
	}
	return TRUE;
}

rfid_modify_flag_t rfid_modify_flag;
operation_info_t operation_info;
cargo_info_t cargo_info;
/**
* @brief 测试使用
*/
void hal_nfc_test(void)
{
    memset(&rfid_modify_flag, 0, sizeof(rfid_modify_flag));
    memset(&operation_info, 0, sizeof(operation_info));
    memset(&cargo_info, 0, sizeof(cargo_info));
    hal_rfid_read_modify_flag(&rfid_modify_flag);
	hal_rfid_updata_operation_info(&operation_info);
	hal_rfid_clear_modify_flag();
	hal_rfid_access_status();
    hal_rfid_updata_cargo_info(&cargo_info);
}