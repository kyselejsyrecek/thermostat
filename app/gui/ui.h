#pragma once

#include "lvgl.h"

#include "io/io.h"
#include "screen/set_temperature.h"
#include "screen/main.h"

/*
 * UiHandle – top-level handle returned by ui_init().
 *
 * Gives app logic access to every screen's reactive subjects without coupling
 * it to LVGL widget internals.  Example:
 *
 *   IO io = { .thermometer = thermometer_init(), .battery = battery_init() };
 *   UiHandle *ui = ui_init(&io);
 *
 *   // Observe the user's temperature selection:
 *   lv_subject_add_observer(&ui->set_temperature->picker.value, my_cb, NULL);
 */
typedef struct {
    SetTemperatureScreen *set_temperature;
    MainScreen           *main_screen;
} UiHandle;

/* Initialise the GUI.  Creates the circular viewport, mounts the active
 * screen and returns a pointer to the static UiHandle.
 * Sensors must be initialised by the caller and passed via `io`.
 * The return value may be ignored if the app does not need programmatic
 * control of the UI. */
UiHandle *ui_init(const IO *io);

/* Instantly reset the UI to the main screen without animation. */
void ui_sleep(void);
