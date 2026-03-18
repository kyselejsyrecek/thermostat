#include "lvgl.h"

#include "config.h"

#include "background.h"

void background_set(lv_obj_t *viewport)
{
    static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    static lv_grad_dsc_t grad;
    static const lv_color_t grad_colors[3] = {
        UI_GRAD_COLOR_START,
        UI_GRAD_COLOR_MID,
        UI_GRAD_COLOR_END,
    };
    static const uint8_t grad_fracs[3] = { 0, UI_GRAD_MID_FRAC, 255 };
    lv_grad_init_stops(&grad, grad_colors, NULL, grad_fracs, 3);
    lv_grad_linear_init(&grad,
                        UI_GRAD_START_X, UI_GRAD_START_Y,
                        UI_GRAD_END_X,   UI_GRAD_END_Y,
                        LV_GRAD_EXTEND_PAD);
    lv_style_set_bg_grad(&style_bg, &grad);
    lv_obj_add_style(viewport, &style_bg, 0);
}
