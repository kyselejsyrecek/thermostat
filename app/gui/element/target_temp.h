#pragma once

#include "lvgl.h"

#include "gui/common.h"

/*
 * TargetTemp – read-only label group showing the target temperature from
 * Settings.
 *
 * Registers a settings callback internally and self-updates whenever
 * settings_set(temperature, …) is called.  The caller does not need to
 * interact with the element after creation.
 */
typedef struct {
    TempLabels labels;
} TargetTemp;

/* Create the target-temperature label group as children of parent and fill
 * *out.  The initial value is read from Settings; subsequent updates are
 * automatic. */
void target_temp_create(lv_obj_t *parent, TargetTemp *out);
