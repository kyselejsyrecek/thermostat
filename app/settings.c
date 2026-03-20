#include "config.h"

#include "settings.h"

/* ── Callbacks ──────────────────────────────────────────────────────────────*/

typedef struct { settings_cb_t fn; void *user_data; } SettingsCbEntry;
static SettingsCbEntry s_cbs[SETTINGS_MAX_CBS];
static int             s_cb_count = 0;

void settings_register_cb(settings_cb_t cb, void *user_data)
{
    if(s_cb_count < SETTINGS_MAX_CBS) {
        s_cbs[s_cb_count].fn        = cb;
        s_cbs[s_cb_count].user_data = user_data;
        s_cb_count++;
    }
}

static Settings s_settings = {
    .temperature = UI_TEMP_DEFAULT,   /* already in tenths of a degree */
};

/* ── Persistence stub ───────────────────────────────────────────────────────
 *
 * STUB – replace with an actual write to non-volatile storage
 * (e.g. flash, EEPROM, or a file) once the target hardware is available.
 * ────────────────────────────────────────────────────────────────────────── */
static void settings_persist(size_t offset, uint32_t value)
{
    /* nothing yet */
    (void)offset;
    (void)value;
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

void *__settings_get(size_t offset)
{
    return (void *)((uint8_t *)&s_settings + offset);
}

void __settings_set(size_t offset, uint32_t value)
{
    *(uint32_t *)((uint8_t *)&s_settings + offset) = value;
    settings_persist(offset, value);
    for(int i = 0; i < s_cb_count; i++)
        s_cbs[i].fn(&s_settings, s_cbs[i].user_data);
}
