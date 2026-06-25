/**
 * @file    main.c
 * @brief   Lv.0 - 纯色背景
 * @note    在 AMOLED (390x450) 上显示纯白色背景
 */

#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "lvgl.h"

/* 前向声明 */
extern rt_err_t littlevgl2rtt_init(const char *name);
extern void lv_ex_data_pool_init(void);

/**
 * @brief  主函数入口
 * @retval 0 成功
 */
int main(void)
{
    rt_uint32_t ms;

    /* 1. 初始化 LVGL 显示驱动和触摸输入 */
    rt_err_t ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        rt_kprintf("LVGL init failed!\n");
        return ret;
    }

    /* 2. 初始化 LVGL 数据池（信号量 + 链表） */
    lv_ex_data_pool_init();

    /* 3. 获取当前屏幕并设置纯白背景 */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    rt_kprintf("Lv.0: White background ready\n");

    /* 4. 主循环 - 必须持续调用 lv_task_handler() */
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }

    return RT_EOK;
}
