#pragma once

#include "lvgl.h"

#include "sensor/battery.h"
#include "gui/element/battery.h"

/*
 * NotificationBar – container for status-bar elements.
 *
 * Elements:
 *   battery  – charging icon + low-battery icon, driven by Battery subjects.
 */
typedef struct {
    BatteryElement battery;
} NotificationBar;

/* Create the notification bar as a child of parent, subscribe its elements
 * to battery and write handles into *out.
 * The bar is centred horizontally in the arc gap at the bottom of the
 * circular viewport. */
void notification_bar_create(lv_obj_t *parent, NotificationBar *out,
                             Battery *battery);
