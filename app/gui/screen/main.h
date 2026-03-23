#pragma once

#include "lvgl.h"

#include "gui/common.h"
#include "gui/element/notification_bar.h"
#include "gui/element/target_temp.h"
#include "io/thermometer.h"
#include "io/battery.h"

/*
 * MainScreen – read-only display of the current room temperature.
 *
 * Subscribes to thermometer->value and keeps the label group up-to-date for
 * the lifetime of the screen.  Layout and typography are controlled by
 * config.h, identical to TempPicker, so the user sees the same formatting
 * on both screens.
 */
typedef struct {
    lv_obj_t       *root;
    TempLabels      labels;
    TargetTemp      target_temp;
    NotificationBar notif_bar;
} MainScreen;

/* Assemble the main screen inside parent, subscribe to
 * thermometer->value and return a pointer to the static instance. */
MainScreen *main_screen_create(lv_obj_t *parent, Thermometer *thermometer,
                               Battery *battery);
