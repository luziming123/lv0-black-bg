/**
 * @file    lv_analogclock.h
 * @brief   NXP GUI Guider lv_analogclock API 兼容层
 * @note    用 LVGL V8 标准控件（lv_line）实现 NXP 的模拟时钟 API
 *          GUI Guider 导出的代码无需修改，直接调用这些函数
 */

#ifndef LV_ANALOGCLOCK_H
#define LV_ANALOGCLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <time.h>
#include <math.h>

/* 让 #if LV_USE_ANALOGCLOCK 在 widgets_init.c 中通过 */
#ifndef LV_USE_ANALOGCLOCK
#define LV_USE_ANALOGCLOCK 1
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ==================== 内部数据结构 ==================== */

typedef struct {
    lv_obj_t *tick_lines[60];       /* 60 根刻度线 */
    lv_obj_t *hour_hand;            /* 时针 */
    lv_obj_t *min_hand;             /* 分针 */
    lv_obj_t *sec_hand;             /* 秒针 */
    lv_point_t hour_pts[2];
    lv_point_t min_pts[2];
    lv_point_t sec_pts[2];
    int hour, min, sec;
    int cx, cy;                     /* 中心点（相对于父对象） */
    int radius;
} lv_analogclock_data_t;

/* ==================== 内部辅助函数 ==================== */

static void _analogclock_update_hand(lv_obj_t *hand, lv_point_t *pts,
                                      int cx, int cy, int len, float angle_deg)
{
    float rad = (angle_deg - 90.0f) * M_PI / 180.0f;
    pts[0].x = cx;
    pts[0].y = cy;
    pts[1].x = cx + (int)(len * cosf(rad));
    pts[1].y = cy + (int)(len * sinf(rad));
    lv_line_set_points(hand, pts, 2);
    lv_obj_invalidate(hand);
}

static void _analogclock_refresh(lv_analogclock_data_t *d)
{
    float sec_angle  = d->sec * 6.0f;
    float min_angle  = d->min * 6.0f + d->sec * 0.1f;
    float hour_angle = (d->hour % 12) * 30.0f + d->min * 0.5f;

    int sec_len  = d->radius - 10;
    int min_len  = d->radius - 20;
    int hour_len = d->radius - 40;

    _analogclock_update_hand(d->sec_hand,  d->sec_pts,  d->cx, d->cy, sec_len,  sec_angle);
    _analogclock_update_hand(d->min_hand,  d->min_pts,  d->cx, d->cy, min_len,  min_angle);
    _analogclock_update_hand(d->hour_hand, d->hour_pts, d->cx, d->cy, hour_len, hour_angle);
}

static void _analogclock_draw_ticks(lv_analogclock_data_t *d, lv_obj_t *parent)
{
    for (int i = 0; i < 60; i++) {
        float angle = i * 6.0f;
        float rad = (angle - 90.0f) * M_PI / 180.0f;
        int outer_r = d->radius;
        int inner_r = (i % 5 == 0) ? d->radius - 15 : d->radius - 8;
        int w = (i % 5 == 0) ? 3 : 1;

        lv_obj_t *tick = lv_line_create(parent);
        lv_obj_set_style_line_width(tick, w, 0);
        lv_obj_set_style_line_color(tick, lv_color_hex(0x555555), 0);
        lv_obj_clear_flag(tick, LV_OBJ_FLAG_CLICKABLE);

        lv_point_t *tp = lv_mem_alloc(sizeof(lv_point_t) * 2);
        tp[0].x = d->cx + (int)(outer_r * cosf(rad));
        tp[0].y = d->cy + (int)(outer_r * sinf(rad));
        tp[1].x = d->cx + (int)(inner_r * cosf(rad));
        tp[1].y = d->cy + (int)(inner_r * sinf(rad));
        lv_line_set_points(tick, tp, 2);
        d->tick_lines[i] = tick;
    }
}

static lv_obj_t *_analogclock_create_hand(lv_obj_t *parent, int w, lv_color_t color)
{
    lv_obj_t *line = lv_line_create(parent);
    lv_obj_set_style_line_color(line, color, 0);
    lv_obj_set_style_line_width(line, w, 0);
    lv_obj_set_style_line_rounded(line, true, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
    return line;
}

/* ==================== NXP lv_analogclock API ==================== */

static inline lv_obj_t *lv_analogclock_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_analogclock_data_t *d = lv_mem_alloc(sizeof(lv_analogclock_data_t));
    lv_memset(d, 0, sizeof(lv_analogclock_data_t));
    d->radius = 140;   /* 默认半径，后面会被 lv_obj_set_size 覆盖 */
    d->cx = 150;
    d->cy = 150;

    /* 创建中心点 */
    lv_obj_t *dot = lv_obj_create(obj);
    lv_obj_set_size(dot, 12, 12);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(dot, 6, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(dot, LV_ALIGN_CENTER, 0, 0);

    /* 创建刻度 */
    _analogclock_draw_ticks(d, obj);

    /* 创建指针 */
    d->hour_hand = _analogclock_create_hand(obj, 2, lv_color_hex(0x00ff00));
    d->min_hand  = _analogclock_create_hand(obj, 2, lv_color_hex(0xE1FF00));
    d->sec_hand  = _analogclock_create_hand(obj, 2, lv_color_hex(0x6600FF));

    /* 初始化指针位置 */
    lv_point_t init_pts[2] = {{d->cx, d->cy}, {d->cx, d->cy}};
    lv_line_set_points(d->hour_hand, init_pts, 2);
    lv_line_set_points(d->min_hand,  init_pts, 2);
    lv_line_set_points(d->sec_hand,  init_pts, 2);

    lv_obj_set_user_data(obj, d);
    return obj;
}

static inline void lv_analogclock_hide_digits(lv_obj_t *obj, bool hide)
{
    /* 刻度已在线条中绘制，此 API 仅作兼容 */
    (void)obj; (void)hide;
}

static inline void lv_analogclock_set_major_ticks(lv_obj_t *obj, lv_coord_t width,
                                                    lv_coord_t len, lv_color_t color, int cnt_gap)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    for (int i = 0; i < 60; i++) {
        if (d->tick_lines[i] && (i % cnt_gap == 0)) {
            lv_obj_set_style_line_width(d->tick_lines[i], width, 0);
            lv_obj_set_style_line_color(d->tick_lines[i], color, 0);
        }
    }
}

static inline void lv_analogclock_set_ticks(lv_obj_t *obj, lv_coord_t width,
                                              lv_coord_t len, lv_color_t color)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    for (int i = 0; i < 60; i++) {
        if (d->tick_lines[i] && (i % 5 != 0)) {
            lv_obj_set_style_line_width(d->tick_lines[i], width, 0);
            lv_obj_set_style_line_color(d->tick_lines[i], color, 0);
        }
    }
}

static inline void lv_analogclock_set_hour_needle_line(lv_obj_t *obj, lv_coord_t width,
                                                         lv_color_t color, int offset)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    lv_obj_set_style_line_width(d->hour_hand, width, 0);
    lv_obj_set_style_line_color(d->hour_hand, color, 0);
    (void)offset;
}

static inline void lv_analogclock_set_min_needle_line(lv_obj_t *obj, lv_coord_t width,
                                                        lv_color_t color, int offset)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    lv_obj_set_style_line_width(d->min_hand, width, 0);
    lv_obj_set_style_line_color(d->min_hand, color, 0);
    (void)offset;
}

static inline void lv_analogclock_set_sec_needle_line(lv_obj_t *obj, lv_coord_t width,
                                                        lv_color_t color, int offset)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    lv_obj_set_style_line_width(d->sec_hand, width, 0);
    lv_obj_set_style_line_color(d->sec_hand, color, 0);
    (void)offset;
}

static inline void lv_analogclock_set_time(lv_obj_t *obj, int hour, int min, int sec)
{
    lv_analogclock_data_t *d = (lv_analogclock_data_t *)lv_obj_get_user_data(obj);
    if (!d) return;
    d->hour = hour;
    d->min  = min;
    d->sec  = sec;

    /* 更新中心和半径（从控件实际尺寸获取） */
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    d->cx = w / 2;
    d->cy = h / 2;
    d->radius = (w < h ? w : h) / 2 - 5;

    _analogclock_refresh(d);
}

#ifdef __cplusplus
}
#endif

#endif /* LV_ANALOGCLOCK_H */
