#include "lvgl.h"

#include "config.h"
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
 * This correctly models physical backlight dimming: each pixel channel is
 * scaled multiplicatively rather than shifted by a constant additive offset,
 * so relative contrast between colours is preserved at every brightness level.
 *
 * Brightness transitions are animated with lv_anim_t.
 * hal_display_init() must be called after lv_init() and after the SDL window
 * has been created (i.e. from hal_init() in main.c).
 */

static lv_obj_t *s_overlay           = NULL;
static uint8_t   s_target_brightness = 100;  /* 0–100 %, last requested target */
static uint8_t   s_current_grey      = 0xFF; /* grey level currently on-screen  */

/* Convert a brightness percentage (0–100) to a grey level (0–255).
 *   100 % -> 0xFF  (white  = full brightness in multiply mode)
 *     0 % -> 0x00  (black  = display off) */
static uint8_t brightness_to_grey(uint8_t pct)
{
    return (uint8_t)((uint32_t)pct * 0xFFu / 100u);
}

/* Animation exec callback: sets the overlay background to the animated grey level.
 * Also keeps s_current_grey up to date so interrupted animations start from
 * the correct visual position. */
static void anim_brightness_cb(void *var, int32_t v)
{
    uint8_t g = (uint8_t)v;
    lv_obj_set_style_bg_color((lv_obj_t *)var, lv_color_make(g, g, g), 0);
    s_current_grey = g;
}

static void fade_to(uint8_t target_pct, uint32_t duration_ms)
{
    uint8_t to_grey = brightness_to_grey(target_pct);

    s_target_brightness = target_pct;

    if(s_current_grey == to_grey) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_overlay);
    lv_anim_set_exec_cb(&a, anim_brightness_cb);
    lv_anim_set_values(&a, (int32_t)s_current_grey, (int32_t)to_grey);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_start(&a);
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

    s_target_brightness = 100;
    s_current_grey      = 0xFF;
}

void hal_display_on(void)
{
    fade_to(100, DISPLAY_FADE_IN_MS);
}

void hal_display_off(void)
{
    fade_to(0, DISPLAY_FADE_OUT_MS);
}

void hal_display_set_brightness(uint8_t percent)
{
    if(percent > 100) percent = 100;

    uint32_t duration_ms = (percent >= s_target_brightness)
                           ? DISPLAY_FADE_IN_MS
                           : DISPLAY_FADE_OUT_MS;

    fade_to(percent, duration_ms);
}

uint8_t hal_display_get_brightness(void)
{
    return s_target_brightness;
}
