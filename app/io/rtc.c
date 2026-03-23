#include <time.h>
#include <string.h>

#include "lvgl.h"

#include "config.h"

#include "rtc.h"

/* ── Internals ───────────────────────────────────────────────────────────── */

typedef struct {
    struct tm   last_time;                    /* last sampled local time      */
    int32_t     tz_offset_s;                  /* UTC offset (east = positive) */
    char        tz_name[RTC_TZ_NAME_LEN];     /* abbreviated tz name          */

    rtc_cb_t    second_cbs[RTC_MAX_CBS];
    uint8_t     second_cb_count;

    rtc_cb_t    minute_cbs[RTC_MAX_CBS];
    uint8_t     minute_cb_count;

    rtc_cb_t    date_cbs[RTC_MAX_CBS];
    uint8_t     date_cb_count;

    lv_timer_t *timer;
} Rtc;

static Rtc rtc;

static void rtc_timer_cb(lv_timer_t *timer);

static void rtc_update_timer(void)
{
    if((rtc.second_cb_count + rtc.minute_cb_count + rtc.date_cb_count) == 0) {
        if(rtc.timer != NULL) {
            lv_timer_delete(rtc.timer);
            rtc.timer = NULL;
        }
        return;
    }

    uint32_t period = (rtc.second_cb_count > 0)
                      ? RTC_POLL_INTERVAL_MS
                      : RTC_POLL_INTERVAL_SLOW_MS;

    if(rtc.timer == NULL)
        rtc.timer = lv_timer_create(rtc_timer_cb, period, NULL);
    else
        lv_timer_set_period(rtc.timer, period);
}

/* ── Callback array helpers ──────────────────────────────────────────────── */

static inline void rtc_cb_reg(rtc_cb_t *arr, uint8_t *count, rtc_cb_t cb)
{
    LV_ASSERT(*count < RTC_MAX_CBS);
    arr[(*count)++] = cb;
    rtc_update_timer();
}

static inline void rtc_cb_unreg(rtc_cb_t *arr, uint8_t *count, rtc_cb_t cb)
{
    for(uint8_t i = 0; i < *count; i++) {
        if(arr[i] == cb) {
            for(uint8_t j = i; j < *count - 1; j++)
                arr[j] = arr[j + 1];
            (*count)--;
            rtc_update_timer();
            return;
        }
    }
}

static void rtc_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    time_t    now = time(NULL);
    struct tm time;
    localtime_r(&now, &time);

    if(time.tm_sec == rtc.last_time.tm_sec) return;

    const struct tm prev = rtc.last_time;
    rtc.last_time = time;

#if RTC_DST_SUPPORT
    if(time.tm_isdst != prev.tm_isdst) {
        rtc.tz_offset_s = (int32_t)time.tm_gmtoff;
        const char *tzn = tzname[time.tm_isdst > 0 ? 1 : 0];
        strncpy(rtc.tz_name, tzn, RTC_TZ_NAME_LEN - 1);
        rtc.tz_name[RTC_TZ_NAME_LEN - 1] = '\0';
    }
#endif

    for(uint8_t i = 0; i < rtc.second_cb_count; i++)
        rtc.second_cbs[i](&time);

    if(rtc.minute_cb_count > 0 && time.tm_min != prev.tm_min) {
        for(uint8_t i = 0; i < rtc.minute_cb_count; i++)
            rtc.minute_cbs[i](&time);
    }

    if(rtc.date_cb_count > 0 && time.tm_mday != prev.tm_mday) {
        for(uint8_t i = 0; i < rtc.date_cb_count; i++)
            rtc.date_cbs[i](&time);
    }
}

/* ── Public API ──────────────────────────────────────────────────────────── */

void rtc_init(void)
{
    /* Populate timezone globals (timezone, daylight, tzname[]). */
    tzset();

    time_t    now = time(NULL);
    struct tm time;
    localtime_r(&now, &time);

#if RTC_DST_SUPPORT
    rtc.tz_offset_s = (int32_t)time.tm_gmtoff;
#else
    /* POSIX 'timezone' is seconds WEST of UTC; flip the sign. */
    rtc.tz_offset_s = -(int32_t)timezone;
#endif

    const char *tzn = tzname[time.tm_isdst > 0 ? 1 : 0];
    strncpy(rtc.tz_name, tzn, RTC_TZ_NAME_LEN - 1);
    rtc.tz_name[RTC_TZ_NAME_LEN - 1] = '\0';

    rtc.last_time = time;
    /* Timer is created on demand when the first callback is registered. */
}

void rtc_reg_second_cb(rtc_cb_t cb)   { rtc_cb_reg  (rtc.second_cbs, &rtc.second_cb_count, cb); }
void rtc_unreg_second_cb(rtc_cb_t cb) { rtc_cb_unreg(rtc.second_cbs, &rtc.second_cb_count, cb); }
void rtc_reg_minute_cb(rtc_cb_t cb)   { rtc_cb_reg  (rtc.minute_cbs, &rtc.minute_cb_count, cb); }
void rtc_unreg_minute_cb(rtc_cb_t cb) { rtc_cb_unreg(rtc.minute_cbs, &rtc.minute_cb_count, cb); }
void rtc_reg_date_cb(rtc_cb_t cb)     { rtc_cb_reg  (rtc.date_cbs,   &rtc.date_cb_count,   cb); }
void rtc_unreg_date_cb(rtc_cb_t cb)   { rtc_cb_unreg(rtc.date_cbs,   &rtc.date_cb_count,   cb); }

int32_t     rtc_get_tz_offset(void)  { return rtc.tz_offset_s; }
const char *rtc_get_tz_name(void)    { return rtc.tz_name; }

struct tm rtc_get_local_time(void)
{
    time_t    now = time(NULL);
    struct tm time;
    localtime_r(&now, &time);
    return time;
}
