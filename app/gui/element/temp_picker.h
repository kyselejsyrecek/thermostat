#pragma once

#include "lvgl.h"

/*
 * TempPicker – reactive arc-based temperature selector.
 *
 * The `value` subject holds the current position as an arc-step index.
 * App logic can interact with it directly via the LVGL subject API:
 *
 *   // Observe changes (e.g. user turned the arc):
 *   lv_subject_add_observer(&picker->value, my_cb, NULL);
 *
 *   // Programmatically set a new step value:
 *   lv_subject_set_int(&picker->value, new_step);
 */
typedef struct {
    lv_obj_t    *arc;
    lv_subject_t value;    /* arc-step index; reactive via lv_subject API  */
    lv_obj_t    *int_lbl;  /* integer part label; full label in simple modes       */
    lv_obj_t    *frac_lbl; /* decimal part label (",X") (decimal mode only)       */
    lv_obj_t    *unit_lbl; /* unit label (" °C"), fixed position (decimal mode)   */
} TempPicker;

/* Create the picker widget as a child of parent and populate *out.
 * The subject is initialised to UI_TEMP_DEFAULT. */
void temp_picker_create(lv_obj_t *parent, TempPicker *out);
