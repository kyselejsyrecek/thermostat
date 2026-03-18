#include "lvgl.h"

#include "gui/screen/thermostat.h"

#include "ui.h"

static lv_obj_t *bouding_circle_create(void)
{
    lv_obj_t *circle;

    /* Screen background: black outside the circular display area. */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);

    /* Circular container – clips all children to a circle.
     * The gradient is applied by the individual screen; the screen corners
     * (outside the circle) stay black. */
    circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(circle, true, 0);
    lv_obj_set_style_pad_all(circle, 0, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE);

    return circle;
}

static UiHandle s_handle;

UiHandle *ui_init(void)
{
    lv_obj_t *viewport = bouding_circle_create();
    s_handle.thermostat = thermostat_screen_create(viewport);
    return &s_handle;
}
