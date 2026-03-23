#include <stdio.h>
#include "SDL2/SDL.h"
#include "lvgl.h"
#include "drivers/sdl/lv_sdl_window.h"
#include "drivers/sdl/lv_sdl_mouse.h"

#include "io/display.h"

void app_start(void);

/* Clamp mouse coordinates to non-negative values before SDL delivers events
 * to LVGL.  When the user drags a swipe fast enough to leave the top or left
 * window edge, SDL reports negative coordinates.  LVGL's indev_pointer_proc
 * warns and then clamps internally, but we suppress the warning at the source
 * by clamping the raw SDL event data here instead. */
static int sdl_clamp_coords_watch(void *userdata, SDL_Event *event)
{
    (void)userdata;
    int w = 0, h = 0;
    switch(event->type) {
        case SDL_MOUSEMOTION: {
            SDL_Window *win = SDL_GetWindowFromID(event->motion.windowID);
            if(win) SDL_GetWindowSize(win, &w, &h);
            if(event->motion.x < 0)        event->motion.x = 0;
            if(w > 0 && event->motion.x >= w) event->motion.x = w - 1;
            if(event->motion.y < 0)        event->motion.y = 0;
            if(h > 0 && event->motion.y >= h) event->motion.y = h - 1;
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            SDL_Window *win = SDL_GetWindowFromID(event->button.windowID);
            if(win) SDL_GetWindowSize(win, &w, &h);
            if(event->button.x < 0)        event->button.x = 0;
            if(w > 0 && event->button.x >= w) event->button.x = w - 1;
            if(event->button.y < 0)        event->button.y = 0;
            if(h > 0 && event->button.y >= h) event->button.y = h - 1;
            break;
        }
        default:
            break;
    }
    return 1;
}

void hal_init(void)
{
    SDL_AddEventWatch(sdl_clamp_coords_watch, NULL);

    lv_sdl_window_create(UI_DISPLAY_WIDTH  + 2 * UI_SIM_MARGIN_H,
                         UI_DISPLAY_HEIGHT + 2 * UI_SIM_MARGIN_V);
    lv_sdl_mouse_create();

    /* Display overlay must be created after the SDL window / LVGL display
     * exists, so that lv_layer_sys() returns a valid object. */
    hal_display_init();
}

int main(void)
{
    lv_init();

    hal_init();

    app_start();
}
