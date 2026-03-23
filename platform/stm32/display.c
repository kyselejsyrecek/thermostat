#include <stdint.h>

#include "io/display.h"

/*
 * HAL display stub for the STM32 platform.
 * Replace the function bodies with real hardware control
 * (e.g. PWM on the backlight GPIO via TIM peripheral).
 */

static uint8_t s_brightness = 100;

void hal_display_init(void)
{
    /* Stub. */
}

void hal_display_on(void)
{
    s_brightness = 100;
    /* Stub. */
}

void hal_display_off(void)
{
    s_brightness = 0;
    /* Stub. */
}

void hal_display_set_brightness(uint8_t percent)
{
    if(percent > 100) percent = 100;
    s_brightness = percent;
    /* Stub. */
}

uint8_t hal_display_get_brightness(void)
{
    return s_brightness;
}
