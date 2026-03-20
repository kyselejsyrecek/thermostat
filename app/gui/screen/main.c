#include "lvgl.h"

#include "config.h"
#include "gui/common.h"
#include "sensor/thermometer.h"

#include "main.h"

static MainScreen s_screen;

/* ── Observer callback ───────────────────────────────────────────────────── */

static void thermometer_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    MainScreen *scr = (MainScreen *)lv_observer_get_user_data(observer);
    gui_temp_label_update(&scr->labels, lv_subject_get_int(subject));
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

MainScreen *main_screen_create(lv_obj_t *parent, Thermometer *thermometer)
{
    lv_obj_t *root = gui_screen_root_create(parent);
    s_screen.root = root;

    /* Black background – no gradient for this screen. */
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    gui_temp_labels_create(root, &s_screen.labels, UI_FG_COLOR);

    /* Subscribe to thermometer; observer is auto-removed when int_lbl is
     * deleted (lifetime tied to the parent widget). */
    lv_subject_add_observer_obj(&thermometer->value, thermometer_observer_cb,
                                s_screen.labels.int_lbl, &s_screen);

    return &s_screen;
}
