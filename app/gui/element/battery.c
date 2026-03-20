#include "lvgl.h"

#include "config.h"
#include "gui/battery_charge.h"
#include "gui/animations.h"

#include "battery.h"

/* ── Icon factory ────────────────────────────────────────────────────────── */

static lv_obj_t *icon_create(lv_obj_t *parent, lv_color_t color)
{
    lv_obj_t *img = lv_image_create(parent);
    lv_image_set_src(img, &battery_charge_icon);
    lv_obj_set_style_image_recolor    (img, color,        0);
    lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, 0);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, UI_DISPLAY_HEIGHT * 14 / 17);
    return img;
}

/* ── Charging observer ───────────────────────────────────────────────────── */

static void charging_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    lv_obj_t *icon = (lv_obj_t *)lv_observer_get_user_data(observer);
    if(lv_subject_get_int(subject))
        lv_obj_remove_flag(icon, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
}

/* ── Level observer ──────────────────────────────────────────────────────── */

static void level_observer_cb(lv_observer_t *observer, lv_subject_t *subject)
{
    lv_obj_t *icon = (lv_obj_t *)lv_observer_get_user_data(observer);
    int32_t pct    = lv_subject_get_int(subject);

    /* Determine new state. */
    lv_color_t color;
    bool       visible;
    bool       blink;

    if(pct <= UI_BATTERY_CRITICAL_PERCENT) {
        color   = UI_BATTERY_CRITICAL_COLOR;
        visible = true;
        blink   = true;
    } else if(pct <= UI_BATTERY_MIN_PERCENT) {
        color   = UI_BATTERY_MIN_COLOR;
        visible = true;
        blink   = true;
    } else if(pct <= UI_BATTERY_LOW_PERCENT) {
        color   = UI_BATTERY_LOW_COLOR;
        visible = true;
        blink   = false;
    } else {
        color   = UI_FG_COLOR;
        visible = false;
        blink   = false;
    }

    /* Apply colour. */
    lv_obj_set_style_image_recolor(icon, color, 0);

    /* Apply visibility + animation. */
    if(!visible) {
        gui_anim_stop(icon);
        lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_remove_flag(icon, LV_OBJ_FLAG_HIDDEN);

    if(blink) {
        /* Only start a new blink cycle if one isn't already running. */
        if(!lv_anim_get(icon, NULL))
            gui_anim_blink(icon,
                           UI_ANIM_FADE_OUT_MS,
                           UI_ANIM_BLINK_DELAY_MS,
                           UI_ANIM_FADE_IN_MS);
    } else {
        gui_anim_stop(icon);  /* ensure fully opaque */
    }
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

void battery_element_create(lv_obj_t *parent, BatteryElement *out,
                             Battery *battery)
{
    out->charging_icon = icon_create(parent, UI_BATTERY_CHARGING_COLOR);
    out->level_icon    = icon_create(parent, UI_BATTERY_LOW_COLOR);

    /* Charging icon: hidden until observer fires. */
    lv_obj_add_flag(out->charging_icon, LV_OBJ_FLAG_HIDDEN);
    /* Level icon: hidden by default. */
    lv_obj_add_flag(out->level_icon, LV_OBJ_FLAG_HIDDEN);

    /* Observers are automatically removed when the icon widgets are deleted. */
    lv_subject_add_observer_obj(&battery->charging,
                                charging_observer_cb,
                                out->charging_icon,
                                out->charging_icon);

    lv_subject_add_observer_obj(&battery->percent,
                                level_observer_cb,
                                out->level_icon,
                                out->level_icon);
}
