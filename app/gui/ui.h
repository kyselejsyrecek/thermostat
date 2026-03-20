#pragma once

#include "lvgl.h"

#include "sensor/thermometer.h"
#include "screen/thermostat.h"
#include "screen/thermometer.h"

/*
 * UiHandle – top-level handle returned by ui_init().
 *
 * Gives app logic access to every screen's reactive subjects without coupling
 * it to LVGL widget internals.  Example:
 *
 *   UiHandle *ui = ui_init();
 *
 *   // Observe the user's temperature selection:
 *   lv_subject_add_observer(&ui->thermostat->picker.value, my_cb, NULL);
 *
 *   // Read the current sensor value:
 *   lv_subject_get_int(&ui->thermometer->value);
 */
typedef struct {
    ThermostatScreen   *thermostat;
    ThermometerScreen  *thermometer_screen;
    Thermometer        *thermometer;
} UiHandle;

/* Initialise the GUI.  Creates the circular viewport, mounts the active
 * screen and returns a pointer to the static UiHandle.
 * The return value may be ignored if the app does not need programmatic
 * control of the UI. */
UiHandle *ui_init(void);
