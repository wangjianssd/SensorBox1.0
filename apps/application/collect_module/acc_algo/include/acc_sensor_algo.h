#ifndef _ACC_SENSOR_ALGO_H
#define _ACC_SENSOR_ALGO_H
#include <acc_sensor_queue.h>

//存放原始采集来的X Y Z，
typedef struct
{
    int16_t  src_x;
    int16_t  src_y;
    int16_t  src_z;
}src_data_buffer_t;

//存放和加速度值和x y z轴加速度分量
typedef struct
{
    int16_t combine_acc;
    int16_t  x;
    int16_t  y;
    int16_t  z;
}combine_acc_buffer_t;

//存放合加速度、x、y、z的方差值
typedef struct
{
    fp32_t v_combine_acc;
    fp32_t  v_x;
    fp32_t  v_y;
    fp32_t  v_z;
}variance_t;

//存放计数值
typedef struct
{
    uint16_t  count1;
    uint16_t  count2;
}count_buffer_t;  


//定义运动静止状态
typedef enum
{
    ACT_INVALID_STATUS,
    ACT_NORMAL_DRIVE_STATUS,//正常行驶状态
    ACT_STATIC_STATUS,//静止状态 
    ACT_OVERRUN_STATUS,//超限状态    
}act_status_e;

typedef enum
{
    ACC_ALG_ACTIVITY_OVERRUN_DETECT_PARAM = 0, //超限检测算法参数
    ACC_ALG_TOPPLE_DETECT_PARAM,  //倾倒检测算法参数
    ACC_ALG_TURN_DETECT_PARAM,    //转向检测算法参数
    ACC_ALG_FALL_DETECT_PARAM,  //丢落检测算法参数

    ACC_ALG_OVERRUN_EVENT_TYPE,  //超限事件
    ACC_ALG_ACTIVITY_EVENT_TYPE, //静动事件
    ACC_ALG_FALL_EVENT_TYPE,     //跌落事件
    ACC_ALG_TOPPLE_EVENT_TYPE,   //倾倒事件
    ACC_ALG_TURN_EVENT_TYPE,     //转向事件
    ACC_ALG_DISA_EVENT_TYPE,            //拆卸事件 9

    ACC_ALG_METER_SINGLE_TAP_EVENT_TYPE, // 单击事件
    ACC_ALG_METER_DOUBLE_TAP_EVENT_TYPE, // 双击事件
    ACC_ALG_METER_ACTIVITY_EVENT_TYPE,   // 活动事件
    ACC_ALG_METER_FREE_FALL_EVENT_TYPE,  // 自由落体事件13
}e_acc_alg_t;

typedef enum
{
    ACC_ALG_STATIC_TO_ACT_STATUS = 0x00,//静到动状态
    ACC_ALG_STATIC_RETAIN_STATUS = 0x11,//静态保持
    ACC_ALG_ACT_RETAIN_STATUS = 0xee,//动态保持
    ACC_ALG_ACT_TO_STATIC_STATUS = 0xff,//动到静状态
}acc_act_static_e;

//静动、超限检测配置参数
typedef struct
{
    uint16_t sample_detect_time;//采样间隔时间，默认为10ms
    uint16_t median_filter_buffer_len;//中值滤波缓存长度，默认为3
    uint16_t combine_acc_buffer_len;//合加速度缓存长度，默认为10
    uint16_t variance_thresh_1;//方差阈值1，默认为8
    uint16_t variance_thresh_2;//方差阈值2，默认为20
    uint16_t variance_thresh_3;//方差阈值3，默认为500
    uint16_t variance_thresh_4;//方差阈值4，默认为10000
    uint16_t data_out_time_thresh;//定时输出结果的时间阈值，默认10000ms
    uint16_t overrun_percent;//超限百分比 默认12
    uint16_t overrun_lim;//超限计数上限  默认30
    uint16_t disa_percent;//超限防拆百分比 默认25
    uint16_t disa_lim;//超限防拆计数上限  默认3    
}acc_alg_activity_overrun_detect_param_t;

 //超限事件的参数类型
typedef struct
{
    uint8_t overrun_flg;//超限状态标志位
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
}acc_alg_overrun_event_args_t;

//静态保持特征
typedef struct
{
    uint16_t feature_flag;
    uint16_t feature_1;
    uint16_t feature_2;
    uint16_t feature_3;
}acc_alg_static_retain_feature_t;

//动态保持特征
typedef struct
{
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
    uint16_t c4;
}acc_alg_act_retain_feature_t;

//静到动特征
typedef struct
{
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
    uint16_t c4;
}acc_alg_static_to_act_feature_t;

//动到静特征
typedef struct
{
    uint16_t feature_flag;
    uint16_t feature_1;
    uint16_t feature_2;
    uint16_t feature_3;
}acc_alg_act_to_static_feature_t;

//静动事件的参数类型
typedef struct
{
    uint8_t activity_status_type;//静动状态标示
    uint8_t  feature_dimen;//特征维度  
    union
    {
        acc_alg_static_retain_feature_t static_retain_feature;
        acc_alg_act_retain_feature_t act_retain_feature;
        acc_alg_static_to_act_feature_t static_to_act_feature;
        acc_alg_act_to_static_feature_t act_to_static_feature;
    };     
}acc_alg_activity_event_args_t; 

 //跌落事件的参数类型
typedef struct
{
    uint16_t acc_x;//x轴绝对加速度
    uint16_t acc_y;//y轴绝对加速度
    uint16_t acc_z;//z轴绝对加速度
}acc_alg_fall_event_args_t;

//倾倒事件的参数类型
typedef struct
{
    uint8_t  feature_dimen;//特征维度  
    uint16_t feature_1;//当前向量与背景向量差值模值
    uint16_t feature_2; //当前向量模值   
}acc_alg_topple_event_args_t; 

 //转向事件的参数类型
typedef struct
{
    uint8_t  feature_dimen;//特征维度  
    int16_t  feature_1;//车辆加速度累加值
}acc_alg_turn_event_args_t;
 
//拆卸时间的参数类型
typedef struct
{
    uint8_t disa_flag;// 1代表有拆卸行为，0代表无
}acc_alg_disa_event_args_t;

typedef struct
{
    uint8_t type; //事件类型
    union
    {
            acc_alg_overrun_event_args_t  overrun_args;
            acc_alg_activity_event_args_t activity_args;
            acc_alg_fall_event_args_t     fall_args;
            acc_alg_topple_event_args_t   topple_args; //倾倒事件的参数
            acc_alg_turn_event_args_t     turn_args;   //转向事件的参数
            acc_alg_disa_event_args_t     disa_args;   //拆卸事件的参数
    };
}acc_alg_event_t;

typedef void (*acc_alg_event_callback_t)(const acc_alg_event_t *acc_alg_event);

typedef struct
{
    uint16_t                  data_rate; //数据速率
    acc_alg_event_callback_t acc_alg_cb;
}acc_alg_config_t;

typedef struct
{
    int16_t x; //单位ms
    int16_t y;
    int16_t z;
}acc_alg_data_t;

void acc_alg_new_data(const acc_alg_data_t *acc_data);
void acc_alg_init(void);
void acc_alg_config(const acc_alg_config_t *config);
#endif

