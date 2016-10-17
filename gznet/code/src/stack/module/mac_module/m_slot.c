/**
 * @brief       : Provide a superframe scheduling maintenance framework
 *
 * @file        : m_slot.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include "common/lib/lib.h"
#include "common/hal/hal.h"
#include "mac_module.h"

bool_t sync_en = TRUE;
static uint16_t slot_seq = 0;               //!< 用来表示当前运行的时隙的统一编号
static uint8_t time_type = 0;               //!< 用来表示当前的时间是本地时间还是全局时间
static bool_t slot_running = FALSE;
static slot_cfg_t *current_node = NULL;     //!< 用来指示当前运行到了哪一个时隙
static bool_t force_stop = FALSE;           //!< 用来强制停止时隙模块
static uint32_t root_node_begin;            //!< 用来表示根节点的起始时间

PROCESS_NAME(slot_event_process);

static void slot_node_handler(slot_cfg_t *const node);

static void slot_timeout_cb(void *arg)
{
    slot_cfg_t *slot_cfg = (slot_cfg_t *)arg;
    slot_cfg->slot_timer = NULL;
    
    osel_event_t event;
    event.sig = M_SLOT_TIMEOUT_EVENT;
    event.param = (osel_param_t)arg;
    osel_post(NULL, &slot_event_process, &event);
    
}

bool_t m_slot_get_state(void)
{
    return slot_running;
}

uint32_t m_slot_get_remain_time(void)
{
    hal_time_t now = hal_timer_now();
    hal_time_t node_time;

    if (current_node->first_child != NULL)
    {
        return 0;
    }
    else
    {
        node_time.w = current_node->slot_record_start;
        if (time_type != TRUE)      // 全局时间切换到本地时间
        {
			if(m_sync_get())
			{
				 m_sync_g2l(&node_time);
			}
        }

        if (node_time.w > now.w)
        {
            return ( 0xffffffff - node_time.w + now.w );
        }
        else
        {
            if ((node_time.w + current_node->slot_duration) <=  now.w)
            {
                return 0;
            }
            else
            {
                return (node_time.w + current_node->slot_duration - now.w);
            }
        }
    }
}

uint32_t m_slot_get_root_begin(void)
{
    return root_node_begin;
}

uint16_t m_slot_get_seq(void)
{
    return slot_seq;
}

// 注: 时隙的持续时间必须大于函数的执行时间，否则会出现无法预估的情况
static bool_t m_slot_check_slot_describ(const slot_cfg_t *const sup_desc)
{
    slot_cfg_t *next_sl_desc = NULL;

    if (NULL == sup_desc)
    {
        return FALSE;
    }

    if ((0 == sup_desc->slot_duration) && (NULL == sup_desc->first_child))   // 时隙的duration必须大于0
    {
        return FALSE;
    }

    if ((NULL != sup_desc->parent) && (0 == sup_desc->slot_repeat_cnt)) // 子时隙不能无限制重复
    {
        return FALSE;
    }
    if ((NULL == sup_desc->parent) && (0 != sup_desc->slot_repeat_cnt)) // 根节点必须无限循环
    {
        return FALSE;
    }

    if (NULL != sup_desc->first_child)
    {
        for (next_sl_desc = sup_desc->first_child;
                NULL != next_sl_desc->next_sibling;
                next_sl_desc = next_sl_desc->next_sibling)
        {
            if (FALSE == m_slot_check_slot_describ(next_sl_desc))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static slot_cfg_t *find_my_previous_brother(slot_cfg_t *const sup_desc)
{
    slot_cfg_t *first_child = sup_desc->parent->first_child;
    while(first_child->next_sibling != sup_desc)
    {
        first_child = first_child->next_sibling;
    }
    
    return first_child;
}

static void m_slot_describ_empty_remove(slot_cfg_t *const sup_desc)
{
    if (sup_desc->parent == NULL)
    {
        m_slot_describ_empty_remove(sup_desc->first_child);
    }
    else
    {
        if ((sup_desc->slot_duration == 0) || (sup_desc->slot_repeat_cnt == 0))//delete myslef , connect father and brother
        {
            if(sup_desc->parent->first_child == sup_desc)
            {
                sup_desc->parent->first_child = sup_desc->next_sibling;
            }
            else
            {
                find_my_previous_brother(sup_desc)->next_sibling = sup_desc->next_sibling;
            }
            
            if(sup_desc->next_sibling != NULL)
            {
                m_slot_describ_empty_remove(sup_desc->next_sibling);
            }

            // mfree(&sup_desc); free myself
        }
        else
        {      
            if(sup_desc->first_child != NULL)
            {
                m_slot_describ_empty_remove(sup_desc->first_child);
            }

            if(sup_desc->next_sibling != NULL)
            {
                m_slot_describ_empty_remove(sup_desc->next_sibling);
            }
        }
    }
}

static void m_slot_describ_calc(slot_cfg_t *const sup_desc)
{
    /* 判断是否有孩子 */
    if (NULL != sup_desc->first_child)
    {
        sup_desc->slot_duration = 0;    //非叶子节点时间是有模块计算得出，这里清零，防止用户错误的输入
        m_slot_describ_calc(sup_desc->first_child);
    }
    else
    {
        // 对节点时间与重复性进行验证
        //DBG_ASSERT(0 != sup_desc->slot_duration __DBG_LINE);
        //DBG_ASSERT(0 != sup_desc->slot_repeat_cnt __DBG_LINE);
    }

    /* 判断是否是根节点，是就直接退出 */
    if (NULL == sup_desc->parent)
    {
        return;
    }

    /* 这里已经是时隙的最底层了，把这个时隙的时间添加到父时隙的时间内 */
    sup_desc->parent->slot_duration += sup_desc->slot_duration * sup_desc->slot_repeat_cnt;

    /* 判断是否有兄弟 */
    if (NULL != sup_desc->next_sibling)
    {
        m_slot_describ_calc(sup_desc->next_sibling);
    }
}

void m_slot_cfg(slot_cfg_t *const sup_frame, uint8_t type)
{
    /* 对时隙表的每个时隙时间进行软件修正 */
    m_slot_describ_calc(sup_frame);

    m_slot_describ_empty_remove(sup_frame);

    /* 时隙表配置检测 */
    if (!m_slot_check_slot_describ(sup_frame))
    {
        DBG_ASSERT(FALSE __DBG_LINE);
        return;
    }

    /* 初始化追踪器 */
    current_node = sup_frame;

    if (SLOT_GLOBAL_TIME != type)
    {
        time_type = TRUE;
    }
    else
    {
        time_type = FALSE;
    }
}

static void slot_child_begin(  slot_cfg_t *const sub_node,
                               uint32_t start_time)
{
    if (sub_node->first_child != NULL)
    {
        sub_node->first_child->slot_start = start_time;
        slot_child_begin(sub_node->first_child, start_time);
    }
}

static void slot_timer_set(const slot_cfg_t *const sup_frame)
{
    hal_time_t node_time;

    node_time.w = sup_frame->slot_start;
    if (time_type != TRUE)      // 全局时间切换到本地时间
    {
		if(m_sync_get())
        	m_sync_g2l(&node_time);
    }
    
    if(sup_frame->slot_timer != NULL)
    {
        hal_timer_cancel(&(((slot_cfg_t *)sup_frame)->slot_timer));
    }
//    static hal_time_t now;
//    now = hal_timer_now();

    ((slot_cfg_t *)sup_frame)->slot_timer = hal_timer_set(node_time, slot_timeout_cb, (void *)sup_frame, NULL);
    DBG_ASSERT(sup_frame->slot_timer != NULL __DBG_LINE);
}

void m_slot_run(hal_time_t *start_time)
{
    hal_time_t now = hal_timer_now();

    if (time_type != TRUE)      // 全局时间切换到本地时间
    {
		if(m_sync_get())
        	m_sync_l2g(&now);
    }

    if (start_time->w <= now.w)
    {
        /* 当前的追踪器指向的是最顶层，所以直接加上偏移，指向下一个时隙起始 */
        start_time->w += current_node->slot_duration;
        m_slot_run(start_time);
    }
    else
    {
        force_stop = FALSE;
        current_node->slot_start = start_time->w;
        slot_timer_set(current_node);
        slot_running = TRUE;
    }
}

void m_slot_stop(void)
{
    force_stop = TRUE;
    slot_running = FALSE;
}

static void slot_node_execute(slot_cfg_t *const node)
{
    hal_int_state_t s;
    HAL_ENTER_CRITICAL(s);
    current_node = node;                                    // 1, current_node = current_node;
    HAL_EXIT_CRITICAL(s);

    node->slot_record_start = node->slot_start;
    slot_child_begin(node, node->slot_start);               // 2, 设置第一个孩子的时隙起始
    if (NULL != node->func)                                 // 3, 执行current_node的回调
    {
        node->func((void *)&node->slot_repeat_seq);
    }
    node->slot_repeat_seq++;
    node->slot_start += node->slot_duration;                // 4, 添加时间偏移，下次定时

    slot_timer_set(node);
}

static bool_t node_has_child(slot_cfg_t const *node)
{
    DBG_ASSERT(node != NULL __DBG_LINE);
    return (node->first_child != NULL);
}

/* 返回FALSE表示无孩子需要处理，否则有孩子需要处理 */
static bool_t slot_node_and_child_excute(slot_cfg_t *const node)
{
    /* 执行本时隙回调、定本时隙结束时间 */
    slot_node_execute(node);

    if (node_has_child(node))
    {
        slot_node_handler(node->first_child); // 递归
    }
    else
    {
        slot_seq++; // 叶子节点编号(统一编号)自增
        return FALSE;
    }

    return TRUE;
}

static void slot_node_handler(slot_cfg_t *const node)
{
    /* 判断是否根节点 */
    if (node->parent == NULL)
    {
        DBG_ASSERT(node->slot_repeat_cnt == 0 __DBG_LINE);      // 根节点再次判断验证
        slot_seq = 0;
        node->slot_repeat_seq = 0;

        root_node_begin = node->slot_start; // 记录根节点的起始时间赋值

        if (!slot_node_and_child_excute(node))
        {
            return;
        }
    }
    else
    {
        /* 判断重复结束 */
        if (node->slot_repeat_seq < node->slot_repeat_cnt)  // 重复没有结束
        {
            if (!slot_node_and_child_excute(node))
            {
                return;
            }
        }
        else
        {
            node->slot_repeat_seq = 0; // 复位本时隙重复计数器
            if (node->next_sibling != NULL)
            {
                node->next_sibling->slot_start = node->slot_start;  // 本时隙的结束就是兄妹时隙的开始
                if (!slot_node_and_child_excute(node->next_sibling))
                {
                    return;
                }
            }
            else
            {
                current_node = node->parent;    // 追踪器指向上级
            }
        }
    }
}

static void m_slot_event_handler(const void *const pmsg)
{
    DBG_ASSERT(pmsg != NULL __DBG_LINE);

    hal_time_t now = hal_timer_now();

    if (TRUE == force_stop)
    {
        return;
    }

    slot_cfg_t *_current = (slot_cfg_t *)(pmsg);
    slot_node_handler(_current);
}

PROCESS(slot_event_process, "slot_event_process");
PROCESS_THREAD(slot_event_process, ev, data)
{
    PROCESS_BEGIN();
    
    while(1)
    {   
        if(ev == M_SLOT_TIMEOUT_EVENT) {
            m_slot_event_handler(data);
        }

        PROCESS_YIELD();
    }

    PROCESS_END();
}

void m_sync_en(bool_t state)
{
	sync_en = state;
}

bool_t m_sync_get(void)
{
	return sync_en;
}

void m_slot_init(osel_task_t *task)
{
    osel_pthread_create(task, &slot_event_process, NULL);
}
