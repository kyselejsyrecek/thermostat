#include "lvgl.h"

#include "settings.h"

#include "gui/element/background.h"
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
    return &s_screen;
}

void set_temperature_nav_transition_cb(lv_obj_t *from, lv_obj_t *to,
                                       nav_gesture_t gesture)
{
    (void)to;
    /* Only act when leaving this screen. */
    if(from != s_screen.root) return;
    /* Double-tap = cancel: restore picker to the last saved value.
     * Programmatic reset (sleep) is treated the same way. */
    if(gesture == NAV_GESTURE_DOUBLE_TAP || gesture == NAV_GESTURE_RESET) {
        temp_picker_set_tenths(&s_screen.picker,
                               *(uint32_t *)settings_get(temperature));
        return;
    }

    settings_set(temperature,
                 (uint32_t)lv_subject_get_int(&s_screen.picker.value));
}
