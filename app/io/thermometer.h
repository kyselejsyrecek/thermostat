#pragma once

#include "lvgl.h"

/*
 * Thermometer – stub driver for a room thermometer sensor.
 *
 * Maintains a reactive subject (value) holding the current temperature in
 * tenths of a degree Celsius (e.g. 215 = 21.5 °C).
 *
 * An LVGL timer fires every THERMOMETER_INTERVAL_MS milliseconds, reads the
 * hardware and updates the subject only when the value changes, so observers
 * are not notified unnecessarily.
 *
 * Usage:
 *   Thermometer *t = thermometer_init();
 *   lv_subject_add_observer(&t->value, my_cb, NULL);
 */
typedef struct {
    lv_subject_t value; /* temperature in tenths of a degree (e.g. 215 = 21.5 °C) */
} Thermometer;

/* Initialise the thermometer, seed the subject with the first reading and
 * start the periodic refresh timer.  Returns a pointer to the static
 * instance.  Must be called after lv_init(). */
Thermometer *thermometer_init(void);
