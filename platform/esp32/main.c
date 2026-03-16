#include "lvgl.h"

void hal_init(void);
void app_start(void);

int app_main(void)
{
    lv_init();

    hal_init();

    app_start();
}
