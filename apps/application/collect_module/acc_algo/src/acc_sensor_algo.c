/**
 * @brief       : 主要是加速度传感器的算法处理 
 *
 * @file        : acc_sensor_algo.c 
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
//#include <osel_arch.h>
//#include <hal_sensor.h>
//#include <hal_timer.h>
//#include <hal_board.h>
#include <math.h>
#include <acc_sensor_queue.h>
#include <acc_sensor_algo.h>

#define ACT_MEDIAN_FILTER_BUFFER_MAX_LEN    3
#define ACT_COMBINE_ACC_BUFFER_MAX_LEN      10
#define ACT_COUNT_BUFFER_MAX_LEN            4

//测试用宏
#define ACC_SAMPLE_TIMER                    80 //MS
#define TEST_ACC_ALG_DEBUG                  0
#define PLAN_A_ALL_DISCARD                  0
#define PLAN_B_ONLY_ONE_DISCARD             1

#define TIME_DISA                           5000
#define KEEP_STATIC_MAX_TIME                40  //450是一个小时,算法8秒一个结果
#define ACT_TO_STATIC_MAX_TIME              3  //15是两分钟
#define ACC_ORI_OVERRUN                     1000

//定义拆卸状态
#define DISASSEMBLY_FIRST                   0
#define DISASSEMBLY_SECOND                  1

//移动静止取中值buffer
#if TEST_ACC_ALG_DEBUG
static src_data_buffer_t act_buffer_1[ACT_MEDIAN_FILTER_BUFFER_MAX_LEN] = {{1,2,3},{1,2,3},{1,2,3}};
#else
static src_data_buffer_t act_buffer_1[ACT_MEDIAN_FILTER_BUFFER_MAX_LEN] = {0};
#endif
//移动静止取合加速度buffer
static combine_acc_buffer_t act_buffer_2[ACT_COMBINE_ACC_BUFFER_MAX_LEN] = {0};
//移动静止计数buffer
static count_buffer_t act_count_buffer[ACT_COUNT_BUFFER_MAX_LEN] = {0};

//定义移动静止状态
static uint8_t act_history_status = 0;

//定义移动静止数据阈值
static uint16_t act_data_out_count_thresh = 0;

static uint8_t act_buf1_len= 0;
static uint8_t act_buf2_len = 0;
//2014-05-13:南通夜间报移动优化，暂时屏蔽
//static uint16_t ori_acc = 0;
//static uint32_t t_acc = 0;//合加速度累加值
static uint16_t comb_acc_value = 0;
static uint16_t acc_count = 0;//超限中计数器
static uint16_t acc_count_disa = 0;//超限防拆计数器

static acc_alg_activity_overrun_detect_param_t activity_overrun_param_local;
static acc_alg_config_t config_alg_local;

acc_circular_queue_t buffer_queue;
static uint16_t act_to_static_t = 0;
static uint16_t keep_static_t = 0;
static uint16_t act_to_static_flag = 0;
static uint8_t in_move_flag = 0;//进入移动标志位，当进入移动置成1；当恢复静止后置成0，

// 配置算法回调函数
void acc_alg_config(const acc_alg_config_t *config)
{
    DBG_ASSERT(NULL != config __DBG_LINE);
    config_alg_local.data_rate = config->data_rate;
    config_alg_local.acc_alg_cb = config->acc_alg_cb; 
}

// 初始化算法模块
void acc_alg_init(void)
{
    act_history_status = ACT_STATIC_STATUS;

    activity_overrun_param_local.combine_acc_buffer_len = 10;
    activity_overrun_param_local.data_out_time_thresh = 8000;
    activity_overrun_param_local.median_filter_buffer_len = 3;
    //优化低功耗，采集频率改成12.5hz
    //activity_overrun_param_local.sample_detect_time = 10;
    activity_overrun_param_local.sample_detect_time = ACC_SAMPLE_TIMER;
    activity_overrun_param_local.variance_thresh_1 = 8;
    activity_overrun_param_local.variance_thresh_2 = 85;
    activity_overrun_param_local.variance_thresh_3 = 500;
    activity_overrun_param_local.variance_thresh_4 = 10000;
    //优化低功耗，采集频率改成12.5hz，超限上限数由12改成20
    activity_overrun_param_local.overrun_percent = 20;
    //优化低功耗，采集频率改成12.5hz，超限上限数由30改成5
    activity_overrun_param_local.overrun_lim = 10;//5
    activity_overrun_param_local.disa_percent = 25;//25
    //优化低功耗，采集频率改成12.5hz，超限上限数由3改成1
    activity_overrun_param_local.disa_lim = 1;// 3
    act_data_out_count_thresh = 100;            

    //创建队列
    acc_queue_create(&buffer_queue);
}

//中值处理3个数中取中值
int16_t acc_sensor_algo_median_proc(int16_t a,int16_t b,int16_t c)
{
    return a > b ? (b > c ? b : (a > c ? c : a)) : (a > c ? a : (b > c ? c : b));
}


//合加速度计算
void acc_sensor_algo_combine_proc(void)
{
    int32_t median_x = 0;
    int32_t median_y = 0;
    int32_t median_z = 0;
        
    median_x = acc_sensor_algo_median_proc(act_buffer_1[0].src_x,act_buffer_1[1].src_x,act_buffer_1[2].src_x);
    median_y = acc_sensor_algo_median_proc(act_buffer_1[0].src_y,act_buffer_1[1].src_y,act_buffer_1[2].src_y);
    median_z = acc_sensor_algo_median_proc(act_buffer_1[0].src_z,act_buffer_1[1].src_z,act_buffer_1[2].src_z);

    comb_acc_value = (int16_t)sqrt((median_x * median_x) + (median_y * median_y) +  (median_z * median_z));

    return ;    
}

//计算方差函数
variance_t acc_sensor_algo_variance_calc_proc(const combine_acc_buffer_t *pacc_buff)
{
    uint8_t i = 0;
    fp64_t v_combine_acc_1 = 0;
    fp64_t v_combine_acc_2 = 0;
    fp64_t v_x1 = 0;
    fp64_t v_x2 = 0;
    variance_t acc_v = {0};  

    uint16_t buffer_2_len = 0;
#if TEST_ACC_ALG_DEBUG
    buffer_2_len = 10;
#else
    buffer_2_len = activity_overrun_param_local.combine_acc_buffer_len;
#endif    

    DBG_ASSERT(NULL != pacc_buff __DBG_LINE);

    for (i = 0;i < buffer_2_len;i++,pacc_buff++)
    {
        fp64_t  v_combine_temp = 0;
        fp64_t  v_x_temp = 0;
        v_combine_temp = pacc_buff->combine_acc;
        v_x_temp = pacc_buff->x;
        v_combine_acc_1 += (v_combine_temp * v_combine_temp);
        v_combine_acc_2 += v_combine_temp;
        v_x1 += v_x_temp * v_x_temp;
        v_x2 += v_x_temp;        
    }

    acc_v.v_combine_acc = (fp32_t)(10*v_combine_acc_1 - v_combine_acc_2 * v_combine_acc_2)/90;
    acc_v.v_x = (fp32_t)(10*v_x1 - v_x2 * v_x2)/90;
    return acc_v;    
}

 //方差阈值判断处理，得出车辆状态
 //熄火、点火静止、正常行驶
bool_t acc_sensor_algo_variance_judge_proc(const variance_t *pacc_v)
{
    fp32_t v_combine_acc = 0;
    fp32_t v_x = 0;
    uint16_t sum_v_1 = 0;
    uint16_t sum_v_2 = 0;
    acc_alg_event_t act_alg_cfm;

    DBG_ASSERT(NULL != pacc_v __DBG_LINE);

    v_combine_acc = pacc_v->v_combine_acc;
    v_x = pacc_v->v_x;

    if (v_x < activity_overrun_param_local.variance_thresh_1)
    {
        act_count_buffer[0].count1 += 1;//C(1,1)=C(1,1)+1
    }
    else
    {
        act_count_buffer[1].count1 += 1;//C(1,2)=C(1,2)+1
    }

    if (v_combine_acc < activity_overrun_param_local.variance_thresh_2)
    {
        act_count_buffer[0].count2 += 1;//C(2,1)=C(2,1)+1
    }
    else if ((v_combine_acc >= activity_overrun_param_local.variance_thresh_2)
                &&(v_combine_acc < activity_overrun_param_local.variance_thresh_3))
        {
            act_count_buffer[1].count2 += 1;//C(2,2)=C(2,2)+1
        }
    else if ((v_combine_acc >= activity_overrun_param_local.variance_thresh_3)
                &&(v_combine_acc < activity_overrun_param_local.variance_thresh_4))
        {
            act_count_buffer[2].count2 += 1;//C(2,3)=C(2,3)+1
        }
    else if (v_combine_acc >= activity_overrun_param_local.variance_thresh_4)
        {
            act_count_buffer[3].count2 += 1;//C(2,4)=C(2,4)+1
        }    

    sum_v_1 = act_count_buffer[0].count2\
                    + act_count_buffer[1].count2\
                    + act_count_buffer[2].count2\
                    + act_count_buffer[3].count2;
    
    sum_v_2 = act_count_buffer[1].count2\
                + act_count_buffer[2].count2\
                + act_count_buffer[3].count2;
    
    static uint32_t static_to_act_count = 0;
    if (sum_v_1 >= act_data_out_count_thresh)
    {
        if (sum_v_2 > (uint16_t)(act_data_out_count_thresh * 0.2))//移动状态
        {            
            //如果之前状态是静止并且还是等红绿灯状态下的相对静止(实际我们认为车还是在运动)，
            //现在突然变移动 ,是不会上报静到动帧            
            if (act_to_static_flag == 0)//如果现在不处于恢复静止状态下
            {
                static_to_act_count++;
            }

            if ((ACT_STATIC_STATUS == act_history_status)
                &&(act_to_static_flag == 0)
                &&(NULL != config_alg_local.acc_alg_cb)
                &&(static_to_act_count > 3))//3
            {//如果从静止到运动则报状态变化消息
                act_alg_cfm.type = ACC_ALG_ACTIVITY_EVENT_TYPE;
                act_alg_cfm.activity_args.activity_status_type = ACC_ALG_STATIC_TO_ACT_STATUS;
                act_alg_cfm.activity_args.feature_dimen = 0x04;                    
                act_alg_cfm.activity_args.static_to_act_feature.c1 = act_count_buffer[0].count2;
                act_alg_cfm.activity_args.static_to_act_feature.c2 = act_count_buffer[1].count2;
                act_alg_cfm.activity_args.static_to_act_feature.c3 = act_count_buffer[2].count2;
                act_alg_cfm.activity_args.static_to_act_feature.c4 = act_count_buffer[3].count2;
                config_alg_local.acc_alg_cb(&act_alg_cfm);  

                act_history_status = ACT_NORMAL_DRIVE_STATUS;        
                in_move_flag = 1;
            }

            keep_static_t = 0;
            act_to_static_t = 0;
            
            if ((in_move_flag == 1)//只要不恢复静止，都会发出动保持
                &&(NULL != config_alg_local.acc_alg_cb))                 
            {
                act_alg_cfm.type = ACC_ALG_ACTIVITY_EVENT_TYPE;
                act_alg_cfm.activity_args.activity_status_type = ACC_ALG_ACT_RETAIN_STATUS;
                act_alg_cfm.activity_args.feature_dimen = 0x04;
                act_alg_cfm.activity_args.act_retain_feature.c1 = act_count_buffer[0].count2;
                act_alg_cfm.activity_args.act_retain_feature.c2 = act_count_buffer[1].count2;
                act_alg_cfm.activity_args.act_retain_feature.c3 = act_count_buffer[2].count2;
                act_alg_cfm.activity_args.act_retain_feature.c4 = act_count_buffer[3].count2;
                if (NULL != config_alg_local.acc_alg_cb)
                {
                    config_alg_local.acc_alg_cb(&act_alg_cfm);
                }                 
            }                         
            osel_memset(act_count_buffer,0x00,(sizeof(count_buffer_t)*ACT_COUNT_BUFFER_MAX_LEN));            
        }
        else//静止状态
        {   
            static_to_act_count = 0;
            if (ACT_NORMAL_DRIVE_STATUS == act_history_status)
            {//如果从运动到熄火则报状态变化消息                
                act_to_static_flag = 1;
            }
            
            if (act_to_static_flag)
            {
                act_to_static_t ++;
                if ((act_to_static_t >= ACT_TO_STATIC_MAX_TIME)// 2分钟=15
                    &&(NULL != config_alg_local.acc_alg_cb))
                {
                    act_to_static_t = 0;
                    act_to_static_flag = 0;
                    act_alg_cfm.type = ACC_ALG_ACTIVITY_EVENT_TYPE;
                    act_alg_cfm.activity_args.activity_status_type = ACC_ALG_ACT_TO_STATIC_STATUS;
                    act_alg_cfm.activity_args.feature_dimen = 0x04;
                    //动到静
                    act_alg_cfm.activity_args.act_to_static_feature.feature_flag = act_count_buffer[0].count2;
                    act_alg_cfm.activity_args.act_to_static_feature.feature_1 = act_count_buffer[1].count2;
                    act_alg_cfm.activity_args.act_to_static_feature.feature_2 = act_count_buffer[2].count2;
                    act_alg_cfm.activity_args.act_to_static_feature.feature_3 = act_count_buffer[3].count2;
                    config_alg_local.acc_alg_cb(&act_alg_cfm);
                    in_move_flag = 0;
                    
                }
            }
#if GPS_DEBUG_INFO == 1
	extern uint8_t acc_status_flag;
    acc_status_flag = 200;
#endif             
            keep_static_t++;
            act_history_status = ACT_STATIC_STATUS;         

            if ((keep_static_t >= KEEP_STATIC_MAX_TIME)
                &&(NULL != config_alg_local.acc_alg_cb))
            {// 5.3分钟调用一次静保持，主要为防止GPS未关闭
                keep_static_t = 0;
                act_alg_cfm.type = ACC_ALG_ACTIVITY_EVENT_TYPE;
                act_alg_cfm.activity_args.activity_status_type = ACC_ALG_STATIC_RETAIN_STATUS;
                act_alg_cfm.activity_args.feature_dimen = 0x04;
                //静态保持
                act_alg_cfm.activity_args.static_retain_feature.feature_flag = act_count_buffer[0].count2;
                act_alg_cfm.activity_args.static_retain_feature.feature_1 = act_count_buffer[1].count2;
                act_alg_cfm.activity_args.static_retain_feature.feature_2 = act_count_buffer[2].count2;
                act_alg_cfm.activity_args.static_retain_feature.feature_3 = act_count_buffer[3].count2;
                config_alg_local.acc_alg_cb(&act_alg_cfm);  
            }           
            osel_memset(act_count_buffer,0x00,(sizeof(count_buffer_t)*ACT_COUNT_BUFFER_MAX_LEN));           
        }
    }

    return TRUE;    
}

//静动处理函数
void acc_alg_action_proc(acc_data_type buffer_in)
{
    uint16_t buffer_1_len = 0;
    uint16_t buffer_2_len = 0;
#if PLAN_B_ONLY_ONE_DISCARD
    uint16_t i = 0;
#endif
    variance_t act_acc_variance = {0};
        
    buffer_1_len = activity_overrun_param_local.median_filter_buffer_len;
    buffer_2_len = activity_overrun_param_local.combine_acc_buffer_len;

    //静动处理
    if (act_buf1_len >= buffer_1_len)
    {
        acc_sensor_algo_combine_proc();//计算buffer_1 合加速度

        if (act_buf2_len >= buffer_2_len)
        {                    
            act_acc_variance = acc_sensor_algo_variance_calc_proc(act_buffer_2);//计算buffer_2 方差
            //判断方差阈值,返回车辆状态
            acc_sensor_algo_variance_judge_proc(&act_acc_variance);
            
         #if PLAN_A_ALL_DISCARD
            osel_memset(act_buffer_2, 0x00, sizeof(combine_acc_buffer_t)*ACT_COMBINE_ACC_BUFFER_MAX_LEN);
            act_buf2_len = 0;
         #endif

         #if PLAN_B_ONLY_ONE_DISCARD
         //删除最后最旧的一组数
         
             for (i = 0;i < (act_buf2_len - 1);i++)
            {
                act_buffer_2[i] = act_buffer_2[i + 1];
            }
            
             osel_memset((act_buffer_2 + act_buf2_len - 1),0x00,sizeof(combine_acc_buffer_t));
             //osel_memset((act_buffer_2 + act_buf2_len - 2),0x00,sizeof(combine_acc_buffer_t) * 2);
             act_buf2_len--;
             //act_buf2_len = act_buf2_len - 2;
         #endif       
        }
        
        act_buffer_2[act_buf2_len].combine_acc = comb_acc_value;
        act_buffer_2[act_buf2_len].x = act_buffer_1[0].src_x;
        act_buffer_2[act_buf2_len].y = act_buffer_1[0].src_y;
        act_buffer_2[act_buf2_len].z = act_buffer_1[0].src_z;
        act_buf2_len ++;

        act_buffer_1[0] = act_buffer_1[1];
        act_buffer_1[1] = act_buffer_1[2];
        act_buf1_len--;              
    }
    
    act_buffer_1[act_buf1_len].src_x = buffer_in.src_x;
    act_buffer_1[act_buf1_len].src_y = buffer_in.src_y;
    act_buffer_1[act_buf1_len].src_z = buffer_in.src_z;
    act_buf1_len ++;   

}

//超限处理函数
void acc_alg_overrun_proc(void)
{
    int16_t acc_temp = 0;
    fp32_t p = 0;
    uint16_t ovr_lim = 0;
    fp32_t p_disa = 0;
    uint16_t ovr_lim_disa = 0;    
    acc_alg_event_t act_alg_cfm;
    
    p = (fp32_t)activity_overrun_param_local.overrun_percent/100;
    ovr_lim = activity_overrun_param_local.overrun_lim;
    p_disa = (fp32_t)activity_overrun_param_local.disa_percent/100;
    ovr_lim_disa = activity_overrun_param_local.disa_lim;    
    //2014-05-13:南通夜间报移动优化，暂时把ori_acc直接赋值成1000
    //acc_temp = comb_acc_value - ori_acc;
    acc_temp = comb_acc_value - ACC_ORI_OVERRUN;
    //协助防拆判断处理
   // if ((ori_acc != 0) && (ABS(acc_temp) >= (uint16_t)(ori_acc * p_disa)))
    if (ABS(acc_temp) >= (uint16_t)(ACC_ORI_OVERRUN * p_disa))
    {
        acc_count_disa ++;
        if (acc_count_disa >= ovr_lim_disa)
        {
            acc_count_disa = 0;  
        }  
    }
    else
    {
        acc_count_disa = 0;
    }    
    //超限判断处理
    //if ((ori_acc != 0) && (ABS(acc_temp) >= (uint16_t)(ori_acc * p)))
    if (ABS(acc_temp) >= (uint16_t)(ACC_ORI_OVERRUN * p))
    {
        acc_count ++;
        if ((acc_count >= ovr_lim)
            &&(NULL != config_alg_local.acc_alg_cb))
        {
            act_alg_cfm.type = ACC_ALG_OVERRUN_EVENT_TYPE;
            act_alg_cfm.overrun_args.overrun_flg = 0x01;
            act_alg_cfm.overrun_args.acc_x = act_buffer_2[0].x;
            act_alg_cfm.overrun_args.acc_y = act_buffer_2[0].y;
            act_alg_cfm.overrun_args.acc_z = act_buffer_2[0].z;
            config_alg_local.acc_alg_cb(&act_alg_cfm); 
            acc_count = 0;  
        }
        else
        {
            ;
        }  
    }
    else
    {
        acc_count = 0;
    }    
    return;
}
#include <math.h>
#define BUFF_SIZE_MAX   10
#define THRESHOLD       9//2
#define DELAY_NUM       12

acc_new_alg_type_t data_bak = {0};//背景值
acc_new_alg_type_t data_buf[BUFF_SIZE_MAX] = {0};
//acc_new_alg_type_t tmp_filter = {0};//低通滤波值
fp32_t tmp_filter_c[4] = {0};
uint8_t delay_num = 0;
uint8_t data_coming_indx = 0;
bool_t tmp_result = FALSE;
bool_t tmp_change = FALSE;
/**
* 求和
*/
acc_new_alg_type_t acc_alg_sum(const acc_new_alg_type_t *in_buff)
{
    acc_new_alg_type_t sum_out = {0};
    int16_t sum_x = 0;
    int16_t sum_y = 0;
    int16_t sum_z = 0;
    int16_t sum_temp = 0;
    
    for (uint8_t i = 0;i < BUFF_SIZE_MAX;i++,in_buff++)
    {
        sum_x = sum_x + in_buff->src_x;
        sum_y = sum_y + in_buff->src_y;
        sum_z = sum_z + in_buff->src_z;
        sum_temp = sum_temp + in_buff->src_temp;
    }
    
    sum_out.src_x = sum_x;
    sum_out.src_y = sum_y;
    sum_out.src_z = sum_z;
    sum_out.src_temp = sum_temp;
    return sum_out;
}
/**
* 求最大值
*/
fp32_t c_max_all(fp32_t *p,uint8_t n)
{
    fp32_t max_t = 0;
    max_t = *p;
    p++;
    for (uint8_t i = 0;i < (n-1);i++,p++)
    {
        if (max_t <= *p)
        {
            max_t = *p;
        }
    }
    return max_t;
}
int16_t c_max(int16_t *p)
{
    int16_t max_t = 0;
    max_t = *p;
    p++;
    for (uint8_t i = 0;i < (BUFF_SIZE_MAX-1);i++,p++)
    {
        if (max_t <= *p)
        {
            max_t = *p;
        }
    }
    return max_t;
}

acc_new_alg_type_t acc_alg_max(const acc_new_alg_type_t *in_buff)
{
    acc_new_alg_type_t max_out = {0};
    int16_t max_x_c[BUFF_SIZE_MAX] = {0};
    int16_t max_y_c[BUFF_SIZE_MAX] = {0};
    int16_t max_z_c[BUFF_SIZE_MAX] = {0};
    int16_t max_temp_c[BUFF_SIZE_MAX] = {0};
    
    for (uint8_t i = 0;i < BUFF_SIZE_MAX;i++,in_buff++)
    {
        max_x_c[i] = in_buff->src_x;
        max_y_c[i] = in_buff->src_y;
        max_z_c[i] = in_buff->src_z;
        max_temp_c[i] = in_buff->src_temp;
    }
    //x的最大值
    max_out.src_x = c_max(max_x_c);       
    //y的最大值
    max_out.src_y = c_max(max_y_c);    
    //z的最大值
    max_out.src_z = c_max(max_z_c);
    //temp的最大值
    max_out.src_temp = c_max(max_temp_c);
    return max_out;    
}
/**
* 求最小值
*/
int16_t c_min(int16_t *p)
{
    int16_t min_t = 0;
    min_t = *p;
    p++;
    for (uint8_t i = 0;i < (BUFF_SIZE_MAX-1);i++,p++)
    {
        if (min_t >= *p)
        {
            min_t = *p;
        }
    }
    return min_t;    
}

acc_new_alg_type_t acc_alg_min(const acc_new_alg_type_t *in_buff)
{
    acc_new_alg_type_t min_out = {0};
    int16_t min_x_c[BUFF_SIZE_MAX] = {0};
    int16_t min_y_c[BUFF_SIZE_MAX] = {0};
    int16_t min_z_c[BUFF_SIZE_MAX] = {0};
    int16_t min_temp_c[BUFF_SIZE_MAX] = {0};
    
    for (uint8_t i = 0;i < BUFF_SIZE_MAX;i++,in_buff++)
    {
        min_x_c[i] = in_buff->src_x;
        min_y_c[i] = in_buff->src_y;
        min_z_c[i] = in_buff->src_z;
        min_temp_c[i] = in_buff->src_temp;
    }
    //x的最大值
    min_out.src_x = c_min(min_x_c);       
    //y的最大值
    min_out.src_y = c_min(min_y_c);    
    //z的最大值
    min_out.src_z = c_min(min_z_c);
    //temp的最大值
    min_out.src_temp = c_max(min_temp_c);
    return min_out;   
}

void acc_alg_knock_detect(acc_data_type buf_in)
{
    acc_new_alg_type_t tmp_data;
    acc_new_alg_type_t tmp_data_2;
    int32_t x;
    int32_t y;
    int32_t z;
    
    acc_new_alg_type_t sum_out;
    acc_new_alg_type_t max_out;
    acc_new_alg_type_t min_out;
    acc_alg_event_t act_alg_cfm;    
    
    x = buf_in.src_x;
    y = buf_in.src_y;
    z = buf_in.src_z;
    tmp_data.src_x = x;
    tmp_data.src_y = y;
    tmp_data.src_z = z;
    tmp_data.src_temp = (int16_t)sqrt(x*x + y*y + z*z);
    tmp_data_2.src_x = (int16_t)fabs(tmp_data.src_x - data_bak.src_x);
    tmp_data_2.src_y = (int16_t)fabs(tmp_data.src_y - data_bak.src_y);
    tmp_data_2.src_z = (int16_t)fabs(tmp_data.src_z - data_bak.src_z);
    tmp_data_2.src_temp = (int16_t)fabs(tmp_data.src_temp - data_bak.src_temp);
    //更新背景值
    data_bak.src_x = tmp_data.src_x;
    data_bak.src_y = tmp_data.src_y;
    data_bak.src_z = tmp_data.src_z;
    data_bak.src_temp = tmp_data.src_temp;
    
    if (data_coming_indx++ >= BUFF_SIZE_MAX)//如果来10组数据就进行算法处理
    {
        //先循环覆盖之前数据
         for (uint8_t i = 0;i < (BUFF_SIZE_MAX - 1);i++)
        {
            data_buf[i] = data_buf[i + 1];
        }
        //永远保存到队列的最后一个
        data_buf[BUFF_SIZE_MAX - 1].src_x = tmp_data_2.src_x;
        data_buf[BUFF_SIZE_MAX - 1].src_y = tmp_data_2.src_y;
        data_buf[BUFF_SIZE_MAX - 1].src_z = tmp_data_2.src_z;
        data_buf[BUFF_SIZE_MAX - 1].src_temp = tmp_data_2.src_temp; 
        
        data_coming_indx--;        
       //进行低通滤波        
        // 1.求和
        sum_out = acc_alg_sum(data_buf);
        //2.求最大值
        max_out = acc_alg_max(data_buf);
        //3.求最小值
        min_out = acc_alg_min(data_buf);
        
        tmp_filter_c[0] = (sum_out.src_x - max_out.src_x - min_out.src_x)/(BUFF_SIZE_MAX-2);
        tmp_filter_c[1] = (sum_out.src_y - max_out.src_y - min_out.src_y)/(BUFF_SIZE_MAX-2);
        tmp_filter_c[2] = (sum_out.src_z - max_out.src_z - min_out.src_z)/(BUFF_SIZE_MAX-2);
        tmp_filter_c[3] = (sum_out.src_temp - max_out.src_temp - min_out.src_temp)/(BUFF_SIZE_MAX-2);
        //4.求这四个值的最大值
        fp32_t max_tmp_filter = 0;
        max_tmp_filter = c_max_all(tmp_filter_c,4);
        
        if (max_tmp_filter >= THRESHOLD)//达到阈值
        {
            if (tmp_result == FALSE)
            {
                tmp_change = TRUE;
                tmp_result = TRUE;
                act_alg_cfm.type = ACC_ALG_OVERRUN_EVENT_TYPE;
                config_alg_local.acc_alg_cb(&act_alg_cfm);                 
            }
            else
            {
                tmp_result = TRUE;
            }
        }
        else//未达到阈值
        {
            if (tmp_result == TRUE)
            {
                delay_num++;
                if (delay_num >= DELAY_NUM)
                {
                    tmp_result = FALSE;
                    delay_num = 0;
                }
                else
                {
                    tmp_result = TRUE;
                }
            }
            else
            {
                tmp_result = FALSE;
            }
        }
    }
    else//对刚进来的10个数据进行保存
    {
        //对来的数据进行缓存到tmp_filter
        data_buf[data_coming_indx - 1].src_x = tmp_data_2.src_x;
        data_buf[data_coming_indx - 1].src_y = tmp_data_2.src_y;
        data_buf[data_coming_indx - 1].src_z = tmp_data_2.src_z;
        data_buf[data_coming_indx - 1].src_temp = tmp_data_2.src_temp;           
    }       
}

// 传入加速度数据
void acc_alg_new_data(const acc_alg_data_t *acc_data)
{
    acc_data_type buf_in;
    DBG_ASSERT(NULL != acc_data __DBG_LINE);

    buf_in.src_x = acc_data->x;
    buf_in.src_y = acc_data->y;
    buf_in.src_z = acc_data->z;   
    //数据入队列
    acc_queue_send(&buffer_queue, buf_in);
    //静动处理
    acc_alg_action_proc(buf_in);
    //超限处理
//    acc_alg_overrun_proc();
    //敲击检测
    acc_alg_knock_detect(buf_in);
    //数据出队列
    if (buffer_queue.count >= ACC_QUEUE_MAXLEN)
    {
        buffer_queue.count--;
    }        

    return;
}