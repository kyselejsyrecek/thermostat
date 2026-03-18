#include "lvgl.h"

#include "gui/battery_charge.h"
#include "config.h"

#include "notification_bar.h"

void notification_bar_create(lv_obj_t *parent)
{
    /* Battery / charging icon is centred horizontally and placed in the
     * arc gap at the bottom. The gap runs from the arc's lower edge
     * (~LV_VER_RES * 79 / 100) to the viewport bottom; the icon is
     * positioned at roughly 82 % of the viewport height. */
    lv_obj_t *battery = lv_image_create(parent);
    lv_image_set_src(battery, &battery_charge_icon);
    lv_obj_set_style_image_recolor(battery, UI_FG_COLOR, 0);
    lv_obj_set_style_image_recolor_opa(battery, LV_OPA_COVER, 0);
    lv_obj_align(battery, LV_ALIGN_TOP_MID, 0, LV_VER_RES * 14 / 17);
}
