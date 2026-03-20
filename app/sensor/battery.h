#pragma once

#include "lvgl.h"

/*
 * Battery – stub driver for a battery sensor.
 *
 * Exposes two reactive subjects:
 *   percent  – current battery level in whole percent (0–100).
 *   charging – 1 if the device is currently charging, 0 otherwise.
 *
 * Two independent LVGL timers drive updates:
 *   • BATTERY_PERCENT_INTERVAL_MS  – reads the battery level.
 *   • BATTERY_CHARGING_INTERVAL_MS – polls the charging state.
 *
 * Both subjects are updated only when their value changes, so observers are
 * not notified unnecessarily.
 *
 * Usage:
 *   Battery *b = battery_init();
 *   lv_subject_add_observer(&b->charging, my_cb, NULL);
 */
typedef struct {
    lv_subject_t percent;   /* battery level (0‒100 %) */
    lv_subject_t charging;  /* 1 = charging, 0 = on battery */
} Battery;

/* Initialise the battery sensor, seed both subjects with the first readings
 * and start the periodic refresh timers.  Returns a pointer to the static
 * instance.  Must be called after lv_init(). */
Battery *battery_init(void);
