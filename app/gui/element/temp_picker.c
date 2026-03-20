#include "lvgl.h"
#include "core/lv_obj_event_private.h"   /* lv_hit_test_info_t fields (read-only use) */

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
    return (ARC_FINE_MIN - UI_TEMP_MIN)  / ARC_PREC_NORMAL
         + (ARC_FINE_MAX - ARC_FINE_MIN) / ARC_PREC_FINE
         + (UI_TEMP_MAX  - ARC_FINE_MAX) / ARC_PREC_NORMAL;
}

static int32_t temp_picker_step_to_tenths(int32_t step)
{
    int32_t low_steps  = (ARC_FINE_MIN - UI_TEMP_MIN)  / ARC_PREC_NORMAL;
    int32_t fine_steps = (ARC_FINE_MAX - ARC_FINE_MIN) / ARC_PREC_FINE;
    if(step <= low_steps) {
        return UI_TEMP_MIN  + step * ARC_PREC_NORMAL;
    } else if(step <= low_steps + fine_steps) {
        return ARC_FINE_MIN + (step - low_steps) * ARC_PREC_FINE;
    } else {
        return ARC_FINE_MAX + (step - low_steps - fine_steps) * ARC_PREC_NORMAL;
    }
}

static int32_t temp_picker_tenths_to_step(int32_t tenths)
{
    int32_t low_steps  = (ARC_FINE_MIN - UI_TEMP_MIN)  / ARC_PREC_NORMAL;
    int32_t fine_steps = (ARC_FINE_MAX - ARC_FINE_MIN) / ARC_PREC_FINE;
    if(tenths <= ARC_FINE_MIN) {
        return (tenths - UI_TEMP_MIN) / ARC_PREC_NORMAL;
    } else if(tenths <= ARC_FINE_MAX) {
        return low_steps + (tenths - ARC_FINE_MIN) / ARC_PREC_FINE;
    } else {
        return low_steps + fine_steps + (tenths - ARC_FINE_MAX) / ARC_PREC_NORMAL;
    }
}

/* ── Step → tenths observer ─────────────────────────────────────────────────
 * Fires whenever the arc step changes (user interaction or programmatic).
 * Converts step → tenths and publishes to the public value subject.
 * ────────────────────────────────────────────────────────────────────────── */

static void step_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    TempPicker *picker = (TempPicker *)lv_observer_get_user_data(observer);
    lv_subject_set_int(&picker->value,
                       temp_picker_step_to_tenths(lv_subject_get_int(subject)));
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
    gui_temp_label_update(&picker->labels, lv_subject_get_int(subject));
}

/* ── Knob-only guard ─────────────────────────────────────────────────────────
 * When UI_TEMP_PICKER_RESPONSIVITY_KNOB_ONLY is enabled the arc receives a
 * LV_EVENT_HIT_TEST callback that runs AFTER arc's own class handler.  If the
 * touch point is outside the knob area we set info->res = false so the object
 * never becomes the active indev target → no PRESSED / PRESSING ever fires.
 *
 * Geometry mirrors lv_arc.c's static get_center() + get_knob_area() using
 * only public LVGL style/layout APIs – no LVGL source files are modified.
 * ────────────────────────────────────────────────────────────────────────── */
#if UI_TEMP_PICKER_RESPONSIVITY_KNOB_ONLY
static void arc_knob_hittest_cb(lv_event_t *e)
{
    lv_hit_test_info_t *info = lv_event_get_hit_test_info(e);
    /* If arc's own handler already rejected (e.g. dead-zone inside) keep it. */
    if(!info->res) return;

    lv_obj_t *obj = lv_event_get_current_target(e);

    /* ── Replicate get_center() ────────────────────────────────────────────
     * centre and radius use the BG-part padding, matching the arc source. */
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);   /* absolute screen coordinates */
    int32_t left_bg   = lv_obj_get_style_pad_left  (obj, LV_PART_MAIN);
    int32_t right_bg  = lv_obj_get_style_pad_right (obj, LV_PART_MAIN);
    int32_t top_bg    = lv_obj_get_style_pad_top   (obj, LV_PART_MAIN);
    int32_t bottom_bg = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);
    int32_t r = LV_MIN(lv_area_get_width (&coords) - left_bg - right_bg,
                       lv_area_get_height(&coords) - top_bg  - bottom_bg) / 2;
    lv_point_t center = {
        .x = coords.x1 + left_bg + r,
        .y = coords.y1 + top_bg  + r,
    };

    /* ── Replicate get_knob_area() radius ──────────────────────────────────
     * subtract indic_width/2 and indicator-part max padding. */
    int32_t indic_w     = lv_obj_get_style_arc_width(obj, LV_PART_INDICATOR);
    int32_t indic_w2    = indic_w / 2;
    int32_t indic_max_pad = LV_MAX4(
        lv_obj_get_style_pad_left  (obj, LV_PART_INDICATOR),
        lv_obj_get_style_pad_right (obj, LV_PART_INDICATOR),
        lv_obj_get_style_pad_top   (obj, LV_PART_INDICATOR),
        lv_obj_get_style_pad_bottom(obj, LV_PART_INDICATOR));
    int32_t arc_r = r - indic_w2 - indic_max_pad;

    /* ── Knob angle: rotation + indicator-end angle ────────────────────── */
    int32_t knob_ang = (int32_t)lv_arc_get_angle_end(obj)
                     + lv_arc_get_rotation(obj)
                     + lv_arc_get_knob_offset(obj);

    /* ── Knob centre in screen coords (lv_trigo_sin: 0°=right, LV_TRIGO_SHIFT scale) */
    int32_t kx = center.x + ((arc_r * lv_trigo_sin(knob_ang + 90)) >> LV_TRIGO_SHIFT);
    int32_t ky = center.y + ((arc_r * lv_trigo_sin(knob_ang))      >> LV_TRIGO_SHIFT);

    /* ── Hit radius: half indic-width + knob padding, scaled by config ─── */
    int32_t knob_r = (indic_w2
                   + lv_obj_get_style_pad_top(obj, LV_PART_KNOB))
                   * UI_TEMP_PICKER_KNOB_HIT_SCALE / 10;

    int32_t dx = info->point->x - kx;
    int32_t dy = info->point->y - ky;
    if(dx * dx + dy * dy > knob_r * knob_r)
        info->res = false;
}
#endif

static void labels_create(lv_obj_t *arc, TempPicker *out)
{
    gui_temp_labels_create(arc, &out->labels, UI_FG_COLOR, &UI_FONT);
    lv_subject_add_observer_obj(&out->value, label_observer_cb,
                                out->labels.int_lbl, out);
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

void temp_picker_set_tenths(TempPicker *picker, int32_t tenths)
{
    lv_subject_set_int(&picker->arc_step, temp_picker_tenths_to_step(tenths));
}

void temp_picker_create(lv_obj_t *parent, TempPicker *out)
{
    /* arc_step is the internal arc index; value exposes the result in tenths.
     * arc_step drives the arc widget via lv_arc_bind_value; step_observer_cb
     * converts step → tenths and writes to value. */
    lv_subject_init_int(&out->arc_step,
                        temp_picker_tenths_to_step(UI_TEMP_DEFAULT));
    lv_subject_init_int(&out->value, UI_TEMP_DEFAULT);

    lv_obj_t *arc = lv_arc_create(parent);
    static const uint8_t dial_scale_perc = 58;
    lv_obj_set_size(arc,
                    LV_HOR_RES * dial_scale_perc / 100,
                    LV_VER_RES * dial_scale_perc / 100);
    lv_obj_center(arc);
    lv_arc_set_range(arc, 0, arc_total_steps());
    lv_arc_bind_value(arc, &out->arc_step);   /* arc ⇄ arc_step */

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

#if UI_TEMP_PICKER_RESPONSIVITY_KNOB_ONLY
    lv_obj_add_event_cb(arc, arc_knob_hittest_cb, LV_EVENT_HIT_TEST, NULL);
#endif

    /* arc_step → value (tenths) conversion observer.  Must be added after
     * lv_arc_bind_value so that the initial arc display is set first. */
    lv_subject_add_observer(&out->arc_step, step_observer_cb, out);

    labels_create(arc, out);
}
