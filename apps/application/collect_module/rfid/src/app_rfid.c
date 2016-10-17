 /**
 * @brief       : rfid的业务处理流程
 *
 * @file        : app_rfid.c
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#include <gznet.h>
#include <hal_rfid.h>
#include <app_rfid.h>
//#include <hal_timer.h>
//#include <data_type_def.h>
//#include <osel_arch.h>
//#include <hal_board.h>
#include <app_send.h>
#include <app_frame.h>
#include <hal_bh1750.h>
#include <app_light_sensor.h>
//#include <mac_moniter.h>
//#include <ssn.h>
#include <app_gps.h>
#include <md5.h>
#include <hal_elec_lock.h>
#include <elec_lock.h>
#include <app_humiture.h>
#include <gps.h>
#include "app_task.h"
#include "app_acc_sensor.h"

#define BOX_NFC_ECHO_COUNT       3
#define BOX_NFC_WAIT_TIME_LIMIT  2000 //2S
#define BOX_NFC_ECHO_TIME_LIMIT  500 //0.5S
     
#define BOX_GPRS_STOP_TIME_LIMIT    120000

__no_init uint32_t box_access_valid_key;
uint8_t box_operation_status_flag = FALSE;//0 代表感知箱不处于用户操作阶段；1 代表感知箱处于用户操作阶段
//static uint8_t box_lock_status_flag = FALSE;//0代表开箱，1代表闭箱

extern device_info_t device_info;
extern osel_etimer_t nfc_wait_isr_timer;   //*< nfc中断延迟定时器

extern sensor_info_t box_sensor;
extern gps_simple_info_t box_location;
bool_t if_get_password = FALSE;


extern osel_etimer_t gprs_test_stop_timer;
void start_gprs_test_stop_timer(void)
{
    osel_etimer_arm(&gprs_test_stop_timer,(BOX_GPRS_STOP_TIME_LIMIT/OSEL_TICK_PER_MS),0);
}
extern bool_t gprs_cur_power_flag;
void box_gprs_stop_timer_handle(void *arg)
{
    P1SEL &=~BIT3;//Close power
    P1DIR |= BIT3;
    P1OUT |= BIT3;    
    gprs_cur_power_flag = FALSE;
}



//nfc中断延迟定时器：2s
static void start_wait_isr_timer(void)
{
    osel_etimer_arm(&nfc_wait_isr_timer,(BOX_NFC_WAIT_TIME_LIMIT/OSEL_TICK_PER_MS),0);
}

void m24lr64e_int_proc(void)
{
	start_wait_isr_timer();
}

uint64_t box_id = NODE_ID;//感知箱ID
extern uint8_t box_uid[8];
void box_nfc_init(void)
{
    hal_rfid_read_info(ADDRESS_DEVICE_ID_INFO_BASE, sizeof(box_id), (uint8_t*)(&box_id));
    device_info = hal_board_info_get();
    osel_memcpy(device_info.device_id, (uint8_t *)&box_id, sizeof(uint64_t));
    box_uid[0] = device_info.device_id[7];
    box_uid[1] = device_info.device_id[6];
    box_uid[2] = device_info.device_id[5];
    box_uid[3] = device_info.device_id[4];
    box_uid[4] = device_info.device_id[3];
    box_uid[5] = device_info.device_id[2];
    box_uid[6] = device_info.device_id[1];
    box_uid[7] = device_info.device_id[0];
    //osel_memcpy(box_uid,(uint8_t *)&box_id, sizeof(uint64_t));
}

osel_etimer_t nfc_no_lock_timer;

void stop_nfc_no_lock_timeout(void)
{
    osel_etimer_disarm(&nfc_no_lock_timer);
}

/**
*@brief nfc 获取授权之后的操作定时器超时
*/
void nfc_no_lock_timeout_timer_cb(void)
{
    elec_lock_init();
    buzzer_cycle_timer_start(1000);
    if_get_password = FALSE;//此处未开锁，所以不能得到授权
}
/**
*@brief nfc 获取授权之后的操作定时器
*/
static void nfc_no_lock_timeout_set(uint32_t ticks)
{
    stop_nfc_no_lock_timeout();
    osel_etimer_arm(&nfc_no_lock_timer, (ticks/OSEL_TICK_PER_MS), 0); 

}

#if 1
void box_nfc_int_handle(void *arg)
{
    uint8_t ret = FALSE;
    uint32_t ack_data = 0;
    box_frame_t box_frame;
    rfid_modify_flag_t rfid_modify_flag;
    uint32_t operation_modify_flag = 0;
        
    //先读取是否有操作或应用配置信息变更标志位
    ret = hal_rfid_read_modify_flag(&rfid_modify_flag);
    DBG_ASSERT(TRUE == ret __DBG_LINE);
    operation_modify_flag = rfid_modify_flag.operation_modify_flag;
    
    uint64_t box_id_temp = 0;
    hal_rfid_read_info(ADDRESS_BOX_ID_INFO, 8*sizeof(uint8_t), (uint8_t*)(&box_id_temp));
    uint32_t use_id = 0;
    hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4*sizeof(uint8_t), (uint8_t*)(&use_id)); 
    uint32_t opt_timer = 0;
    hal_rfid_read_info(ADDRESS_OPERATION_TIMER_INFO, 4*sizeof(uint8_t), (uint8_t*)(&opt_timer)); 
    fp32_t location_lat = 0;
    hal_rfid_read_info(ADDRESS_LOCATION_LAT_INFO, 4*sizeof(uint8_t), (uint8_t*)(&location_lat)); 
    fp32_t location_long = 0;
    hal_rfid_read_info(ADDRESS_LOCATION_LONG_INFO, 4*sizeof(uint8_t), (uint8_t*)(&location_long));
    
    box_location.longitude = location_long;
    box_location.latitude = location_lat;
    
    
    if (operation_modify_flag)//如果是操作信息
    {
        uint32_t phone_operation = 0;
        hal_rfid_read_info(ADDRESS_PHONE_OPERATION_INFO, 4*sizeof(uint8_t), (uint8_t*)(&phone_operation));
        
        if (box_id_temp == box_id)
        {
            if (phone_operation == 0x01)//开锁处理
            {
                if_get_password = TRUE;                
                buzzer_cycle_timer_start(200);

                #ifdef USE_EXTERN_LOCK   
                elec_lock_open();

                extern_lock_timeout_set(EXTERN_LOCK_AUTO_LOCK_DELAY); // 10s timeout lock #endif
                #else
                nfc_no_lock_timeout_set(10000); //10s timeout            
                #endif
            }
            else if (phone_operation == 0x02)//开启预警
            {
                box_operation_status_flag = TRUE;
                box_enable_acc_alg();
                box_humiture_init();            
            }
            else if (phone_operation == 0x03)//放弃感知箱使用权
            {
                elec_lock_init();
                humiture_stop_timer();
                box_disable_acc_alg();
                
                box_frame.frame_type = BOX_SENSOR_FRAME;
                box_frame.box_type_frame_u.sensor_info.len = 0x00;
                box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOSE_RIGHTS;
                box_frame.box_type_frame_u.sensor_info.user_id = box_sensor.user_id;
                box_frame.box_type_frame_u.sensor_info.timestamp = box_sensor.timestamp;                
                box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
                box_send();
                
                box_operation_status_flag = FALSE;
                extern void start_gprs_test_stop_timer(void);
                start_gprs_test_stop_timer();
                
                //开启锁的中断使能
#ifdef USE_Fingerprints
                    elec_lock_access();
#endif
            }
            //给手机反馈操作成功指令应答
            ack_data = 1;
            hal_rfid_write_info(ADDRESS_BOX_ACK_INFO, 4*sizeof(uint8_t), (uint8_t*)&ack_data);
        }
        else
        {
            //ID 不符合
            ack_data = 2;
            hal_rfid_write_info(ADDRESS_BOX_ACK_INFO, 4*sizeof(uint8_t), (uint8_t*)&ack_data);            
        }
    }
    else
    {
        ;//其他操作不做处理
    }
    
    hal_rfid_clear_modify_flag();
    hal_wdt_clear(16000);
}
#endif

#if 0
void box_nfc_int_handle(void *arg)
{
    uint8_t ret = FALSE;
    uint8_t profile_md5[16] = {0x00};
    uint8_t cargo_md5[16] = {0x00};
    application_info_t application_info;
    box_frame_t box_frame;
    rfid_modify_flag_t rfid_modify_flag;
    uint32_t application_modify_flag = 0;
    uint32_t operation_modify_flag = 0;
    user_type_e user_type = OPERATION_USER;
    cargo_info_t cargo_info;
    user_log_info_t user_log;
        
    //先读取是否有操作或应用配置信息变更标志位
    ret = hal_rfid_read_modify_flag(&rfid_modify_flag);
    DBG_ASSERT(TRUE == ret __DBG_LINE);
    application_modify_flag = rfid_modify_flag.application_modify_flag;
    operation_modify_flag = rfid_modify_flag.operation_modify_flag;
    
    if (application_modify_flag)//如果是应用配置信息
    {
        //只可能出现010204,020408,
        uint8_t app_filed_change = 0;//0x01激活变更；0x02用户ID变更；0x04操作时间变更；0x08profile变更
        app_filed_change = ((uint8_t)application_modify_flag & 0xFF);
        if ((app_filed_change == 0x0E))//profile配置
        {
            //保存profile信息到flash
            ret = hal_rfid_updata_application_info(&application_info);
            DBG_ASSERT(TRUE == ret __DBG_LINE);
            device_info = hal_board_info_get();
            
            osel_memcpy(device_info.profile_id,application_info.profile_info.id,4);            
            device_info.acceleration_threshold = application_info.profile_info.profile_option.acceleration_threshold;
            device_info.heartbeat_interval_in_room = application_info.profile_info.profile_option.heartbeat_interval_in_room;
            device_info.heartbeat_interval_out_room = application_info.profile_info.profile_option.heartbeat_interval_out_room;
            device_info.humidity_threshold = application_info.profile_info.profile_option.humidity_threshold;
            device_info.illumination_threshold = application_info.profile_info.profile_option.illumination_threshold;
            device_info.location_info_interval_in_room = application_info.profile_info.profile_option.location_info_interval_in_room;
            device_info.location_info_interval_out_room = application_info.profile_info.profile_option.location_info_interval_out_room;
            device_info.temperature_threshold = application_info.profile_info.profile_option.temperature_threshold;
            device_info.box_cfg_timer = application_info.timer;
            device_info.box_app_id = application_info.id;
            device_info.box_activation = application_info.activation;
            
            //计算profile的MD5值
            md5((uint8_t*)(&application_info.profile_info),sizeof(application_info.profile_info),profile_md5);
            osel_memcpy(device_info.profile_md5,profile_md5,16);
            hal_board_info_save(&device_info, TRUE);            
        }
        else if ((app_filed_change == 0x07))//激活操作
        {            
            //1设备启动帧
            //更新配置信息，读取用户ID和时间搓
            ret = hal_rfid_updata_application_info(&application_info);
            DBG_ASSERT(TRUE == ret __DBG_LINE);
            device_info = hal_board_info_get();//此处读取profile ID 和profile MD5
                        
            box_frame.frame_type = BOX_REG_DEVICE_FRAME;
            box_frame.box_type_frame_u.box_device_info.user_id = application_info.id;
            box_frame.box_type_frame_u.box_device_info.timestamp = application_info.timer;
            osel_memcpy(box_frame.box_type_frame_u.box_device_info.profile_id,device_info.profile_id,4);
            
            osel_memcpy(box_frame.box_type_frame_u.box_device_info.profile_md5,
                        device_info.profile_md5,16);    
//            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,TRUE);            
            
            //2.打开gps电源,等待发送gps位置信息帧
//            box_gps_vcc_open();
            
            //搜寻SSN网络或gprs网络,如果成功发送设备启动帧,否则激活失败
//           uint8_t monitor_timer_len = 0;
//           monitor_timer_len = mac_moniter_interval_time_get();
//           ssn_open(ssn_open_cb, monitor_timer_len);             
        }
        else
        {
            ;//此种情况全部是手机配置异常
        }
    }
    else if (operation_modify_flag)//如果是操作信息
    {
        //规定：只可能出现010204,020408,020410,
        uint8_t oper_filed_change = 0;//0x01开关锁变更；0x02用户ID变更；0x04操作时间变更；0x08感知箱使用权变更;0x10货物信息变更
        oper_filed_change = ((uint8_t)operation_modify_flag & 0xFF);
        //读取操作时间
        uint32_t operation_timer = 0;
        hal_rfid_read_info(ADDRESS_OPERATION_TIMER_INFO, 4*sizeof(uint8_t), (uint8_t*)(&operation_timer));
                
        if (oper_filed_change == 0x0E)//获取使用权限的处理
        {
            uint32_t box_access_status = 0;
            //读取感知箱使用权限变更信息
            hal_rfid_read_info(ADDRESS_ACCESS_INFO, 4*sizeof(uint8_t), (uint8_t*)(&box_access_status));
            if (box_access_status)
            {
                uint32_t user_id = 0;
                ret = hal_rfid_read_user_id(user_type,&user_id);//读取用户ID
                DBG_ASSERT(TRUE == ret __DBG_LINE);
                //保存用户ID
                device_info = hal_board_info_get();
                device_info.box_app_id = user_id;
                hal_board_info_save(&device_info, TRUE);
                //擦除nfc中的用户名和时间戳
                uint32_t temp = 0;
                hal_rfid_write_info(ADDRESS_OPERATION_USER_ID_INFO, 4, (uint8_t*)&temp);
                hal_rfid_write_info(ADDRESS_OPERATION_TIMER_INFO, 4, (uint8_t*)&temp);
                //向后台发送获得使用权事件
//                box_frame.frame_type = BOX_SENSOR_FRAME;
//                box_frame.box_type_frame_u.sensor_info.type = SENSOR_ACQQUIRE_RIGHTS;
//                box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
//				box_send();
                
                //保存获取感知箱使用权信息到nfc
                user_log.id = user_id;
                user_log.timer = operation_timer;
                user_log.log_type = 0x04;//获取感知箱使用权
                hal_rfid_write_info(ADDRESS_USER_INFO, 9, (uint8_t*)(&user_log));                
                box_operation_status_flag = TRUE;
                //打开温湿度周期上报定时器
                box_humiture_init();
            }
            else//归还感知箱使用权
            {
                if (box_operation_status_flag)
                {
                    uint32_t user_id = 0;
                    ret = hal_rfid_read_user_id(user_type,&user_id);//读取用户ID
                    DBG_ASSERT(TRUE == ret __DBG_LINE);
                    //从flash中读取保存的用户ID
                    device_info = hal_board_info_get();                    
                    if (user_id == device_info.box_app_id)//如果验证用户通过
                    {
                        //mcu控制电子锁进入初始化状态
                        elec_lock_init();
                        //货物信息清空
//                        uint32_t temp = 0;
//                        hal_rfid_updata_cargo_info(&cargo_info);
//                        hal_rfid_write_info(ADDRESS_CARGO_INFO, cargo_info.len, (uint8_t*)&temp);                        
                        //向后台发送归还箱子使用权事件
                        box_frame.frame_type = BOX_SENSOR_FRAME;
                        box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOSE_RIGHTS;
                        box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
						box_send();
                        
                        //保存获归还感知箱使用权信息到nfc
//                        user_log.id = user_id;
//                        user_log.timer = operation_timer;
//                        user_log.log_type = 0x05;//获取感知箱使用权
//                        hal_rfid_write_info(ADDRESS_USER_INFO, 9, (uint8_t*)(&user_log));                
                        box_operation_status_flag = FALSE;
                        //两分钟后关闭gprs电源gprs_test_stop_timer
                        start_gprs_test_stop_timer();
                        //关闭温湿度周期上报定时器
                        box_humiture_set_period(0);                        
                    }
                }
            }

        }
        else if (oper_filed_change == 0x16)//货物信息变更
        {
            if (box_lock_status_flag)//开锁状态
            {
                //读取用户ID信息
                uint32_t user_id_status = 0;
                hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4*sizeof(uint8_t), (uint8_t*)(&user_id_status));
                device_info = hal_board_info_get();
                if (device_info.box_app_id == user_id_status)//用户ID符合
                {
                    //上报货物变更信息
                    hal_rfid_updata_cargo_info(&cargo_info);
                    //保存货物信息MD5值到cargo_md5[]
                    md5(cargo_info.info,cargo_info.len,cargo_md5);
                    //上报感知箱货物变更信息
                    box_frame.frame_type = BOX_SENSOR_FRAME;
                    box_frame.box_type_frame_u.sensor_info.type = SENSOR_CARGO;
                    osel_memcpy(box_frame.box_type_frame_u.sensor_info.content,cargo_md5,16*sizeof(uint8_t));
//                    box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
                    
                    //保存货物变更信息到nfc
                    user_log.id = user_id_status;
                    user_log.timer = operation_timer;
                    user_log.log_type = 0x03;//获取感知箱使用权
                    hal_rfid_write_info(ADDRESS_USER_INFO, 9, (uint8_t*)(&user_log));                
                    box_operation_status_flag = TRUE;                    
                }                
            }
        }
        else if (oper_filed_change == 0x07)//开关锁操作
        {
            uint32_t lock_status = 0;
            //读取锁的开关信息
            hal_rfid_read_info(ADDRESS_LOCK_STATUS_INFO, 4*sizeof(uint8_t), (uint8_t*)(&lock_status));

            if (lock_status)//开锁操作
            {
                if (box_operation_status_flag)
                {
                    //读取用户ID信息
                    uint32_t user_id_status = 0;
                    hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4*sizeof(uint8_t), (uint8_t*)(&user_id_status));
                    device_info = hal_board_info_get(); 
                    if (device_info.box_app_id == user_id_status)//用户ID符合
                    {
                        //关闭光照度检测功能
                        box_light_sensor_close();
                        //开锁
                        hal_elec_lock_open();
                        box_lock_status_flag = TRUE;
                        //擦除nfc中的用户名和时间戳
                        uint32_t temp = 0;
                        hal_rfid_write_info(ADDRESS_OPERATION_USER_ID_INFO, 4, (uint8_t*)&temp);
                        hal_rfid_write_info(ADDRESS_OPERATION_TIMER_INFO, 4, (uint8_t*)&temp);
                        //上报感知箱开锁信息
                        box_frame.frame_type = BOX_SENSOR_FRAME;
                        box_frame.box_type_frame_u.sensor_info.type = SENSOR_UNLOCK;
                        box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,TRUE);
						box_send();
                        
                        //保存开锁信息到nfc
    //                    user_log.id = user_id_status;
    //                    user_log.timer = operation_timer;
    //                    user_log.log_type = 0x01;//获取感知箱使用权
    //                    hal_rfid_write_info(ADDRESS_USER_INFO, 9, (uint8_t*)(&user_log));                     
                    }
                    else//用户ID不符合的异常
                    {
                        ;//异常如果通知?
                    }                                

    /**< debug >*/
    #if 0
                    /**< 异常日志操作测试样本 >*/
                    uint8_t abnormal_info[7] = 
                    {
                        0x01,0x00,              /**< 长度 >*/
                        0xaa,0xaa,0xaa,0xaa,    /**< 时间戳 >*/
                        0x01                    /**< 温度超限 >*/
                    };
                    /**< 用户日志操作测试样本 >*/
                    uint8_t user_info[11] = 
                    {
                        0x01,0x00,              /**< 长度 >*/
                        0xd2,0x04,0x00,0x00,    /**< 用户ID >*/
                        0xaa,0xaa,0xaa,0xaa,    /**< 时间戳 >*/
                        0x01                    /**< 开锁操作 >*/
                    };
                    /**< 保存异常日志 >*/
                    hal_rfid_write_info(ADDRESS_ABNORMAL_INFO, 7, abnormal_info);
                    /**< 保存用户日志 >*/
                    hal_rfid_write_info(ADDRESS_USER_INFO, 11, user_info);
    #endif   
    /**< End of debug >*/                    
                }                
            }
            else//闭锁操作
            {
//                if (box_lock_status_flag)//之前是开锁状态
//                {
                    if (box_operation_status_flag)//正处于用户操阶段
                    {
                        //读取用户ID信息
                        uint32_t user_id_status = 0;
                        hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4*sizeof(uint8_t), (uint8_t*)(&user_id_status));
                        device_info = hal_board_info_get();
                        if (device_info.box_app_id == user_id_status)//用户ID符合
                        {
                            //闭锁
                            elec_lock_close();
                            box_lock_status_flag = FALSE;                            
                            //打开光照度检测功能
                            box_light_sensor_open();
                            //擦除nfc中的用户名和时间戳
                            uint32_t temp = 0;
                            hal_rfid_write_info(ADDRESS_OPERATION_USER_ID_INFO, 4, (uint8_t*)&temp);
                            hal_rfid_write_info(ADDRESS_OPERATION_TIMER_INFO, 4, (uint8_t*)&temp);
                            //上报感知箱闭锁信息
                            box_frame.frame_type = BOX_SENSOR_FRAME;
                            box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOCK;
                            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
							box_send();
                            
                            //保存关锁信息到nfc
//                            user_log.id = user_id_status;
//                            user_log.timer = operation_timer;
//                            user_log.log_type = 0x02;//获取感知箱使用权
//                            hal_rfid_write_info(ADDRESS_USER_INFO, 9, (uint8_t*)(&user_log));                             
                        }
                        else//用户ID不符合的异常
                        {
                            ;
                        }                        
                    }
//                }
            }            
        }
    }
	hal_rfid_clear_modify_flag();
    hal_wdt_clear(16000);
}
#endif
#ifdef USE_Fingerprints
void fingerprints_int_proc(void)
{
    //osel_etimer_arm(&fingerprints_isr_timer,(BOX_NFC_WAIT_TIME_LIMIT/OSEL_TICK_PER_MS),0);
    
    osel_event_t event;
    event.sig = BOX_FRIGNER_INT_EVENT;
    osel_post(NULL, &app_task_thread_process, &event);
}


void box_fingerprints_int_handle(void *arg)
{
    //读取用户ID信息
    //uint32_t user_id_status = 0;
    //hal_rfid_read_info(ADDRESS_OPERATION_USER_ID_INFO, 4*sizeof(uint8_t), (uint8_t*)(&user_id_status));
    
    box_frame_t box_frame;
	
    if(elec_lock_State_updata == 1)
    {    
        elec_lock_State_updata = 0;

        if(elec_lock_State == lock_Open)
        {
            extern bool_t is_sending;
            is_sending = FALSE;

            box_frame.frame_type = BOX_SENSOR_FRAME;
            box_frame.box_type_frame_u.sensor_info.type = SENSOR_UNLOCK;
            box_frame.box_type_frame_u.sensor_info.len = 0x00;
            box_frame.box_type_frame_u.sensor_info.user_id = box_sensor.user_id;
            box_frame.box_type_frame_u.sensor_info.timestamp = box_sensor.timestamp;

            box_blu_send_request(&box_frame); //blutooth
			
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
            box_send();
			
            box_operation_status_flag = TRUE;
                    
            box_frame.frame_type = BOX_LOCATION_FRAME;
            box_frame.box_type_frame_u.location_info.len = 8;
            box_frame.box_type_frame_u.location_info.type = 0x01;
    
            //box_frame.box_type_frame_u.location_info.gps_info.longitude = 120.3801434053; //baidu  //120.373694; �ߵ�
            //box_frame.box_type_frame_u.location_info.gps_info.latitude = 31.4958789655;  //baidu  //31.489608; �ߵ� 
            box_frame.box_type_frame_u.location_info.gps_info.longitude = box_location.longitude;
            box_frame.box_type_frame_u.location_info.gps_info.latitude = box_location.latitude;

            //插入链表
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);//for test not close 2016-04-05
            box_send();
        }
        else
        {
                 //上报感知箱闭锁信息
//            box_frame.frame_type = BOX_SENSOR_FRAME;
//            box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOCK;
//            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
        }
    }
	hal_wdt_clear(16000);
}
#endif

#ifdef USE_EXTERN_LOCK          
void extern_lock_lock_handle(void *arg)
{
    elec_lock_close();
}

void extern_lock_frozen_handle(void *arg)
{
    elec_lock_CtrlIO_Float_EX();
}

void extern_lock_status_send(void)
{
    box_frame_t box_frame;

    if(elec_lock_State_updata == 1)
    {    
        elec_lock_State_updata = 0;

        if(elec_lock_State == lock_Open)
        {
            extern bool_t is_sending;
            is_sending = FALSE;

            box_frame.frame_type = BOX_SENSOR_FRAME;
            box_frame.box_type_frame_u.sensor_info.type = SENSOR_UNLOCK;
            box_frame.box_type_frame_u.sensor_info.len = 0x00;
            box_frame.box_type_frame_u.sensor_info.user_id = box_sensor.user_id;
            box_frame.box_type_frame_u.sensor_info.timestamp = box_sensor.timestamp;

            box_blu_send_request(&box_frame); //blutooth
    		
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
            box_send();
    		
            box_operation_status_flag = TRUE;
                    
            box_frame.frame_type = BOX_LOCATION_FRAME;
            box_frame.box_type_frame_u.location_info.len = 8;
            box_frame.box_type_frame_u.location_info.type = 0x01;

            //box_frame.box_type_frame_u.location_info.gps_info.longitude = 120.3801434053; //baidu  //120.373694; �ߵ�
            //box_frame.box_type_frame_u.location_info.gps_info.latitude = 31.4958789655;  //baidu  //31.489608; �ߵ� 
            box_frame.box_type_frame_u.location_info.gps_info.longitude = box_location.longitude;
            box_frame.box_type_frame_u.location_info.gps_info.latitude = box_location.latitude;

            //插入链表
            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);//for test not close 2016-04-05
            box_send();
        }
        else
        {
                 //上报感知箱闭锁信息
    //            box_frame.frame_type = BOX_SENSOR_FRAME;
    //            box_frame.box_type_frame_u.sensor_info.type = SENSOR_LOCK;
    //            box_send_request(&box_frame,BOX_SEND_LIST_PRIORITY_HIGH,1,FALSE);
        }
    }
    hal_wdt_clear(16000);
}
#endif  


