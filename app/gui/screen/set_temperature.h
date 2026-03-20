#pragma once

#include "lvgl.h"

#include "gui/element/temp_picker.h"

/*
 * SetTemperatureScreen – layout handle for the temperature set-point screen.
 *
 * Exposes all reactive subjects so that app logic can observe or drive the UI
 * without touching LVGL widgets directly:
 *
 *   // Observe user changes to the setpoint:
 *   lv_subject_add_observer(&screen->picker.value, my_cb, NULL);
 *
 *   // Programmatically move the arc:
 *   lv_subject_set_int(&screen->picker.value, new_step);
 */
typedef struct {
    lv_obj_t  *root;
    TempPicker picker;
} SetTemperatureScreen;

/* Assemble the set-temperature screen inside parent and return a pointer to
 * the static SetTemperatureScreen instance owned by this translation unit. */
SetTemperatureScreen *set_temperature_screen_create(lv_obj_t *parent);
