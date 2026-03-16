#include "lvgl.h"
#include "ui.h"
#include "battery_charge.h"

static lv_obj_t *bouding_circle_create()
{
    lv_obj_t * circle;

    /* Screen background: black outside the circular display area */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);

    /* Circular container – clips all children to a circle.
     * The gradient is applied here; the screen (corners) stays black. */
    circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(circle, true, 0);
    lv_obj_set_style_pad_all(circle, 0, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE);

    return circle;
}

static void background_set(lv_obj_t *viewport)
{
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
    static const lv_color_t grad_colors[3] = {
        UI_GRAD_COLOR_START,
        UI_GRAD_COLOR_MID,
        UI_GRAD_COLOR_END,
    };
    static const uint8_t grad_fracs[3] = { 0, UI_GRAD_MID_FRAC, 255 };
    lv_grad_init_stops(&grad, grad_colors, NULL, grad_fracs, 3);
    lv_grad_linear_init(&grad,
                        UI_GRAD_START_X, UI_GRAD_START_Y,
                        UI_GRAD_END_X,   UI_GRAD_END_Y,
                        LV_GRAD_EXTEND_PAD);
    lv_style_set_bg_grad(&style_bg, &grad);
    lv_obj_add_style(viewport, &style_bg, 0);
}

/* Format a temperature value given in tenths of a degree into buf. */
static void temp_label_format(char * buf, size_t size, int32_t tenths)
{
    int whole = (int)(tenths / 10);
    int frac  = (int)(tenths % 10);
#if UI_TEMP_SHOW_DECIMALS
    lv_snprintf(buf, size, "%d%c%d \xc2\xb0" "C", whole, UI_TEMP_DECIMAL_SEP, frac);
#else
    lv_snprintf(buf, size, "%d \xc2\xb0" "C", whole);
    (void)frac;
#endif
}

/* Return the pixel width of the widest possible temperature label string. */
static int32_t temp_label_max_width(void)
{
    int32_t max_w = 0;
    char buf[24];
    int32_t steps = (UI_TEMP_MAX - UI_TEMP_MIN) * 10 / UI_TEMP_PRECISION;
    for(int32_t s = 0; s <= steps; s++) {
        int32_t tenths = UI_TEMP_MIN * 10 + s * UI_TEMP_PRECISION;
        temp_label_format(buf, sizeof(buf), tenths);
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

/* Observer callback: updates the label text whenever the subject changes. */
static void temp_label_observer_cb(lv_observer_t * observer, lv_subject_t * subject)
{
    lv_obj_t * label = lv_observer_get_target_obj(observer);
    int32_t tenths = UI_TEMP_MIN * 10 + lv_subject_get_int(subject) * UI_TEMP_PRECISION;
    char buf[24];
    temp_label_format(buf, sizeof(buf), tenths);
    lv_label_set_text(label, buf);
}

static void temp_picker_create(lv_obj_t *viewport)
{
    static lv_subject_t value;
    lv_subject_init_int(&value, (UI_TEMP_DEFAULT - UI_TEMP_MIN) * 10 / UI_TEMP_PRECISION);

    /* The arced temperature picker. */
    lv_obj_t * arc = lv_arc_create(viewport);
    static const uint8_t dial_scale_perc = 58;
    lv_obj_set_size(arc, LV_HOR_RES * dial_scale_perc / 100, LV_VER_RES * dial_scale_perc / 100);
    lv_obj_center(arc);
    lv_arc_set_range(arc, 0, (UI_TEMP_MAX - UI_TEMP_MIN) * 10 / UI_TEMP_PRECISION);
    lv_arc_bind_value(arc, &value);

    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, UI_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arc, UI_FG_COLOR, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc, 15, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, LV_OPA_40, LV_PART_KNOB);
    lv_obj_set_style_shadow_offset_y(arc, 5, LV_PART_KNOB);

    /* Value of the temperature picker, drawn in the middle of the arc. */
    lv_obj_t * label = lv_label_create(arc);
    lv_obj_set_width(label, temp_label_max_width());

    /* RIGHT-align: digits grow to the left while "°C" stays fixed on the right. */
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_center(label);
    lv_subject_add_observer_obj(&value, temp_label_observer_cb, label, NULL);
    lv_obj_set_style_text_font(label, &UI_FONT, 0);
    lv_obj_set_style_text_color(label, UI_FG_COLOR, 0);
}

static void notification_bar_create(lv_obj_t *viewport)
{
    /* Battery / charging status icon sits in the arc gap at the bottom.
     * The arc bottom edge is at ~284 px; screen bottom at 360 px.
     * Centre of gap ≈ 322 px from top → offset from BOTTOM_MID = -32. */
    lv_obj_t * battery = lv_image_create(viewport);
    lv_image_set_src(battery, &battery_charge_icon);
    lv_obj_set_style_image_recolor(battery, UI_FG_COLOR, 0);
    lv_obj_set_style_image_recolor_opa(battery, LV_OPA_COVER, 0);
    lv_obj_align(battery, LV_ALIGN_BOTTOM_MID, 0, -54);
}

void ui_init(void)
{
    lv_obj_t *viewport = bouding_circle_create();
    background_set(viewport);
    temp_picker_create(viewport);
    notification_bar_create(viewport);
}
