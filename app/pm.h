#pragma once

/*
 * Power management.
 *
 * Monitors user inactivity via LVGL's built-in inactive-time counter and
 * dims / turns off the display automatically.
 *
 * Call pm_init() from app_start() after display_init().
 */

void pm_init(void);
