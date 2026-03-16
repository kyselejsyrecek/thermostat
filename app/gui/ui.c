#include "lvgl.h"
#include "ui.h"

void ui_init(void)
{
    /* Screen background: black outside the circular display area */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);

    /* Circular container – clips all children to a circle.
     * The gradient is applied here; the screen (corners) stays black. */
    lv_obj_t * circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(circle, true, 0);
    lv_obj_set_style_pad_all(circle, 0, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE);

    /* Gradient background: green (bottom-left) -> teal (top-right).
     *
     * LVGL SW renderer uses int32 arithmetic with a fixed-point <<16 shift.
     * For a 360x360 display, using LV_GRAD_BOTTOM (100 %) as y-start overflows
     * int32 (360 * 360 * 65536 > INT32_MAX).  Using LV_GRAD_CENTER (50 %) maps
     * to 180 px, which stays within range and still produces a correct smooth
     * 45° diagonal thanks to EXTEND_PAD clamping to the endpoint colours. */
    static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    static lv_grad_dsc_t grad;
    static const lv_color_t grad_colors[2] = {
        UI_GRAD_COLOR_START,
        UI_GRAD_COLOR_END,
    };
    lv_grad_init_stops(&grad, grad_colors, NULL, NULL, 2);
    lv_grad_linear_init(&grad,
                        UI_GRAD_START_X, UI_GRAD_START_Y,
                        UI_GRAD_END_X,   UI_GRAD_END_Y,
                        LV_GRAD_EXTEND_PAD);
    lv_style_set_bg_grad(&style_bg, &grad);
    lv_obj_add_style(circle, &style_bg, 0);

    static lv_subject_t value;
    lv_subject_init_int(&value, UI_TEMP_DEFAULT);

    lv_obj_t * arc = lv_arc_create(circle);
    lv_obj_set_size(arc, LV_HOR_RES * 55 / 100, LV_VER_RES * 55 / 100);
    lv_obj_center(arc);
    lv_arc_set_range(arc, UI_TEMP_MIN, UI_TEMP_MAX);
    lv_arc_bind_value(arc, &value);

    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, UI_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arc, UI_FG_COLOR, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc, 15, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, LV_OPA_40, LV_PART_KNOB);
    lv_obj_set_style_shadow_offset_y(arc, 5, LV_PART_KNOB);

    lv_obj_t * label = lv_label_create(arc);

    /* Measure the widest rendered string in the range [UI_TEMP_MIN, UI_TEMP_MAX]
     * so the label box fits exactly and does not shift the display centre. */
    int32_t max_w = 0;
    char buf[16];
    for(int v = UI_TEMP_MIN; v <= UI_TEMP_MAX; v++) {
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0""C", v);   /* "N °C" in UTF-8 */
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX,
                         LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    lv_obj_set_width(label, max_w);

    /* RIGHT-align: digits grow to the left while "°C" stays fixed on the right. */
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_center(label);
    lv_label_bind_text(label, &value, "%d °C");
    lv_obj_set_style_text_font(label, &UI_FONT, 0);
    lv_obj_set_style_text_color(label, UI_FG_COLOR, 0);
}
