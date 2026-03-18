#pragma once

#include "lvgl.h"

#include "gui/screen/thermostat.h"

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
 *   // Programmatically move the arc:
 *   lv_subject_set_int(&ui->thermostat->picker.value, new_step);
 */
typedef struct {
    ThermostatScreen *thermostat;
    /* future screens go here */
} UiHandle;

/* Initialise the GUI.  Creates the circular viewport, mounts all screens,
 * and returns a pointer to the static UiHandle.
 * The return value may be ignored if the app does not need programmatic
 * control of the UI. */
UiHandle *ui_init(void);
