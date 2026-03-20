#pragma once

#include <stddef.h>   /* offsetof */
#include <stdint.h>

/* Settings type is defined here so callers can name fields in macros.
 * The actual instance is private to settings.c; it is never exported.
 *
 * All temperature values are stored in tenths of °C (200 = 20.0 °C). */
typedef struct {
    uint32_t temperature;
} Settings;

/* Internal helpers – do not call directly. */
void    *__settings_get(size_t offset);
void     __settings_set(size_t offset, uint32_t value);

/* Get a pointer to a settings field by name.  Cast to the field type:
 *   uint32_t t = *(uint32_t *)settings_get(temperature); */
#define settings_get(attr)         __settings_get(offsetof(Settings, attr))

/* Write a value to a settings field and trigger persistence:
 *   settings_set(temperature, 205); */
#define settings_set(attr, value)  __settings_set(offsetof(Settings, attr), (value))
