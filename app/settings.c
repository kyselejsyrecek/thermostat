#include "config.h"

#include "settings.h"

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
}
