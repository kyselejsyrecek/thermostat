#include "lvgl.h"

void hal_init(void);
void app_start(void);

int main(void)
{
    HAL_Init();

    lv_init();

    hal_init();

    app_start();
}
