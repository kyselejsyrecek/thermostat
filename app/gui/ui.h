#pragma once

/* Temperature range and default value in °C. */
#define UI_TEMP_MIN      5
#define UI_TEMP_MAX      30
#define UI_TEMP_DEFAULT  20

/* Decimal precision of the temperature display.
 * Arc and subject store the temperature as integer steps of UI_TEMP_PRECISION
 * tenths of a degree below the minimum (so actual tenths =
 * UI_TEMP_MIN*10 + step * UI_TEMP_PRECISION). */
#define UI_TEMP_PREC_HALF    5   /* 0.5 °C per step */
#define UI_TEMP_PREC_FIFTH   2   /* 0.2 °C per step */
#define UI_TEMP_PREC_TENTH   1   /* 0.1 °C per step */
#define UI_TEMP_PRECISION    UI_TEMP_PREC_HALF

/* Decimal separator character used in the temperature label (',' or '.'). */
#define UI_TEMP_DECIMAL_SEP  ','

/* 1: show decimal fraction in the label; 0: whole degrees only. */
#define UI_TEMP_SHOW_DECIMALS  1

/* Foreground (text, arc indicator, knob). */
#define UI_FG_COLOR  lv_color_hex(0xFFFFFF)

/* Gradient colours (RGB). */
#define UI_GRAD_COLOR_START  LV_COLOR_MAKE(0x6D, 0xF0, 0x96)   /* green – bottom-left  */
#define UI_GRAD_COLOR_END    LV_COLOR_MAKE(0x52, 0xBA, 0xBE)   /* teal  – top-right    */

/* Mid-point stop: 50 % blend of start/end, placed at 40 % of gradient length.
 * Shifts the transition balance toward the green end. */
#define UI_GRAD_COLOR_MID    LV_COLOR_MAKE(0x5F, 0xD5, 0xAA)
#define UI_GRAD_MID_FRAC     102u   /* 40 % × 255 ≈ 102 */

/* Gradient direction: start point (x, y) and end point (x, y) in
 * LV_GRAD_* units (LV_GRAD_LEFT=0%, LV_GRAD_CENTER=50%, LV_GRAD_TOP=0%…). */
#define UI_GRAD_START_X  LV_GRAD_LEFT
#define UI_GRAD_START_Y  LV_GRAD_CENTER
#define UI_GRAD_END_X    LV_GRAD_CENTER
#define UI_GRAD_END_Y    LV_GRAD_TOP

#define UI_FONT          lv_font_montserrat_32

void ui_init(void);
