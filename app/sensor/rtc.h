#pragma once

#include <time.h>
#include <stdint.h>

/*
 * Rtc – system-clock driver (local time, not monotonic).
 *
 * Reads local time via localtime_r() and fires registered callbacks directly
 * when the relevant time unit changes.  All callbacks receive a pointer to the
 * current struct tm in local time – callers extract whatever fields they need.
 *
 * Three callback types:
 *   second  – fired on every second tick
 *   minute  – fired when the minute changes
 *   date    – fired when the calendar date (day/month/year) changes
 *
 * Timer period:
 *   When all callbacks are unregistered the polling timer is deleted.
 *   When there are callbacks but none for seconds, the timer runs at the
 *   slower RTC_POLL_INTERVAL_SLOW_MS rate.  Registering the first second
 *   callback switches the timer to the faster RTC_POLL_INTERVAL_MS rate;
 *   unregistering the last one reverts to the slow rate.
 *
 * Timezone information is cached at initialisation:
 *   tz_offset_s – UTC offset in seconds, east-of-UTC positive
 *                 (e.g. UTC+1 → 3600, UTC-5 → -18000)
 *   tz_name     – abbreviated timezone name (e.g. "CET", "EST")
 *
 * Usage:
 *   rtc_init();
 *   rtc_reg_second_cb(my_second_handler);
 *   rtc_reg_minute_cb(my_minute_handler);
 *   rtc_reg_date_cb(my_date_handler);
 *
 *   // Inside a callback:
 *   void my_minute_handler(const struct tm *time) {
 *       LV_LOG_USER("%02d:%02d", time->tm_hour, time->tm_min);
 *   }
 *
 *   // Read current local time on demand:
 *   struct tm now = rtc_get_local_time();
 */

typedef void (*rtc_cb_t)(const struct tm *time);

/* Initialise the RTC driver: cache timezone data and seed internal state with
 * the current local time.  No timer is started until at least one callback is
 * registered.  Must be called after lv_init(). */
void rtc_init(void);

/* Register/unregister a callback fired on every second tick.
 * Registering the first second callback switches the timer to the fast rate.
 * Unregistering the last second callback reverts to the slow rate.
 * Asserts if RTC_MAX_CBS is exceeded; silently ignores unknown callbacks. */
void rtc_reg_second_cb(rtc_cb_t cb);
void rtc_unreg_second_cb(rtc_cb_t cb);

/* Register/unregister a callback fired when the minute changes. */
void rtc_reg_minute_cb(rtc_cb_t cb);
void rtc_unreg_minute_cb(rtc_cb_t cb);

/* Register/unregister a callback fired when the calendar date changes. */
void rtc_reg_date_cb(rtc_cb_t cb);
void rtc_unreg_date_cb(rtc_cb_t cb);

/* Return the cached UTC offset in seconds (east-of-UTC = positive).
 * Example: UTC+1 → 3600; UTC-5 → -18000. */
int32_t rtc_get_tz_offset(void);

/* Return the cached abbreviated timezone name (e.g. "CET", "EST"). */
const char *rtc_get_tz_name(void);

/* Read the current local time directly from the system clock.
 * Returns a struct tm filled by localtime_r(). */
struct tm rtc_get_local_time(void);
