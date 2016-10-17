 /**
 * @brief       : 感知箱锁控制的抽象层
 *
 * @file        : hal_elec_lock.c
 * @author      : xxx
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
#include <hal_elec_lock.h>
#include <elec_lock.h>
//#include <driver.h>

/**
* @brief 锁初始化
*
*/
void hal_elec_lock_init(void)
{
  elec_lock_init();
}

/**
* @brief 开锁操作 
*
*/
bool_t hal_elec_lock_open(void)
{
  return elec_lock_open();
}

/**
* @brief 闭锁操作 
*
*/
void hal_elec_lock_close(void)
{
  elec_lock_close();
}


/**
* @brief return elec lock state:TRUE- open, FALSE- close
*
*/
bool_t hal_elec_lock_state(void)
{
  return elec_lock_state();
}