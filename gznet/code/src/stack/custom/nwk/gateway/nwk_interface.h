/***************************************************************************
* @brief        : this
* @file         : nwk_interface.h
* @version      : v0.1
* @author       : gang.cheng
* @date         : 2015-09-07
* @change Logs  :
* Date        Version      Author      Notes
* 2015-09-07      v0.1      gang.cheng    first version
***************************************************************************/
#ifndef __NWK_INTERFACE_H__
#define __NWK_INTERFACE_H__

typedef enum
{
    /* NWK */
    MAC2NWK_PRIM_EVENT  = ((NWK_TASK_PRIO<<8) | 0x01),
    APP2NWK_PRIM_EVENT,
    NWK_CYCLE_EVENT,
    NWK_QUERY_TIMEOUT_EVENT,
    NWK_QUERY_RESP_EVENT
} nwk_task_sig_enum_t;

PROCESS_NAME(mac2nwk_process);

//*< 网关的默认NUI，用网关的网络短地址替代
#define GATEWAY_NUI             (0x0000000000000001ull) 

/**
 * @brief 创建nwk层任务，初始化资源以及定时器
 */
void nwk_init(void);

/**
 * @brief 启动网络
 */
void nwk_run(void);

/**
 * @brief 关闭网络
 */
void nwk_stop(void);

/**
 * @brief 数据发送接口
 *
 * @param[in] nui 发送数据到达的目的地址
 * @param[in] data 指向要发送的数据指针
 * @param[in] len 指向要发送的数据长度
 * 
 * @return 实际发送的数据长度
 */
int8_t nwk_send(uint64_t nui, uint8_t *const data, uint8_t len);

typedef struct 
{
    //!< NWK发送数据后返回确认的回调函数
    void (*nwk_data_confirm) (sbuf_t *sbuf);   
    
    //!< NWK收到数据后通知APP的回调函数
    void (*nwk_data_indicate) (uint8_t *const data, uint8_t len);    
    
    //*< 网络状态指示
    void (*nwk_join_indicate)(bool_t res);
} nwk_dependent_t;

void nwk_dependent_cfg(const nwk_dependent_t *cfg);

void nwk2app_register(void *cb);	//数据从nwk到app的注册接口
#endif
