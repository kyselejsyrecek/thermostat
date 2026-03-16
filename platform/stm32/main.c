#include "lvgl.h"

/* Round display: 360x360 px */
#define DISPLAY_WIDTH  360
#define DISPLAY_HEIGHT 360

void hal_init(void);
void app_start(void);

int main(void)
{
    HAL_Init();

    lv_init();

    hal_init();

    app_start();
}
