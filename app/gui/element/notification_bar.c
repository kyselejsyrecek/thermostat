#include "lvgl.h"

#include "config.h"
#include "gui/element/battery.h"

#include "notification_bar.h"

void notification_bar_create(lv_obj_t *parent, NotificationBar *out,
                             Battery *battery)
{
    battery_element_create(parent, &out->battery, battery);
}
