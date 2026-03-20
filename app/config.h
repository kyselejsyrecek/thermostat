#pragma once

#include "lvgl.h"

/* ── Temperature range ─────────────────────────────────────────────────────── */

#define UI_TEMP_MIN      150  /* tenths of °C – minimum selectable temperature (15.0 °C) */
#define UI_TEMP_MAX      300  /* tenths of °C – maximum selectable temperature (30.0 °C) */
#define UI_TEMP_DEFAULT  200  /* tenths of °C – initial setpoint              (20.0 °C) */

/* Decimal separator character shown in the temperature label (',' or '.'). */
#define UI_TEMP_DECIMAL_SEP  ','

/* 1: show one decimal digit in the label; 0: whole degrees only. */
#define UI_TEMP_ENABLE_DECIMALS  1

/* Arc step precision helpers – use these to configure the zones below. */
#define UI_TEMP_PREC_WHOLE   10   /* 1.0 °C per step */

#if UI_TEMP_ENABLE_DECIMALS
#  define UI_TEMP_PREC_HALF    5   /* 0.5 °C per step */
#  define UI_TEMP_PREC_FIFTH   2   /* 0.2 °C per step */
#  define UI_TEMP_PREC_TENTH   1   /* 0.1 °C per step */

/* Fine-precision zone: higher arc resolution between these temperatures. */
#  define UI_TEMP_FINE_MIN    180   /* tenths of °C – lower bound of fine zone (18.0 °C) */
#  define UI_TEMP_FINE_MAX    270   /* tenths of °C – upper bound of fine zone (27.0 °C) */
#  define UI_TEMP_PREC_NORMAL  UI_TEMP_PREC_WHOLE  /* step size outside zone */
#  define UI_TEMP_PREC_FINE    UI_TEMP_PREC_HALF   /* step size inside zone  */
#endif

/* ── Typography ────────────────────────────────────────────────────────────── */

extern lv_font_t lv_font_montserrat_60;

#define UI_FONT                    lv_font_montserrat_32
#define UI_FONT_MAIN_TEMP          lv_font_montserrat_60
#define UI_FONT_MAIN_TARGET_TEMP   lv_font_montserrat_24

/* Gap between the current-temperature row and the target-temperature row (px). */
#define UI_MAIN_TEMP_GROUP_GAP  10

/* Maximum number of settings change callbacks that can be registered. */
#define SETTINGS_MAX_CBS  8

/* 1 if UI_FONT is a monospace (fixed-pitch) font.
 * Automatically enables UI_TEMP_CENTER_EXACT so the full label is simply
 * centred – justified, because every digit is the same width anyway. */
#define UI_TEMP_MONO_FONT  0

/* 1: render the temperature as a single centred label (the whole string,
 *    e.g. "20,5 °C", is always horizontally centred as one unit).
 * 0: use the split-label trick (decimals) or right-aligned label (integers)
 *    so that the decimal separator / unit stays at a fixed position.
 *
 * Set to 1 explicitly to force centred rendering with a proportional font,
 * or leave at 0 to let it be derived automatically from UI_TEMP_MONO_FONT. */
//#define UI_TEMP_CENTER_EXACT 1
#if !UI_TEMP_CENTER_EXACT && UI_TEMP_MIN >= 100 && UI_TEMP_MAX < 1000
#  define UI_TEMP_CENTER_EXACT  UI_TEMP_MONO_FONT
#endif

/* 1: when UI_TEMP_ENABLE_DECIMALS is active, render the unit (" °C") as a
 *    separate third label so that it stays at a fixed rightmost position
 *    regardless of the number of integer digits.  The resulting layout is:
 *      [ integer | ,X | °C ]
 *    with all three blocks horizontally centred together.
 * 0: unit is appended to the decimal label (",X °C"), which is the simpler
 *    two-label layout. Has no effect when UI_TEMP_ENABLE_DECIMALS is 0. */
#if UI_TEMP_ENABLE_DECIMALS
#  define UI_TEMP_FIXED_UNIT  1
#endif

/* ── Colours ───────────────────────────────────────────────────────────────── */

/* Foreground: text, arc indicator, knob, icons. */
#define UI_FG_COLOR  lv_color_hex(0xFFFFFF)

/* Gradient colours (RGB). */
#define UI_GRAD_COLOR_START  LV_COLOR_MAKE(0x6D, 0xF0, 0x96)   /* green – bottom-left  */
#define UI_GRAD_COLOR_END    LV_COLOR_MAKE(0x52, 0xBA, 0xBE)   /* teal  – top-right    */

/* Mid-point stop: ~50 % blend of start/end placed at 40 % of the gradient.
 * Shifts the transition balance toward the green end. */
#define UI_GRAD_COLOR_MID    LV_COLOR_MAKE(0x5F, 0xD5, 0xAA)
#define UI_GRAD_MID_FRAC     102u   /* 40 % × 255 ≈ 102 */

/* Gradient direction – start/end in LV_GRAD_* units.
 *
 * LVGL SW renderer uses int32 arithmetic with a <<16 fixed-point shift.
 * For a 360×360 display, LV_GRAD_BOTTOM (100 %) overflows int32.
 * LV_GRAD_CENTER (50 % = 180 px) stays within range and still produces a
 * smooth 45° diagonal thanks to EXTEND_PAD clamping to the endpoint colours. */
#define UI_GRAD_START_X  LV_GRAD_LEFT
#define UI_GRAD_START_Y  LV_GRAD_CENTER
#define UI_GRAD_END_X    LV_GRAD_CENTER
#define UI_GRAD_END_Y    LV_GRAD_TOP

/* ── Thermometer sensor ────────────────────────────────────────────────────── */

/* How often the thermometer sensor is polled (ms).  The reactive subject is
 * updated only when the reading changes, so observers are not notified
 * unnecessarily. */
#define THERMOMETER_INTERVAL_MS  15000u

/* ── Simulation bezel ──────────────────────────────────────────────────────── */

/* Width and colour of the ring drawn around the circular display boundary
 * in simulation.  Keep UI_SIM_BORDER_WIDTH <= UI_SIM_MARGIN_H / UI_SIM_MARGIN_V
 * (defined in lv_conf.h) so the ring is not clipped at the window edge. */
#ifdef UI_SIMULATION
#  define UI_SIM_BORDER_WIDTH  2
#  define UI_SIM_BORDER_COLOR  lv_color_hex(0xFFFFFF)
#endif

/* ── Screen navigation ─────────────────────────────────────────────────────── */

/* Individual gesture flags – combine with | to form a navigation mask. */
#define UI_NAV_TAP          (1u << 0)   /* Short tap anywhere on the screen    */
#define UI_NAV_DOUBLE_TAP   (1u << 1)   /* Two quick taps                      */
#define UI_NAV_LONG_PRESS   (1u << 2)   /* Press held for LVGL long-press time */
#define UI_NAV_SWIPE_LEFT   (1u << 3)   /* Horizontal swipe toward left edge   */
#define UI_NAV_SWIPE_RIGHT  (1u << 4)   /* Horizontal swipe toward right edge  */
#define UI_NAV_SWIPE_UP     (1u << 5)   /* Vertical swipe toward top edge      */
#define UI_NAV_SWIPE_DOWN   (1u << 6)   /* Vertical swipe toward bottom edge   */

/* Gestures that navigate from the main screen to the set-temperature screen. */
#define UI_NAV_TO_SET_TEMPERATURE    (UI_NAV_SWIPE_LEFT | UI_NAV_SWIPE_RIGHT)

/* Gestures that navigate back from the set-temperature screen to the main screen.
 * NOTE: swipe gestures are blocked when the touch starts on the arc picker. */
#define UI_NAV_FROM_SET_TEMPERATURE  (UI_NAV_DOUBLE_TAP | UI_NAV_SWIPE_LEFT | UI_NAV_SWIPE_RIGHT)

/* A drag must travel at least this many percent of the display dimension
 * before it is committed on release.  Below this the screen snaps back. */
#define UI_SWIPE_THRESHOLD_PERCENT  30

/* Minimum finger travel in pixels along either axis before the swipe axis
 * is locked.  Below this threshold the touch is treated as a tap. */
#define UI_SWIPE_AXIS_LOCK_PX  8

/* Duration of the snap / completion animation in milliseconds. */
#define UI_SWIPE_ANIM_MS        250

/* ── Temp-picker interaction ───────────────────────────────────────────────── */

/* 1: the temperature arc only responds to touches that land on the knob.
 *    Touching the scale track elsewhere is ignored, so an accidental swipe
 *    across the screen cannot change the set-point.
 * 0: standard LVGL arc behaviour – touching anywhere on the track jumps
 *    the value to that position immediately. */
#define UI_TEMP_PICKER_RESPONSIVITY_KNOB_ONLY  1

/* Multiplier for the knob touch hit-radius (integer × 10).
 * 10 = 1.0 (tight, visual knob only), 15 = 1.5 (default, comfortable fingertip). */
#define UI_TEMP_PICKER_KNOB_HIT_SCALE  15

/* ── Battery sensor ────────────────────────────────────────────────────────── */

/* How often the battery percentage is read (ms). */
#define BATTERY_PERCENT_INTERVAL_MS   60000u

/* How often the charging state is polled (ms). */
#define BATTERY_CHARGING_INTERVAL_MS   5000u

/* Default stub values. */
#define BATTERY_DEFAULT_PERCENT   80
#define BATTERY_DEFAULT_CHARGING   1

/* ── Battery UI thresholds & colours ───────────────────────────────────────── */

/* Battery level at which the battery icon becomes visible (%). */
#define UI_BATTERY_LOW_PERCENT      25

/* Battery level at which the icon starts blinking (%). */
#define UI_BATTERY_MIN_PERCENT      15

/* Battery level at which the icon is considered critical (%). */
#define UI_BATTERY_CRITICAL_PERCENT  5

/* Icon colours for each state (all white by default). */
#define UI_BATTERY_CHARGING_COLOR   UI_FG_COLOR
#define UI_BATTERY_LOW_COLOR        UI_FG_COLOR
#define UI_BATTERY_MIN_COLOR        UI_FG_COLOR
#define UI_BATTERY_CRITICAL_COLOR   UI_FG_COLOR

/* ── GUI animation timings ─────────────────────────────────────────────────── */

/* Duration of fade-in and fade-out animations (ms). */
#define UI_ANIM_FADE_IN_MS   1000u
#define UI_ANIM_FADE_OUT_MS  1000u

/* Pause between fade-out and next fade-in in the blink animation (ms). */
#define UI_ANIM_BLINK_DELAY_MS  1500u
