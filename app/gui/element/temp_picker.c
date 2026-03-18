#include "lvgl.h"

#include "config.h"
#include "temp_picker.h"

/* ── Step ↔ temperature mapping ─────────────────────────────────────────────
 *
 * The arc is divided into three zones:
 *   [UI_TEMP_MIN … UI_TEMP_FINE_MIN)   – coarse steps (UI_TEMP_PREC_NORMAL)
 *   [UI_TEMP_FINE_MIN … UI_TEMP_FINE_MAX] – fine steps  (UI_TEMP_PREC_FINE)
 *   (UI_TEMP_FINE_MAX … UI_TEMP_MAX]   – coarse steps (UI_TEMP_PREC_NORMAL)
 *
 * All temperatures are stored in tenths of a degree internally.
 * ────────────────────────────────────────────────────────────────────────── */

static int32_t arc_total_steps(void)
{
    return (UI_TEMP_FINE_MIN - UI_TEMP_MIN)      * 10 / UI_TEMP_PREC_NORMAL
         + (UI_TEMP_FINE_MAX - UI_TEMP_FINE_MIN) * 10 / UI_TEMP_PREC_FINE
         + (UI_TEMP_MAX      - UI_TEMP_FINE_MAX) * 10 / UI_TEMP_PREC_NORMAL;
}

int32_t temp_picker_step_to_tenths(int32_t step)
{
    int32_t low_steps  = (UI_TEMP_FINE_MIN - UI_TEMP_MIN)      * 10 / UI_TEMP_PREC_NORMAL;
    int32_t fine_steps = (UI_TEMP_FINE_MAX - UI_TEMP_FINE_MIN) * 10 / UI_TEMP_PREC_FINE;
    if(step <= low_steps) {
        return UI_TEMP_MIN * 10 + step * UI_TEMP_PREC_NORMAL;
    } else if(step <= low_steps + fine_steps) {
        return UI_TEMP_FINE_MIN * 10 + (step - low_steps) * UI_TEMP_PREC_FINE;
    } else {
        return UI_TEMP_FINE_MAX * 10 + (step - low_steps - fine_steps) * UI_TEMP_PREC_NORMAL;
    }
}

int32_t temp_picker_tenths_to_step(int32_t tenths)
{
    int32_t low_steps  = (UI_TEMP_FINE_MIN - UI_TEMP_MIN)      * 10 / UI_TEMP_PREC_NORMAL;
    int32_t fine_steps = (UI_TEMP_FINE_MAX - UI_TEMP_FINE_MIN) * 10 / UI_TEMP_PREC_FINE;
    if(tenths <= UI_TEMP_FINE_MIN * 10) {
        return (tenths - UI_TEMP_MIN * 10) / UI_TEMP_PREC_NORMAL;
    } else if(tenths <= UI_TEMP_FINE_MAX * 10) {
        return low_steps + (tenths - UI_TEMP_FINE_MIN * 10) / UI_TEMP_PREC_FINE;
    } else {
        return low_steps + fine_steps + (tenths - UI_TEMP_FINE_MAX * 10) / UI_TEMP_PREC_NORMAL;
    }
}

/* ── Label observer ─────────────────────────────────────────────────────────
 *
 * The observer callback receives a pointer to the owning TempPicker as
 * user_data so it can update both label widgets without a global variable.
 * ────────────────────────────────────────────────────────────────────────── */

#if UI_TEMP_SHOW_DECIMALS

static void label_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    TempPicker *picker = (TempPicker *)lv_observer_get_user_data(observer);
    int32_t tenths = temp_picker_step_to_tenths(lv_subject_get_int(subject));
    char buf[16];

    lv_snprintf(buf, sizeof(buf), "%d", (int)(tenths / 10));
    lv_label_set_text(picker->int_lbl, buf);

    lv_snprintf(buf, sizeof(buf), "%c%d \xc2\xb0" "C", UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
    lv_label_set_text(picker->frac_lbl, buf);
}

/* Max pixel width of the integer part across the full temperature range. */
static int32_t max_int_width(void)
{
    int32_t max_w = 0;
    char buf[8];
    int32_t steps = arc_total_steps();
    for(int32_t s = 0; s <= steps; s++) {
        lv_snprintf(buf, sizeof(buf), "%d", (int)(temp_picker_step_to_tenths(s) / 10));
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

/* Max pixel width of the fractional part: separator + digit + " °C". */
static int32_t max_frac_width(void)
{
    int32_t max_w = 0;
    char buf[16];
    for(int frac = 0; frac <= 9; frac++) {
        lv_snprintf(buf, sizeof(buf), "%c%d \xc2\xb0" "C", UI_TEMP_DECIMAL_SEP, frac);
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

static void labels_create(lv_obj_t *arc, TempPicker *out)
{
    int32_t int_w  = max_int_width();
    int32_t frac_w = max_frac_width();

    /* Integer part – right-aligned box whose centre is at arc_centre - frac_w/2.
     * Together with the fractional box the full text block is horizontally centred. */
    out->int_lbl = lv_label_create(arc);
    lv_obj_set_width(out->int_lbl, int_w);
    lv_obj_set_style_text_align(out->int_lbl, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(out->int_lbl, LV_ALIGN_CENTER, -(frac_w / 2), 0);
    lv_obj_set_style_text_font(out->int_lbl, &UI_FONT, 0);
    lv_obj_set_style_text_color(out->int_lbl, UI_FG_COLOR, 0);

    /* Fractional part – left-aligned box whose centre is at arc_centre + int_w/2.
     * The decimal separator therefore stays fixed regardless of digit count. */
    out->frac_lbl = lv_label_create(arc);
    lv_obj_set_width(out->frac_lbl, frac_w);
    lv_obj_set_style_text_align(out->frac_lbl, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(out->frac_lbl, LV_ALIGN_CENTER, (int_w / 2), 0);
    lv_obj_set_style_text_font(out->frac_lbl, &UI_FONT, 0);
    lv_obj_set_style_text_color(out->frac_lbl, UI_FG_COLOR, 0);

    lv_subject_add_observer_obj(&out->value, label_observer_cb, out->int_lbl, out);
}

#else /* !UI_TEMP_SHOW_DECIMALS */

static void label_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    TempPicker *picker = (TempPicker *)lv_observer_get_user_data(observer);
    char buf[24];
    lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C",
                (int)(temp_picker_step_to_tenths(lv_subject_get_int(subject)) / 10));
    lv_label_set_text(picker->int_lbl, buf);
}

/* Max pixel width of the whole-number label across the full range. */
static int32_t max_whole_width(void)
{
    int32_t max_w = 0;
    char buf[24];
    int32_t steps = arc_total_steps();
    for(int32_t s = 0; s <= steps; s++) {
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C",
                    (int)(temp_picker_step_to_tenths(s) / 10));
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

static void labels_create(lv_obj_t *arc, TempPicker *out)
{
    out->frac_lbl = NULL;
    out->int_lbl  = lv_label_create(arc);
    lv_obj_set_width(out->int_lbl, max_whole_width());
    lv_obj_set_style_text_align(out->int_lbl, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_center(out->int_lbl);
    lv_obj_set_style_text_font(out->int_lbl, &UI_FONT, 0);
    lv_obj_set_style_text_color(out->int_lbl, UI_FG_COLOR, 0);

    lv_subject_add_observer_obj(&out->value, label_observer_cb, out->int_lbl, out);
}

#endif /* UI_TEMP_SHOW_DECIMALS */

/* ── Public API ─────────────────────────────────────────────────────────────*/

void temp_picker_create(lv_obj_t *parent, TempPicker *out)
{
    lv_subject_init_int(&out->value, temp_picker_tenths_to_step(UI_TEMP_DEFAULT * 10));

    lv_obj_t *arc = lv_arc_create(parent);
    static const uint8_t dial_scale_perc = 58;
    lv_obj_set_size(arc,
                    LV_HOR_RES * dial_scale_perc / 100,
                    LV_VER_RES * dial_scale_perc / 100);
    lv_obj_center(arc);
    lv_arc_set_range(arc, 0, arc_total_steps());
    lv_arc_bind_value(arc, &out->value);

    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, UI_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arc, UI_FG_COLOR, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc, 15, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, LV_OPA_40, LV_PART_KNOB);
    lv_obj_set_style_shadow_offset_y(arc, 5, LV_PART_KNOB);

    out->arc = arc;
    labels_create(arc, out);
}
