#include "lvgl.h"

#include "io/display.h"

/*
 * HAL display implementation for the Linux platform (SDL simulator).
 *
 * Brightness is simulated via an LVGL overlay object placed on lv_layer_sys()
 * using LV_BLEND_MODE_MULTIPLY.  The overlay is a solid grey rectangle:
 *
 *   brightness 100 % -> overlay colour: white  (0xFF, 0xFF, 0xFF) -> result = src      (no change)
 *   brightness  50 % -> overlay colour: grey   (0x80, 0x80, 0x80) -> result = src / 2  (half brightness)
 *   brightness   0 % -> overlay colour: black  (0x00, 0x00, 0x00) -> result = 0        (screen off)
 *
 * Changes are applied instantly.
 *
 * hal_display_init() must be called after lv_init() and after the SDL window
 * has been created (i.e. from hal_init() in main.c).
 */

static lv_obj_t *s_overlay    = NULL;
static uint8_t   s_brightness = 100;  /* last value set via hal_display_set_brightness(), 0–100 % */

/* Convert a brightness percentage (0–100) to a grey level (0–255).
 *   100 % -> 0xFF  (white  = full brightness in multiply mode)
 *     0 % -> 0x00  (black  = display off) */
static uint8_t brightness_to_grey(uint8_t pct)
{
    return (uint8_t)((uint32_t)pct * 0xFFu / 100u);
}

/* ── HAL API ─────────────────────────────────────────────────────────────── */

void hal_display_init(void)
{
    s_overlay = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(s_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(s_overlay, 0, 0);
    lv_obj_set_style_bg_color(s_overlay, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(s_overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_blend_mode(s_overlay, LV_BLEND_MODE_MULTIPLY, 0);
    lv_obj_set_style_border_width(s_overlay, 0, 0);
    lv_obj_set_style_radius(s_overlay, 0, 0);
    lv_obj_set_style_pad_all(s_overlay, 0, 0);
    lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    s_brightness = 100;
}

void hal_display_on(void)
{
    /* No-op on Linux: the physical backlight is always enabled in the SDL
     * simulator.  Brightness is controlled entirely via the overlay. */
}

void hal_display_off(void)
{
    /* No-op on Linux: see hal_display_on(). */
}

void hal_display_set_brightness(uint8_t percent)
{
    if(percent > 100) percent = 100;
    s_brightness = percent;
    uint8_t g = brightness_to_grey(percent);
    lv_obj_set_style_bg_color(s_overlay, lv_color_make(g, g, g), 0);
}

uint8_t hal_display_get_brightness(void)
{
    return s_brightness;
}
