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

/* Register an object that will have LV_OBJ_FLAG_CLICKABLE removed for the
 * entire duration of a swipe drag and the subsequent snap animation, and
 * restored when the navigation returns to idle.  Must be called after
 * navigation_init.  Typical use: pass the arc widget of the temp picker so
 * it cannot be adjusted while the user is swiping between screens. */
void navigation_add_drag_guard(lv_obj_t *obj);

/* Assign a z-order layer to a registered screen root.  The screen with the
 * higher layer value is the "cover": during transitions it slides over or
 * away from the other screen, which stays fixed at its final position (0,0).
 * Screens with the same layer fall back to the current-screen-moves behaviour.
 * Call after navigation_init(); default layer for all screens is 0. */
void navigation_set_layer(lv_obj_t *screen, int8_t layer);

/* Gesture that triggered the last completed transition.
 * NAV_GESTURE_NONE is the default state (no transition in progress or the
 * animation has fully settled). */
typedef enum {
    NAV_GESTURE_NONE,         /* idle / no transition                        */
    NAV_GESTURE_SWIPE_LEFT,   /* committed drag toward left edge             */
    NAV_GESTURE_SWIPE_RIGHT,  /* committed drag toward right edge            */
    NAV_GESTURE_SWIPE_UP,     /* committed drag toward top edge              */
    NAV_GESTURE_SWIPE_DOWN,   /* committed drag toward bottom edge           */
    NAV_GESTURE_TAP,          /* single tap                                  */
    NAV_GESTURE_DOUBLE_TAP,   /* two quick taps (often means "cancel")      */
    NAV_GESTURE_LONG_PRESS,   /* press held for LVGL long-press time        */
} nav_gesture_t;

/* Transition-complete callback type.
 *
 *   from    – root of the screen that was active before the transition.
 *   to      – root of the screen that is now active.
 *   gesture – gesture that triggered the transition.
 *
 * Only called when the transition actually changes the active screen (i.e.
 * not when a drag is cancelled and the screen snaps back).  After the
 * callback returns, last_gesture is reset to NAV_GESTURE_NONE. */
typedef void (*nav_transition_cb_t)(lv_obj_t *from, lv_obj_t *to,
                                    nav_gesture_t gesture);

/* Register a single global callback invoked at the end of every successful
 * screen transition.  Pass NULL to unregister.  Call after navigation_init(). */
void navigation_set_transition_cb(nav_transition_cb_t cb);
