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
 *   hal_display_on()                    – turn the display on (fade in on Linux/SDL)
 *   hal_display_off()                   – turn the display off (fade out on Linux/SDL)
 *   hal_display_set_brightness(pct)     – set brightness 0–100 %
 *   hal_display_get_brightness()        – return the current brightness target (0–100 %)
 *
 * On Linux (SDL) brightness changes are animated (fade in / fade out) using a
 * MULTIPLY-blended overlay, which correctly scales pixel colours multiplicatively.
 * On other platforms the functions are stubs.
 */

/* Platform HAL functions – implemented in platform/{platform}/display.c. */

void    hal_display_init(void);
void    hal_display_on(void);
void    hal_display_off(void);
void    hal_display_set_brightness(uint8_t percent);
uint8_t hal_display_get_brightness(void);

/* Public API – thin forwarding wrappers. */

static inline void    display_on(void)                         { hal_display_on();                   }
static inline void    display_off(void)                        { hal_display_off();                  }
static inline void    display_set_brightness(uint8_t percent)  { hal_display_set_brightness(percent); }
static inline uint8_t display_get_brightness(void)             { return hal_display_get_brightness(); }
