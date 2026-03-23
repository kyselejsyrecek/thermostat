#include "lvgl.h"

#include "config.h"
#include "io/display.h"
#include "gui/ui.h"
#include "pm.h"

/*
 * Power management – display dim / sleep on inactivity.
 *
 * State machine:
 *
 *   ACTIVE   ──[PM_INACTIVITY_TIMEOUT_MS of inactivity]────►  DIMMING
 *   DIMMING  ──[PM_DIM_HOLD_MS elapsed]────────────────────►  SLEEPING
 *   DIMMING  ──[user activity]─────────────────────────────►  ACTIVE
 *   SLEEPING ──[user activity]─────────────────────────────►  ACTIVE
 *
 * Brightness transitions are animated via lv_anim_t in this module.
 * display_set_brightness() (io/display wrapper) is called for each
 * interpolated step; the platform HAL applies each step instantly.
 *
 * Sleep sequence:
 *   1. Animate brightness: s_app_brightness → dimmed  (PM_DISPLAY_FADE_OUT_MS)
 *   2. After PM_DIM_HOLD_MS, animate brightness: dimmed → 0  (PM_DISPLAY_FADE_OUT_MS)
 *   3. On animation complete: display_off(), ui_sleep()
 *
 * Wake sequence:
 *   1. display_on()  (physical backlight on, if coming from SLEEPING)
 *   2. Animate brightness: current → s_app_brightness  (PM_DISPLAY_FADE_IN_MS)
 */

/* ── State ───────────────────────────────────────────────────────────────── */

typedef enum {
    PM_STATE_ACTIVE,
    PM_STATE_DIMMING,
    PM_STATE_SLEEPING,
} PmState;

static PmState     s_state;
static uint8_t     s_app_brightness;
static lv_timer_t *s_pm_timer    = NULL;
static lv_timer_t *s_sleep_timer = NULL;

/* Sentinel variable used solely as a stable pointer to identify the running
 * PM brightness animation.  lv_anim_delete(&s_brightness_anim, NULL) cancels
 * any in-progress brightness transition regardless of direction. */
static int         s_brightness_anim;

/* ── Animation helpers ───────────────────────────────────────────────────── */

/* Per-frame exec_cb: applies the interpolated brightness value. */
static void pm_brightness_step(void *var, int32_t v)
{
    (void)var;
    display_set_brightness((uint8_t)v);
}

/* Start a smooth brightness transition from the current brightness to
 * to_percent.  Cancels any running PM brightness animation first.
 * When already at to_percent, completed_cb is invoked immediately (if set). */
static void pm_brightness_transition(uint8_t to_percent, uint32_t duration_ms,
                                     lv_anim_completed_cb_t completed_cb)
{
    lv_anim_delete(&s_brightness_anim, NULL);

    uint8_t current_brightness = display_get_brightness();
    if(current_brightness == to_percent) {
        if(completed_cb) completed_cb(NULL);
        return;
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &s_brightness_anim);
    lv_anim_set_exec_cb(&a, pm_brightness_step);
    lv_anim_set_values(&a, (int32_t)current_brightness, (int32_t)to_percent);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    if(completed_cb) lv_anim_set_completed_cb(&a, completed_cb);
    lv_anim_start(&a);
}

/* ── Internal helpers ────────────────────────────────────────────────────── */

/* Animation-complete callback for the final fade-to-zero: turn the display
 * off physically and navigate back to the main screen. */
static void pm_sleep_anim_completed_cb(lv_anim_t *a)
{
    (void)a;
    display_off();
    ui_sleep();
    s_state = PM_STATE_SLEEPING;
}

/* Restore display to the pre-dim brightness and return to ACTIVE state.
 * Safe to call from any state, including while a brightness animation is
 * running. */
static void pm_wake(void)
{
    bool was_sleeping = (s_state == PM_STATE_SLEEPING);

    if(s_sleep_timer != NULL) {
        lv_timer_delete(s_sleep_timer);
        s_sleep_timer = NULL;
    }

    lv_timer_set_period(s_pm_timer, PM_TIMER_PERIOD_MS);
    s_state = PM_STATE_ACTIVE;

    if(was_sleeping) display_on();

    pm_brightness_transition(s_app_brightness, PM_DISPLAY_FADE_IN_MS, NULL);
}

/* One-shot timer callback: dim hold time elapsed – fade to black, then sleep. */
static void pm_sleep_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    s_sleep_timer = NULL;

    pm_brightness_transition(0, PM_DISPLAY_FADE_OUT_MS, pm_sleep_anim_completed_cb);
}

/* Initiate the sleep sequence.
 * Briefly dims the display (PM_DIM_AMOUNT %) before turning it off. */
static void pm_sleep(void)
{
    s_app_brightness = display_get_brightness();
    s_state          = PM_STATE_DIMMING;

    uint8_t dimmed = (uint8_t)((uint32_t)s_app_brightness * PM_DISPLAY_DIM_AMOUNT / 100u);
    pm_brightness_transition(dimmed, PM_DISPLAY_FADE_OUT_MS, NULL);

    lv_timer_set_period(s_pm_timer, PM_TIMER_PERIOD_FAST_MS);

    s_sleep_timer = lv_timer_create(pm_sleep_timer_cb, PM_DISPLAY_DIM_HOLD_MS, NULL);
    lv_timer_set_repeat_count(s_sleep_timer, 1);
}

/* ── Periodic inactivity check ───────────────────────────────────────────── */

static void pm_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    uint32_t inactive_ms = lv_display_get_inactive_time(NULL);

    switch(s_state) {

    case PM_STATE_ACTIVE:
        if(inactive_ms >= PM_INACTIVITY_TIMEOUT_MS)
            pm_sleep();
        break;

    case PM_STATE_DIMMING:
        /* The inactive counter was reset since the last tick: user acted. */
        if(inactive_ms < PM_TIMER_PERIOD_FAST_MS * 2u)
            pm_wake();
        break;

    case PM_STATE_SLEEPING:
        if(inactive_ms < PM_TIMER_PERIOD_FAST_MS * 2u)
            pm_wake();
        break;
    }
}

/* ── Public API ──────────────────────────────────────────────────────────── */

void pm_init(void)
{
    s_state          = PM_STATE_ACTIVE;
    s_app_brightness = display_get_brightness();
    s_pm_timer       = lv_timer_create(pm_timer_cb, PM_TIMER_PERIOD_MS, NULL);
}
