#pragma once

#include "lvgl.h"

/*
 * TempLabels – shared label group for displaying a temperature value.
 *
 * Used by any widget or screen that renders temperature text so that the
 * split-label layout is consistent everywhere.
 */
typedef struct {
    lv_obj_t *int_lbl;   /* integer part; full label in center-exact mode   */
    lv_obj_t *frac_lbl;  /* decimal part (",X") – NULL if not applicable    */
    lv_obj_t *unit_lbl;  /* unit (" °C"), fixed position – NULL if N/A      */
} TempLabels;

/* Apply common text-label styling: width, alignment, font (UI_FONT) and the
 * given foreground colour. */
void gui_label_style(lv_obj_t *lbl, int32_t w, lv_text_align_t align, lv_color_t color);

/* Return the maximum rendered pixel width among the count strings in the
 * array, measured with UI_FONT. */
int32_t gui_measure_max_width(const char *strings[], int count);

/* Return the maximum pixel width of the integer portion of a temperature
 * label across the range [min_celsius .. max_celsius], formatted as "%d". */
int32_t gui_max_int_part_width(int min_celsius, int max_celsius);

/* Create the split temperature label group as children of parent and fill
 * *out.  Layout is controlled entirely by config.h (UI_TEMP_CENTER_EXACT,
 * UI_TEMP_ENABLE_DECIMALS, UI_TEMP_FIXED_UNIT).  color is applied to every
 * label. */
void gui_temp_labels_create(lv_obj_t *parent, TempLabels *out, lv_color_t color);

/* Update the label group to show the given temperature (tenths of °C, e.g.
 * 215 = 21.5 °C).  Must be called after gui_temp_labels_create. */
void gui_temp_label_update(TempLabels *labels, int32_t tenths);

/* Create a full-screen root object inside parent with all the common
 * screen-root properties set (display size, no scroll, no border/padding).
 * Used by every screen to avoid boilerplate duplication. */
lv_obj_t *gui_screen_root_create(lv_obj_t *parent);
