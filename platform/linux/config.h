#pragma once

/*
 * Platform configuration for Linux (SDL simulator).
 *
 * Keeps all Linux-specific tunables in one place, mirroring the role of
 * app/config.h for the application layer.
 */

/* Duration of the display fade-in animation triggered by hal_display_on() or
 * hal_display_set_brightness() when increasing brightness (ms). */
#define DISPLAY_FADE_IN_MS   800u

/* Duration of the display fade-out animation triggered by hal_display_off() or
 * hal_display_set_brightness() when decreasing brightness (ms). */
#define DISPLAY_FADE_OUT_MS  800u
