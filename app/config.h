#pragma once

#include "lvgl.h"

/* ── Temperature range ─────────────────────────────────────────────────────── */

#define UI_TEMP_MIN      15   /* °C – minimum selectable temperature          */
#define UI_TEMP_MAX      30   /* °C – maximum selectable temperature          */
#define UI_TEMP_DEFAULT  20   /* °C – initial setpoint                        */

/* Arc step precision helpers – use these to configure the zones below. */
#define UI_TEMP_PREC_WHOLE   10   /* 1.0 °C per step */
#define UI_TEMP_PREC_HALF     5   /* 0.5 °C per step */
#define UI_TEMP_PREC_FIFTH    2   /* 0.2 °C per step */
#define UI_TEMP_PREC_TENTH    1   /* 0.1 °C per step */

/* Fine-precision zone: higher arc resolution between these temperatures. */
#define UI_TEMP_FINE_MIN     18   /* °C – lower bound of fine zone            */
#define UI_TEMP_FINE_MAX     27   /* °C – upper bound of fine zone            */
#define UI_TEMP_PREC_NORMAL  UI_TEMP_PREC_WHOLE   /* step size outside zone  */
#define UI_TEMP_PREC_FINE    UI_TEMP_PREC_HALF    /* step size inside zone   */

/* Decimal separator character shown in the temperature label (',' or '.'). */
#define UI_TEMP_DECIMAL_SEP  ','

/* 1: show one decimal digit in the label; 0: whole degrees only. */
#define UI_TEMP_SHOW_DECIMALS  1

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

/* ── Typography ────────────────────────────────────────────────────────────── */

#define UI_FONT  lv_font_montserrat_32
