#include "lvgl.h"
#include "drivers/sdl/lv_sdl_window.h"
#include "drivers/sdl/lv_sdl_mouse.h"

void app_start(void);

void hal_init(void)
{
    lv_sdl_window_create(800, 480);
    lv_sdl_mouse_create();
}

int main(void)
{
    lv_init();

    hal_init();

    app_start();
}
