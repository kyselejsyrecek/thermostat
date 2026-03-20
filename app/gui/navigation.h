#pragma once

#include "lvgl.h"

/*
 * navigation – screen-transition manager for the multi-screen thermostat UI.
 *
 * Navigation is controlled entirely by config.h macros:
 *   UI_NAV_TO_SET_TEMPERATURE   – gesture mask leaving the main screen
 *   UI_NAV_FROM_SET_TEMPERATURE – gesture mask returning to the main screen
 *   UI_SWIPE_THRESHOLD_PERCENT  – minimum drag travel (% of display) to commit
 *   UI_SWIPE_ANIM_MS            – snap / completion animation duration
 *
 * Swipe gestures trigger a "follows-finger" animation: both screen roots move
 * with the touch and snap to the destination or back on release.
 * Point gestures (tap, double-tap, long-press) trigger an instant slide.
 *
 * Only gesture handlers whose gesture type is enabled in at least one
 * navigation mask are registered at compile time.
 *
 * Usage:
 *   navigation_init((lv_obj_t *[]){ main_screen->root,
 *                                   set_temperature_screen->root, NULL });
 */

/* Initialise navigation.  screens[] is a NULL-terminated array of root
 * objects, all children of the same circular viewport.  After this call,
 * screens[0] is at (0,0) and visible; every other screen is hidden
 * off-screen to the right.  Navigation cycles through screens in order,
 * wrapping around. */
void navigation_init(lv_obj_t *screens[]);
