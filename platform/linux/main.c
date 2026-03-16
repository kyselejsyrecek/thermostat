#include "lvgl.h"
#include "drivers/sdl/lv_sdl_window.h"
#include "drivers/sdl/lv_sdl_mouse.h"

#define DISPLAY_WIDTH  360
#define DISPLAY_HEIGHT 360

void app_start(void);

void hal_init(void)
{
    lv_sdl_window_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_sdl_mouse_create();
}

int main(void)
{
    lv_init();

    hal_init();

    app_start();
}
