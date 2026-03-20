#include "lvgl.h"

#include "config.h"

#include "battery.h"

static Battery s_battery;

/* ── Hardware stubs ─────────────────────────────────────────────────────────
 *
 * STUB – replace with actual hardware reads when the sensor is wired up.
 * ────────────────────────────────────────────────────────────────────────── */

static int32_t battery_read_percent(void)
{
    return BATTERY_DEFAULT_PERCENT;
}

static int32_t battery_read_charging(void)
{
    return BATTERY_DEFAULT_CHARGING;
}

/* ── Timer callbacks ─────────────────────────────────────────────────────── */

static void battery_percent_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    int32_t new_val = battery_read_percent();
    if(new_val != lv_subject_get_int(&s_battery.percent))
        lv_subject_set_int(&s_battery.percent, new_val);
}

static void battery_charging_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    int32_t new_val = battery_read_charging();
    if(new_val != lv_subject_get_int(&s_battery.charging))
        lv_subject_set_int(&s_battery.charging, new_val);
}

/* ── Public API ─────────────────────────────────────────────────────────────*/

Battery *battery_init(void)
{
    lv_subject_init_int(&s_battery.percent,  battery_read_percent());
    lv_subject_init_int(&s_battery.charging, battery_read_charging());
    lv_timer_create(battery_percent_timer_cb,  BATTERY_PERCENT_INTERVAL_MS,  NULL);
    lv_timer_create(battery_charging_timer_cb, BATTERY_CHARGING_INTERVAL_MS, NULL);
    return &s_battery;
}
