/**
 * @file    main.c
 * @brief   Lv.0 - 纯色背景 + 表盘时钟 + 按钮交互
 * @note    白色背景，圆形表盘，时/分/秒针，数字时间，点击计数按钮
 */

#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "lvgl.h"

extern rt_err_t littlevgl2rtt_init(const char *name);
extern void lv_ex_data_pool_init(void);

/* ===================== 表盘常量 ===================== */
#define WATCH_CENTER_X    (LV_HOR_RES_MAX / 2)     /* 195 */
#define WATCH_CENTER_Y    (LV_VER_RES_MAX / 2 - 40) /* 偏上，给按钮留空间 */
#define WATCH_RADIUS      130
#define PI                3.14159265f

#define HOUR_HAND_LEN     60
#define MIN_HAND_LEN      85
#define SEC_HAND_LEN      100

/* ===================== 全局变量 ===================== */
static lv_obj_t *g_hour_line   = NULL;
static lv_obj_t *g_min_line    = NULL;
static lv_obj_t *g_sec_line    = NULL;
static lv_obj_t *g_time_label  = NULL;
static lv_obj_t *g_date_label  = NULL;
static lv_obj_t *g_count_label = NULL;
static int g_count = 0;

/* 模拟时间 */
static int g_hour = 10;
static int g_min  = 30;
static int g_sec  = 0;

/* 指针端点坐标（持久化存储，避免栈上临时数组被回收） */
static lv_point_t g_hour_pts[2];
static lv_point_t g_min_pts[2];
static lv_point_t g_sec_pts[2];

/* ===================== 坐标计算 ===================== */

static void calc_hand_point(float angle_deg, int length,
                             lv_coord_t *out_x, lv_coord_t *out_y)
{
    float rad = (angle_deg - 90.0f) * PI / 180.0f;
    *out_x = WATCH_CENTER_X + (int)(length * cosf(rad));
    *out_y = WATCH_CENTER_Y + (int)(length * sinf(rad));
}

/* ===================== 背景圆环 ===================== */

static void create_dial_background(void)
{
    lv_obj_t *arc = lv_arc_create(lv_scr_act());
    lv_obj_set_size(arc, WATCH_RADIUS * 2 + 20, WATCH_RADIUS * 2 + 20);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, -40);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_color(arc, lv_color_make(0xCC, 0xCC, 0xCC), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(0x00, 0x5A, 0xCC), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 4, LV_PART_INDICATOR);

    /* 中心点 */
    lv_obj_t *dot = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dot, 12, 12);
    lv_obj_align(dot, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_bg_color(dot, lv_color_make(0x33, 0x33, 0x33), 0);
    lv_obj_set_style_radius(dot, 6, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
}

/* ===================== 指针 ===================== */

static lv_obj_t *create_hand_line(lv_color_t color, int width)
{
    lv_obj_t *line = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_color(line, color, 0);
    lv_obj_set_style_line_width(line, width, 0);
    lv_obj_set_style_line_rounded(line, true, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
    /* 初始两点都在中心（不可见），等定时器更新 */
    lv_point_t pts[2] = {{WATCH_CENTER_X, WATCH_CENTER_Y},
                          {WATCH_CENTER_X, WATCH_CENTER_Y}};
    lv_line_set_points(line, pts, 2);
    return line;
}

static void update_hand(lv_obj_t *line, lv_point_t *pts,
                         int length, float angle_deg)
{
    lv_coord_t end_x, end_y;
    calc_hand_point(angle_deg, length, &end_x, &end_y);

    pts[0].x = WATCH_CENTER_X;
    pts[0].y = WATCH_CENTER_Y;
    pts[1].x = end_x;
    pts[1].y = end_y;

    lv_line_set_points(line, pts, 2);
    lv_obj_invalidate(line);  /* 强制重绘 */
}

/* ===================== 数字时间 ===================== */

static void create_time_display(void)
{
    g_time_label = lv_label_create(lv_scr_act());
    lv_label_set_text(g_time_label, "10:30:00");
    lv_obj_set_style_text_color(g_time_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_time_label, &lv_font_montserrat_20, 0);
    lv_obj_align(g_time_label, LV_ALIGN_CENTER, 0, 5);

    g_date_label = lv_label_create(lv_scr_act());
    lv_label_set_text(g_date_label, "2026-06-25");
    lv_obj_set_style_text_color(g_date_label, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_obj_set_style_text_font(g_date_label, &lv_font_montserrat_14, 0);
    lv_obj_align(g_date_label, LV_ALIGN_CENTER, 0, 30);
}

/* ===================== 定时器回调 ===================== */

static void clock_timer_cb(lv_timer_t *timer)
{
    g_sec++;
    if (g_sec >= 60) { g_sec = 0; g_min++; }
    if (g_min >= 60) { g_min = 0; g_hour++; }
    if (g_hour >= 24) { g_hour = 0; }

    float sec_angle  = g_sec * 6.0f;
    float min_angle  = g_min * 6.0f + g_sec * 0.1f;
    float hour_angle = g_hour * 30.0f + g_min * 0.5f;

    update_hand(g_sec_line, g_sec_pts, SEC_HAND_LEN, sec_angle);
    update_hand(g_min_line,  g_min_pts,  MIN_HAND_LEN,  min_angle);
    update_hand(g_hour_line, g_hour_pts, HOUR_HAND_LEN, hour_angle);

    lv_label_set_text_fmt(g_time_label, "%02d:%02d:%02d",
                           g_hour, g_min, g_sec);
    lv_obj_invalidate(g_time_label);
}

/* ===================== 创建完整表盘 ===================== */

static void create_watch_face(void)
{
    create_dial_background();
    create_time_display();

    /* 创建指针：
     * 时针 = 深蓝色 8px（粗短）
     * 分针 = 深绿色 5px（中等）
     * 秒针 = 红色    3px（细长）
     * 颜色与白色背景形成强对比
     */
    g_hour_line = create_hand_line(lv_color_make(0x00, 0x33, 0x99), 8);
    g_min_line  = create_hand_line(lv_color_make(0x00, 0x66, 0x33), 5);
    g_sec_line  = create_hand_line(lv_color_make(0xCC, 0x00, 0x00), 3);

    /* 首次绘制指针 */
    clock_timer_cb(NULL);

    /* 每秒更新 */
    lv_timer_create(clock_timer_cb, 1000, NULL);
}

/* ===================== 按钮交互 ===================== */

static void btn_click_cb(lv_event_t *e)
{
    g_count++;
    lv_label_set_text_fmt(g_count_label, "Count: %d", g_count);
    rt_kprintf("Button clicked! Count = %d\n", g_count);
}

static void create_button_area(void)
{
    g_count_label = lv_label_create(lv_scr_act());
    lv_label_set_text(g_count_label, "Count: 0");
    lv_obj_set_style_text_color(g_count_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_count_label, &lv_font_montserrat_20, 0);
    lv_obj_align(g_count_label, LV_ALIGN_CENTER, 0, 90);

    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 140, 44);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 130);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x00, 0x7A, 0xFF), 0);
    lv_obj_set_style_radius(btn, 22, 0);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me");
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_center(btn_label);

    lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, NULL);
}

/* ===================== 标签 ===================== */

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

    lv_ex_data_pool_init();

    /* 2. 白色背景 */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* 3. 顶部标签 */
    create_top_label("SF32LB52 - HuangShanPi", 10);
    create_top_label("Lv.0 - Watch Face + Button", 30);

    /* 4. 表盘 */
    create_watch_face();

    /* 5. 按钮 */
    create_button_area();

    /* 6. 底部版本 */
    lv_obj_t *ver = lv_label_create(lv_scr_act());
    lv_label_set_text(ver, "SiFli-SDK v2.4 | LVGL V8");
    lv_obj_set_style_text_color(ver, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_12, 0);
    lv_obj_align(ver, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_img_cache_invalidate_src(NULL);

    rt_kprintf("Lv.0: Watch face + button ready\n");

    /* 7. 主循环 */
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }

    return RT_EOK;
}
