#include "lvgl.h"

#include "config.h"

#include "thermometer.h"

static Thermometer s_thermometer;

/* ── Hardware read ──────────────────────────────────────────────────────────
 *
 * STUB – replace with an actual hardware read when the sensor is wired up.
 * Returns the current temperature in tenths of a degree Celsius.
 * ────────────────────────────────────────────────────────────────────────── */
static int32_t thermometer_read_tenths(void)
{
    return 215; /* 21.5 °C */
}

/* ── Periodic timer callback ─────────────────────────────────────────────── */

static void thermometer_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    int32_t new_val = thermometer_read_tenths();
    if(new_val != lv_subject_get_int(&s_thermometer.value)) {
        lv_subject_set_int(&s_thermometer.value, new_val);
    }
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

Thermometer *thermometer_init(void)
{
    lv_subject_init_int(&s_thermometer.value, thermometer_read_tenths());
    lv_timer_create(thermometer_timer_cb, THERMOMETER_INTERVAL_MS, NULL);
    return &s_thermometer;
}
