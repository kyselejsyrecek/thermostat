#pragma once

#include "lvgl.h"

/* Create the notification bar as a child of parent.
 * The bar is centred horizontally and placed in the arc gap at the
 * bottom of the circular viewport. */
void notification_bar_create(lv_obj_t *parent);
