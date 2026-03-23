#pragma once

#include "lvgl.h"

#include "io/battery.h"

/*
 * BatteryElement – notification-bar element that reacts to Battery events.
 *
 * charging_icon  visible while the device is charging.
 * level_icon     hidden above UI_BATTERY_LOW_PERCENT; visible (static) from
 *                UI_BATTERY_LOW_PERCENT down to UI_BATTERY_MIN_PERCENT; blinking
 *                below UI_BATTERY_MIN_PERCENT; colour shifts at
 *                UI_BATTERY_CRITICAL_PERCENT.
 *
 * Note: level_icon currently reuses battery_charge_icon.  Replace the
 * lv_image_set_src() call with a dedicated "low battery" asset when one
 * becomes available.
 */
typedef struct {
    lv_obj_t *charging_icon;
    lv_obj_t *level_icon;
} BatteryElement;

/* Create both icons as children of parent, subscribe to both battery
 * subjects.  Observers are auto-removed when the parent is deleted. */
void battery_element_create(lv_obj_t *parent, BatteryElement *out,
                             Battery *battery);
