/**
 * @file    main.c
 * @brief   Lv.0 - NXP GUI Guider V8 UI 移植到黄山派
 * @note    将 NXP GUI Guider 设计的界面（LVGL V8）集成到 SF32LB52 开发板
 */

#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "lvgl.h"

/* GUI Guider 头文件 */
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

extern rt_err_t littlevgl2rtt_init(const char *name);
extern void lv_ex_data_pool_init(void);

/* GUI Guider 全局 UI 结构体 */
lv_ui guider_ui;

/* ===================== 主函数 ===================== */

int main(void)
{
    rt_uint32_t ms;

    /* 1. 初始化 LVGL 显示驱动 + 触摸 */
    rt_err_t ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK) return ret;

    /* 2. 初始化数据池（必须！否则 rt_sem_take 断言崩溃） */
    lv_ex_data_pool_init();

    /* 3. 初始化 GUI Guider 生成的 UI */
    setup_ui(&guider_ui);

    /* 4. 初始化事件回调（如有） */
    events_init(&guider_ui);

    /* 5. 初始化自定义代码（如有） */
    custom_init(&guider_ui);

    rt_kprintf("Lv.0: GUI Guider V8 UI loaded.\n");

    /* 6. 主循环 */
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }

    return RT_EOK;
}
