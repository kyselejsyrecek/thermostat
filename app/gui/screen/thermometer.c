#include "lvgl.h"

#include "config.h"
#include "gui/common.h"
#include "sensor/thermometer.h"

#include "thermometer.h"

static ThermometerScreen s_screen;

/* ── Observer callback ───────────────────────────────────────────────────── */

static void thermometer_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    ThermometerScreen *scr = (ThermometerScreen *)lv_observer_get_user_data(observer);
    gui_temp_label_update(&scr->labels, lv_subject_get_int(subject));
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

ThermometerScreen *thermometer_screen_create(lv_obj_t *parent,
                                             Thermometer *thermometer)
{
    s_screen.root = parent;

    /* Black background – no gradient for this screen. */
    lv_obj_set_style_bg_color(parent, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    gui_temp_labels_create(parent, &s_screen.labels, UI_FG_COLOR);

    /* Subscribe to thermometer; observer is auto-removed when int_lbl is
     * deleted (lifetime tied to the parent widget). */
    lv_subject_add_observer_obj(&thermometer->value, thermometer_observer_cb,
                                s_screen.labels.int_lbl, &s_screen);

    return &s_screen;
}
