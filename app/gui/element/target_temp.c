#include "lvgl.h"

#include "config.h"
#include "settings.h"
#include "gui/common.h"

#include "target_temp.h"

/* ── Settings callback ───────────────────────────────────────────────────── */

static void settings_cb(const Settings *s, void *user_data)
{
    TargetTemp *tt = (TargetTemp *)user_data;
    gui_temp_label_update(&tt->labels, (int32_t)s->temperature);
}

/* ── Public API ──────────────────────────────────────────────────────────── */

void target_temp_create(lv_obj_t *parent, TargetTemp *out)
{
    gui_temp_labels_create(parent, &out->labels, UI_FG_COLOR, &UI_FONT_MAIN_TARGET_TEMP);

    /* Show the current value immediately. */
    gui_temp_label_update(&out->labels,
                          (int32_t)*(uint32_t *)settings_get(temperature));

    /* Register for automatic updates on every settings change. */
    settings_register_cb(settings_cb, out);
}
