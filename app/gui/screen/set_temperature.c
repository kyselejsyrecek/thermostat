#include "lvgl.h"

#include "gui/element/background.h"
#include "gui/element/notification_bar.h"
#include "gui/element/temp_picker.h"

#include "gui/common.h"

#include "set_temperature.h"

static SetTemperatureScreen s_screen;

SetTemperatureScreen *set_temperature_screen_create(lv_obj_t *parent)
{
    lv_obj_t *root = gui_screen_root_create(parent);
    s_screen.root = root;
    background_set(root);
    temp_picker_create(root, &s_screen.picker);
    notification_bar_create(root);
    return &s_screen;
}
