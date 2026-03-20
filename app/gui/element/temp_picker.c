#include "lvgl.h"

#include "config.h"
#include "gui/common.h"

#include "temp_picker.h"

/* ── Step ↔ temperature mapping ─────────────────────────────────────────────
 *
 * The arc is divided into three zones:
 *   [UI_TEMP_MIN … ARC_FINE_MIN)        – coarse steps (ARC_PREC_NORMAL)
 *   [ARC_FINE_MIN … ARC_FINE_MAX]       – fine steps   (ARC_PREC_FINE)
 *   (ARC_FINE_MAX … UI_TEMP_MAX]        – coarse steps (ARC_PREC_NORMAL)
 *
 * When UI_TEMP_ENABLE_DECIMALS is 0, sub-degree positions are not
 * representable in the label; the fine zone is collapsed (ARC_FINE_MIN =
 * ARC_FINE_MAX = UI_TEMP_MIN) and the arc snaps at 1 °C intervals.
 *
 * All temperatures are stored in tenths of a degree internally.
 * ────────────────────────────────────────────────────────────────────────── */

#if UI_TEMP_ENABLE_DECIMALS
#  define ARC_FINE_MIN    UI_TEMP_FINE_MIN
#  define ARC_FINE_MAX    UI_TEMP_FINE_MAX
#  define ARC_PREC_NORMAL UI_TEMP_PREC_NORMAL
#  define ARC_PREC_FINE   UI_TEMP_PREC_FINE
#else
#  define ARC_FINE_MIN    UI_TEMP_MIN
#  define ARC_FINE_MAX    UI_TEMP_MIN
#  define ARC_PREC_NORMAL UI_TEMP_PREC_WHOLE
#  define ARC_PREC_FINE   UI_TEMP_PREC_WHOLE
#endif

static int32_t arc_total_steps(void)
{
    return (ARC_FINE_MIN - UI_TEMP_MIN)  * 10 / ARC_PREC_NORMAL
         + (ARC_FINE_MAX - ARC_FINE_MIN) * 10 / ARC_PREC_FINE
         + (UI_TEMP_MAX  - ARC_FINE_MAX) * 10 / ARC_PREC_NORMAL;
}

static int32_t temp_picker_step_to_tenths(int32_t step)
{
    int32_t low_steps  = (ARC_FINE_MIN - UI_TEMP_MIN)  * 10 / ARC_PREC_NORMAL;
    int32_t fine_steps = (ARC_FINE_MAX - ARC_FINE_MIN) * 10 / ARC_PREC_FINE;
    if(step <= low_steps) {
        return UI_TEMP_MIN  * 10 + step * ARC_PREC_NORMAL;
    } else if(step <= low_steps + fine_steps) {
        return ARC_FINE_MIN * 10 + (step - low_steps) * ARC_PREC_FINE;
    } else {
        return ARC_FINE_MAX * 10 + (step - low_steps - fine_steps) * ARC_PREC_NORMAL;
    }
}

static int32_t temp_picker_tenths_to_step(int32_t tenths)
{
    int32_t low_steps  = (ARC_FINE_MIN - UI_TEMP_MIN)  * 10 / ARC_PREC_NORMAL;
    int32_t fine_steps = (ARC_FINE_MAX - ARC_FINE_MIN) * 10 / ARC_PREC_FINE;
    if(tenths <= ARC_FINE_MIN * 10) {
        return (tenths - UI_TEMP_MIN * 10) / ARC_PREC_NORMAL;
    } else if(tenths <= ARC_FINE_MAX * 10) {
        return low_steps + (tenths - ARC_FINE_MIN * 10) / ARC_PREC_FINE;
    } else {
        return low_steps + fine_steps + (tenths - ARC_FINE_MAX * 10) / ARC_PREC_NORMAL;
    }
}

/* ── Label rendering ─────────────────────────────────────────────────────────
 *
 * Label creation and value formatting are handled by the shared helpers in
 * gui/common.c (gui_temp_labels_create / gui_temp_label_update) so that
 * the same layout is used by every screen that displays a temperature.
 * ────────────────────────────────────────────────────────────────────────── */

/* ── Observer callback ───────────────────────────────────────────────────── */

static void label_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    TempPicker *picker = (TempPicker *)lv_observer_get_user_data(observer);
    int32_t tenths = temp_picker_step_to_tenths(lv_subject_get_int(subject));
    gui_temp_label_update(&picker->labels, tenths);
}

/* ── Label creation ──────────────────────────────────────────────────────── */

static void labels_create(lv_obj_t *arc, TempPicker *out)
{
    gui_temp_labels_create(arc, &out->labels, UI_FG_COLOR);
    lv_subject_add_observer_obj(&out->value, label_observer_cb,
                                out->labels.int_lbl, out);
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
    /* Enable advanced hit-testing so clicks in the arc centre area fall
     * through to the parent screen root (needed for nav double-tap gesture). */
    lv_obj_add_flag(arc, LV_OBJ_FLAG_ADV_HITTEST);
    labels_create(arc, out);
}
