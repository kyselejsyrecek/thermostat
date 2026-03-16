#include "lvgl.h"
#include <unistd.h>

// TODO These are LVGL demos. Remove.
#include "examples/lv_examples.h"
#include "demos/lv_demos.h"

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
#if 0
    /* Run the default demo */
    /* To try a different demo or example, replace this with one of: */
    /* - lv_demo_benchmark(); */
    /* - lv_demo_stress(); */
    /* - lv_example_label_1(); */
    /* - etc. */
    lv_demo_music();
#endif

    // The real application UI.
    ui_init();

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
