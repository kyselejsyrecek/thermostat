#include "lvgl.h"

#include "config.h"
#include "gui/common.h"
#include "gui/element/notification_bar.h"
#include "gui/element/target_temp.h"
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

MainScreen *main_screen_create(lv_obj_t *parent, Thermometer *thermometer,
                               Battery *battery)
{
    lv_obj_t *root = gui_screen_root_create(parent);
    s_screen.root = root;

    /* Black background – no gradient for this screen. */
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    /* ── Vertical group: current temperature + setpoint ───────────────────── *
     * Both label groups are placed in a transparent container that is centred
     * in the screen, so the pair as a whole is vertically centred.           */
    int32_t curr_lh = (int32_t)UI_FONT_MAIN_TEMP.line_height;
    int32_t setp_lh = (int32_t)UI_FONT_MAIN_TARGET_TEMP.line_height;
    int32_t group_h = curr_lh + UI_MAIN_TEMP_GROUP_GAP + setp_lh;

    lv_obj_t *group = lv_obj_create(root);
    lv_obj_set_size(group, UI_DISPLAY_WIDTH, group_h);
    lv_obj_center(group);
    lv_obj_set_style_pad_all(group, 0, 0);
    lv_obj_set_style_border_width(group, 0, 0);
    lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(group, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(group, LV_OBJ_FLAG_EVENT_BUBBLE);

    /* Row for the measured (current) temperature. */
    lv_obj_t *row_curr = lv_obj_create(group);
    lv_obj_set_size(row_curr, UI_DISPLAY_WIDTH, curr_lh);
    lv_obj_set_pos(row_curr, 0, 0);
    lv_obj_set_style_pad_all(row_curr, 0, 0);
    lv_obj_set_style_border_width(row_curr, 0, 0);
    lv_obj_set_style_bg_opa(row_curr, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(row_curr, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(row_curr, LV_OBJ_FLAG_EVENT_BUBBLE);

    gui_temp_labels_create(row_curr, &s_screen.labels, UI_FG_COLOR, &UI_FONT_MAIN_TEMP);

    /* Row for the setpoint (target) temperature. */
    lv_obj_t *row_setp = lv_obj_create(group);
    lv_obj_set_size(row_setp, UI_DISPLAY_WIDTH, setp_lh);
    lv_obj_set_pos(row_setp, 0, curr_lh + UI_MAIN_TEMP_GROUP_GAP);
    lv_obj_set_style_pad_all(row_setp, 0, 0);
    lv_obj_set_style_border_width(row_setp, 0, 0);
    lv_obj_set_style_bg_opa(row_setp, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(row_setp, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(row_setp, LV_OBJ_FLAG_EVENT_BUBBLE);

    target_temp_create(row_setp, &s_screen.target_temp);

    /* Subscribe to thermometer; observer is auto-removed when int_lbl is
     * deleted (lifetime tied to the parent widget). */
    lv_subject_add_observer_obj(&thermometer->value, thermometer_observer_cb,
                                s_screen.labels.int_lbl, &s_screen);

    notification_bar_create(root, &s_screen.notif_bar, battery);

    return &s_screen;
}
