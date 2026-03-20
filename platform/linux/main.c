#include "lvgl.h"
#include "drivers/sdl/lv_sdl_window.h"
#include "drivers/sdl/lv_sdl_mouse.h"

void app_start(void);

void hal_init(void)
{
    lv_sdl_window_create(UI_DISPLAY_WIDTH  + 2 * UI_SIM_MARGIN_H,
                         UI_DISPLAY_HEIGHT + 2 * UI_SIM_MARGIN_V);
    lv_sdl_mouse_create();
}

int main(void)
{
    lv_init();

    hal_init();

    app_start();
}
