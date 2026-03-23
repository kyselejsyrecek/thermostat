#include "lvgl.h"
#include <unistd.h>

// TODO These are LVGL demos. Remove.
#include "examples/lv_examples.h"
#include "demos/lv_demos.h"

#include "io/io.h"
#include "io/rtc.h"
#include "gui/ui.h"

int app_start();

#ifdef LVGL_LIVE_PREVIEW
void lvgl_live_preview_init(void)
{
    app_start();
}
#endif

int app_start()
{
    IO io = {
        .thermometer = thermometer_init(),
        .battery     = battery_init(),
    };
    rtc_init();

    // The real application UI.
    ui_init(&io);

#ifndef LVGL_LIVE_PREVIEW
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    while(lv_display_get_default() != NULL) {
        uint32_t sleep_ms = lv_timer_handler();
        if(sleep_ms == LV_NO_TIMER_READY) {
            sleep_ms = LV_DEF_REFR_PERIOD;
        }
        usleep(sleep_ms * 1000);
    }
#endif

    return 0;
}
