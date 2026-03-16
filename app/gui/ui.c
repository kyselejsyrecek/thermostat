#include "lvgl.h"
#include "ui.h"

void ui_init(void)
{
    lv_obj_t * screen = lv_obj_create(NULL);

    lv_obj_t * btn = lv_btn_create(screen);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_center(btn);

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "LVGL running");
    lv_obj_center(label);

    lv_scr_load(screen);
}
