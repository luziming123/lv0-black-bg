/**
 * @file    main.c
 * @brief   Lv.0 - 纯色背景 + 文字标签
 * @note    在白色背景上显示黑色 "Hello HSPI" 和当前级别信息
 */

#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "lvgl.h"

extern rt_err_t littlevgl2rtt_init(const char *name);
extern void lv_ex_data_pool_init(void);

/* ===================== 标签创建辅助函数 ===================== */

/**
 * @brief  在屏幕上创建居中文字标签
 * @param  text  要显示的文字
 * @param  font  字体指针（NULL 则使用默认）
 * @param  color 文字颜色
 * @return 标签对象指针
 */
static lv_obj_t *create_center_label(const char *text,
                                      const lv_font_t *font,
                                      lv_color_t color)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, 0);

    if (font != NULL)
    {
        lv_obj_set_style_text_font(label, font, 0);
    }

    lv_obj_center(label);
    return label;
}

/**
 * @brief  创建顶部小字标签
 * @param  text  文字内容
 * @param  y_off Y 偏移量
 * @return 标签对象指针
 */
static lv_obj_t *create_top_label(const char *text, lv_coord_t y_off)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y_off);
    return label;
}

/* ===================== 主函数 ===================== */

int main(void)
{
    rt_uint32_t ms;

    /* 1. 初始化 LVGL */
    rt_err_t ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK) return ret;

    /* 2. 初始化数据池（图像资源管理） */
    lv_ex_data_pool_init();

    /* 3. 设置白色背景 */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ---- 创建文字标签 ---- */

    /* 顶部提示文字（灰色小字） */
    create_top_label("SF32LB52 - HuangShanPi", 30);
    create_top_label("Lv.0 - Black BG + Text", 55);

    /* 主标题（黑色大字） */
    create_center_label("Hello HSPI", &lv_font_montserrat_28, lv_color_black());

    /* 底部版本信息 */
    lv_obj_t *ver_label = lv_label_create(lv_scr_act());
    lv_label_set_text(ver_label, "SiFli-SDK v2.4 | LVGL V8");
    lv_obj_set_style_text_color(ver_label, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_obj_set_style_text_font(ver_label, &lv_font_montserrat_12, 0);
    lv_obj_align(ver_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    /* 清空图像缓存 */
    lv_img_cache_invalidate_src(NULL);

    rt_kprintf("Lv.0: White background + text ready\n");

    /* 4. 主循环 */
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }

    return RT_EOK;
}
