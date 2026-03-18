#include "lvgl.h"

#include "gui/element/background.h"
#include "gui/element/notification_bar.h"
#include "gui/element/temp_picker.h"

#include "thermostat.h"

static ThermostatScreen s_screen;

ThermostatScreen *thermostat_screen_create(lv_obj_t *parent)
{
    s_screen.root = parent;
    background_set(parent);
    temp_picker_create(parent, &s_screen.picker);
    notification_bar_create(parent);
    return &s_screen;
}
