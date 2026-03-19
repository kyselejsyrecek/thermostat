#include "lvgl.h"

#include "config.h"
#include "temp_picker.h"

/* ── Step ↔ temperature mapping ─────────────────────────────────────────────
 *
 * The arc is divided into three zones:
 *   [UI_TEMP_MIN … UI_TEMP_FINE_MIN)     – coarse steps (UI_TEMP_PREC_NORMAL)
 *   [UI_TEMP_FINE_MIN … UI_TEMP_FINE_MAX] – fine steps  (UI_TEMP_PREC_FINE)
 *   (UI_TEMP_FINE_MAX … UI_TEMP_MAX]     – coarse steps (UI_TEMP_PREC_NORMAL)
 *
 * All temperatures are stored in tenths of a degree internally.
 * ────────────────────────────────────────────────────────────────────────── */

static int32_t arc_total_steps(void)
{
    return (UI_TEMP_FINE_MIN - UI_TEMP_MIN)      * 10 / UI_TEMP_PREC_NORMAL
         + (UI_TEMP_FINE_MAX - UI_TEMP_FINE_MIN) * 10 / UI_TEMP_PREC_FINE
         + (UI_TEMP_MAX      - UI_TEMP_FINE_MAX) * 10 / UI_TEMP_PREC_NORMAL;
}

static int32_t temp_picker_step_to_tenths(int32_t step)
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

static int32_t temp_picker_tenths_to_step(int32_t tenths)
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

/* ── Label rendering ─────────────────────────────────────────────────────────
 *
 * Three layout modes, selected at compile time via config.h:
 *
 *   UI_TEMP_CENTER_EXACT=1          – single centred label (whole string)
 *   UI_TEMP_SHOW_DECIMALS=1,
 *     UI_TEMP_FIXED_UNIT=1          – three labels: [ integer | ,X | °C ]
 *   UI_TEMP_SHOW_DECIMALS=1,
 *     UI_TEMP_FIXED_UNIT=0          – two labels:   [ integer | ,X °C ]
 *   UI_TEMP_SHOW_DECIMALS=0         – single right-aligned label
 *
 * In split-label modes the integer part grows/shrinks to the left while the
 * decimal separator (and optionally the unit) stay at a fixed position.
 * ────────────────────────────────────────────────────────────────────────── */

/* Helper: measure the pixel width of the widest string a label will ever show. */
static int32_t measure_max_width(const char *strings[], int count)
{
    int32_t max_w = 0;
    for(int i = 0; i < count; i++) {
        lv_point_t sz;
        lv_text_get_size(&sz, strings[i], &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

/* Helper: measure the widest rendered integer part across all arc steps. */
static int32_t max_int_part_width(void)
{
    int32_t max_w = 0;
    int32_t steps = arc_total_steps();
    for(int32_t s = 0; s <= steps; s++) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", (int)(temp_picker_step_to_tenths(s) / 10));
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

/* Helper: apply common style properties to a label. */
static void label_style(lv_obj_t *lbl, int32_t w, lv_text_align_t align)
{
    lv_obj_set_width(lbl, w);
    lv_obj_set_style_text_align(lbl, align, 0);
    lv_obj_set_style_text_font(lbl, &UI_FONT, 0);
    lv_obj_set_style_text_color(lbl, UI_FG_COLOR, 0);
}

/* ── Observer callback ───────────────────────────────────────────────────── */

static void label_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    TempPicker *picker = (TempPicker *)lv_observer_get_user_data(observer);
    int32_t tenths = temp_picker_step_to_tenths(lv_subject_get_int(subject));
    char buf[24];

#if UI_TEMP_CENTER_EXACT
    /* Single centred label – format the whole string into int_lbl. */
#  if UI_TEMP_SHOW_DECIMALS
    lv_snprintf(buf, sizeof(buf), "%d%c%d \xc2\xb0" "C",
                (int)(tenths / 10), UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  else
    lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", (int)(tenths / 10));
#  endif
    lv_label_set_text(picker->int_lbl, buf);

#elif UI_TEMP_SHOW_DECIMALS
    /* Split-label: update integer part. */
    lv_snprintf(buf, sizeof(buf), "%d", (int)(tenths / 10));
    lv_label_set_text(picker->int_lbl, buf);

    /* Update decimal part (with or without appended unit). */
#  if UI_TEMP_FIXED_UNIT
    lv_snprintf(buf, sizeof(buf), "%c%d", UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  else
    lv_snprintf(buf, sizeof(buf), "%c%d \xc2\xb0" "C", UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  endif
    lv_label_set_text(picker->frac_lbl, buf);

#else
    /* Whole-degrees mode: single right-aligned label. */
    lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", (int)(tenths / 10));
    lv_label_set_text(picker->int_lbl, buf);
#endif
}

/* ── Label creation ──────────────────────────────────────────────────────── */

static void labels_create(lv_obj_t *arc, TempPicker *out)
{
    out->frac_lbl = NULL;
    out->unit_lbl = NULL;

#if UI_TEMP_CENTER_EXACT
    /* Single label, centred – measure the widest string across all steps. */
    int32_t steps = arc_total_steps();
    int32_t label_w = 0;
    for(int32_t s = 0; s <= steps; s++) {
        int32_t tenths = temp_picker_step_to_tenths(s);
        char buf[24];
#  if UI_TEMP_SHOW_DECIMALS
        lv_snprintf(buf, sizeof(buf), "%d%c%d \xc2\xb0" "C",
                    (int)(tenths / 10), UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  else
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", (int)(tenths / 10));
#  endif
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > label_w) label_w = sz.x;
    }
    out->int_lbl = lv_label_create(arc);
    label_style(out->int_lbl, label_w, LV_TEXT_ALIGN_CENTER);
    lv_obj_center(out->int_lbl);

#elif UI_TEMP_SHOW_DECIMALS
    int32_t int_w = max_int_part_width();

#  if UI_TEMP_FIXED_UNIT
    /* Three-label mode: [ integer | ,X | °C ]
     * Block layout (centred at arc centre c):
     *   centre(int_lbl)  = c - (dec_w + unit_w) / 2
     *   centre(frac_lbl) = c + (int_w - unit_w) / 2
     *   centre(unit_lbl) = c + (int_w + dec_w)  / 2  */
    char dec_strs[10][8];
    const char *dec_ptrs[10];
    for(int d = 0; d <= 9; d++) {
        lv_snprintf(dec_strs[d], sizeof(dec_strs[d]), "%c%d", UI_TEMP_DECIMAL_SEP, d);
        dec_ptrs[d] = dec_strs[d];
    }
    int32_t dec_w  = measure_max_width(dec_ptrs, 10);
    const char *unit_str[] = { " \xc2\xb0" "C" };
    int32_t unit_w = measure_max_width(unit_str, 1);

    out->int_lbl = lv_label_create(arc);
    label_style(out->int_lbl, int_w, LV_TEXT_ALIGN_RIGHT);
    lv_obj_align(out->int_lbl, LV_ALIGN_CENTER, -(dec_w + unit_w) / 2, 0);

    out->frac_lbl = lv_label_create(arc);
    label_style(out->frac_lbl, dec_w, LV_TEXT_ALIGN_LEFT);
    lv_obj_align(out->frac_lbl, LV_ALIGN_CENTER, (int_w - unit_w) / 2, 0);

    out->unit_lbl = lv_label_create(arc);
    label_style(out->unit_lbl, unit_w, LV_TEXT_ALIGN_LEFT);
    lv_obj_align(out->unit_lbl, LV_ALIGN_CENTER, (int_w + dec_w) / 2, 0);
    lv_label_set_text(out->unit_lbl, " \xc2\xb0" "C");

#  else /* UI_TEMP_SHOW_DECIMALS && !UI_TEMP_FIXED_UNIT */
    /* Two-label mode: [ integer | ,X °C ]
     * Decimal separator stays at a fixed horizontal position;
     * the unit travels with the decimal digit. */
    char frac_strs[10][16];
    const char *frac_ptrs[10];
    for(int d = 0; d <= 9; d++) {
        lv_snprintf(frac_strs[d], sizeof(frac_strs[d]),
                    "%c%d \xc2\xb0" "C", UI_TEMP_DECIMAL_SEP, d);
        frac_ptrs[d] = frac_strs[d];
    }
    int32_t frac_w = measure_max_width(frac_ptrs, 10);

    out->int_lbl = lv_label_create(arc);
    label_style(out->int_lbl, int_w, LV_TEXT_ALIGN_RIGHT);
    lv_obj_align(out->int_lbl, LV_ALIGN_CENTER, -(frac_w / 2), 0);

    out->frac_lbl = lv_label_create(arc);
    label_style(out->frac_lbl, frac_w, LV_TEXT_ALIGN_LEFT);
    lv_obj_align(out->frac_lbl, LV_ALIGN_CENTER, (int_w / 2), 0);
#  endif

#else /* !UI_TEMP_CENTER_EXACT && !UI_TEMP_SHOW_DECIMALS */
    /* Single right-aligned label; whole degrees only. */
    int32_t steps = arc_total_steps();
    int32_t label_w = 0;
    for(int32_t s = 0; s <= steps; s++) {
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C",
                    (int)(temp_picker_step_to_tenths(s) / 10));
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > label_w) label_w = sz.x;
    }
    out->int_lbl = lv_label_create(arc);
    label_style(out->int_lbl, label_w, LV_TEXT_ALIGN_RIGHT);
    lv_obj_center(out->int_lbl);
#endif

    lv_subject_add_observer_obj(&out->value, label_observer_cb, out->int_lbl, out);
}

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
