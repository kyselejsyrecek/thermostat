#pragma once

#include <stdint.h>

/*
 * Display – platform-independent API for controlling the display.
 *
 * Thin inline wrappers that forward calls to the platform HAL functions
 * defined in platform/{platform}/display.c.
 *
 * Each platform must implement:
 *   hal_display_init()                  – initialise display hardware / simulation layer
 *   hal_display_on()                    – turn the display on (enable backlight)
 *   hal_display_off()                   – turn the display off (disable backlight)
 *   hal_display_set_brightness(pct)     – set brightness 0–100 % instantly
 *   hal_display_get_brightness()        – return the last brightness value set (0–100 %)
 *
 * All HAL functions apply changes instantly.  Gradual brightness transitions
 * (fade-in / fade-out) are the responsibility of the power-management module
 * (pm.c), which drives interpolation by calling hal_display_set_brightness()
 * repeatedly via lv_anim_t.
 *
 * On Linux (SDL), hal_display_on() and hal_display_off() are no-ops because
 * the backlight is simulated entirely through the brightness overlay.
 * On other platforms they control the physical backlight enable signal.
 */

/* Platform HAL functions – implemented in platform/{platform}/display.c. */

void    hal_display_init(void);
void    hal_display_on(void);
void    hal_display_off(void);
void    hal_display_set_brightness(uint8_t percent);
uint8_t hal_display_get_brightness(void);

/* Public API – thin forwarding wrappers. */

/* App-level initialisation – sets the display to DISPLAY_DEFAULT_BRIGHTNESS.
 * Must be called from app_start() after hal_display_init() has run. */
void display_init(void);

static inline void    display_on(void)                         { hal_display_on();                   }
static inline void    display_off(void)                        { hal_display_off();                  }
static inline void    display_set_brightness(uint8_t percent)  { hal_display_set_brightness(percent); }
static inline uint8_t display_get_brightness(void)             { return hal_display_get_brightness(); }
