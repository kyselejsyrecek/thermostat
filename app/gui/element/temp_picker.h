#pragma once

#include "lvgl.h"

#include "gui/common.h"

/*
 * TempPicker – reactive arc-based temperature selector.
 *
 * The `value` subject holds the current set-point in **tenths of a degree
 * Celsius** (e.g. 205 = 20.5 °C).  App logic observes or reads this directly:
 *
 *   // Observe changes (e.g. user turned the arc):
 *   lv_subject_add_observer(&picker->value, my_cb, NULL);
 *
 *   // Read the current set-point (tenths):
 *   int32_t t = lv_subject_get_int(&picker->value);
 *
 * The `arc_step` subject is the internal arc index.  It is public only so its
 * lifetime is tied to the TempPicker instance; do not write to it directly.
 */
typedef struct {
    lv_obj_t     *arc;
    lv_subject_t  value;     /* set-point in tenths of °C (public, read-only) */
    lv_subject_t  arc_step;  /* internal arc step index, bound to the arc    */
    TempLabels    labels;    /* temperature display labels (see gui/common.h) */
} TempPicker;

/* Create the picker widget as a child of parent and populate *out.
 * The subjects are initialised to UI_TEMP_DEFAULT. */
void temp_picker_create(lv_obj_t *parent, TempPicker *out);

/* Programmatically set the picker to a temperature in tenths of °C.
 * Updates arc_step (and therefore the arc widget and value subject). */
void temp_picker_set_tenths(TempPicker *picker, int32_t tenths);
