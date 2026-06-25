/**
 * @file    main.c
 * @brief   Lv.0 - 纯色背景 + 文字标签 + 按钮交互
 * @note    白色背景，黑色文字，蓝色按钮，点击计数
 */

#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "lvgl.h"

extern rt_err_t littlevgl2rtt_init(const char *name);
extern void lv_ex_data_pool_init(void);

/* ===================== 全局变量 ===================== */
static lv_obj_t *g_count_label = NULL;  /* 计数器标签 */
static int g_count = 0;                 /* 计数器值 */

/* ===================== 标签创建辅助函数 ===================== */

static lv_obj_t *create_center_label(const char *text,
                                      const lv_font_t *font,
                                      lv_color_t color)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, 0);
    if (font != NULL)
        lv_obj_set_style_text_font(label, font, 0);
    lv_obj_center(label);
    return label;
}

static lv_obj_t *create_top_label(const char *text, lv_coord_t y_off)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y_off);
    return label;
}

/* ===================== 按钮交互 ===================== */

/**
 * @brief  按钮点击回调函数
 */
static void btn_click_cb(lv_event_t *e)
{
    g_count++;
    lv_label_set_text_fmt(g_count_label, "Count: %d", g_count);
    rt_kprintf("Button clicked! Count = %d\n", g_count);
}

/**
 * @brief  创建交互按钮区域
 */
static void create_button_area(void)
{
    /* 1. 计数器标签（居中偏上，黑色文字适配白背景） */
    g_count_label = lv_label_create(lv_scr_act());
    lv_label_set_text(g_count_label, "Count: 0");
    lv_obj_set_style_text_color(g_count_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_count_label, &lv_font_montserrat_24, 0);
    lv_obj_align(g_count_label, LV_ALIGN_CENTER, 0, -40);

    /* 2. 按钮（居中偏下） */
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 160, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x00, 0x7A, 0xFF), 0);
    lv_obj_set_style_radius(btn, 25, 0);

    /* 3. 按钮文字 */
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me");
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_center(btn_label);

    /* 4. 注册点击事件 */
    lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, NULL);
}

/* ===================== 主函数 ===================== */

int main(void)
{
    rt_uint32_t ms;

    /* 1. 初始化 LVGL */
    rt_err_t ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK) return ret;

    lv_ex_data_pool_init();

    /* 2. 设置白色背景 */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* 3. 顶部标签 */
    create_top_label("SF32LB52 - HuangShanPi", 30);
    create_top_label("Lv.0 - Button Interaction", 55);

    /* 4. 按钮区域 */
    create_button_area();

    /* 5. 底部版本 */
    lv_obj_t *ver = lv_label_create(lv_scr_act());
    lv_label_set_text(ver, "SiFli-SDK v2.4 | LVGL V8");
    lv_obj_set_style_text_color(ver, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_12, 0);
    lv_obj_align(ver, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_img_cache_invalidate_src(NULL);

    rt_kprintf("Lv.0: Button interaction ready\n");

    /* 6. 主循环 */
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }

    return RT_EOK;
}
