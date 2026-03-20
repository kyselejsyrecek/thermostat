#pragma once

#include "lvgl.h"

/*
 * GUI animation helpers.
 *
 * All durations are in milliseconds.  The defaults come from config.h but
 * each call site can override them by passing explicit values.
 *
 * gui_anim_fade_in  – linearly ramps the object's opacity 0 → LV_OPA_COVER.
 * gui_anim_fade_out – linearly ramps the object's opacity LV_OPA_COVER → 0.
 * gui_anim_blink    – one full blink cycle: fade-out, pause, fade-in.
 *                     After the cycle the object is fully opaque again and
 *                     the animation repeats indefinitely.
 */

void gui_anim_fade_in (lv_obj_t *obj, uint32_t duration_ms);
void gui_anim_fade_out(lv_obj_t *obj, uint32_t duration_ms);
void gui_anim_blink   (lv_obj_t *obj,
                       uint32_t fade_out_ms,
                       uint32_t delay_ms,
                       uint32_t fade_in_ms);

/* Stop all running animations on obj and restore full opacity. */
void gui_anim_stop(lv_obj_t *obj);
