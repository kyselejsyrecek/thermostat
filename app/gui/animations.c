#include "lvgl.h"

#include "animations.h"

/* ── Shared exec callback ───────────────────────────────────────────────────*/

static void opa_anim_exec_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

/* ── Fade-in ─────────────────────────────────────────────────────────────── */

void gui_anim_fade_in(lv_obj_t *obj, uint32_t duration_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, opa_anim_exec_cb);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_start(&a);
}

/* ── Fade-out ────────────────────────────────────────────────────────────── */

void gui_anim_fade_out(lv_obj_t *obj, uint32_t duration_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, opa_anim_exec_cb);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_start(&a);
}

/* ── Blink ───────────────────────────────────────────────────────────────────
 *
 * One cycle: fade-out → pause → fade-in, then repeat forever.
 *
 * LVGL does not expose a "play-list" API, so the two phases are chained via
 * the ready callback of the fade-out animation.  The ready callback starts a
 * new fade-in (with the configured delay via lv_anim_set_delay).  The fade-in
 * ready callback in turn restarts the fade-out, producing an infinite loop.
 * ──────────────────────────────────────────────────────────────────────────*/

/* Forward declaration. */
static void blink_fade_in_ready_cb(lv_anim_t *a);
static void blink_fade_out_ready_cb(lv_anim_t *a);

/* The blink parameters are stored in two user-data slots piggybacked onto the
 * animation's path_extra_d field (unused by lv_anim_path_linear).  We use a
 * small heap-allocated context instead so the lifetimes are clear. */
typedef struct {
    uint32_t fade_out_ms;
    uint32_t delay_ms;
    uint32_t fade_in_ms;
} BlinkCtx;

static void blink_fade_in_ready_cb(lv_anim_t *a)
{
    BlinkCtx *ctx = (BlinkCtx *)a->user_data;
    lv_obj_t *obj = (lv_obj_t *)a->var;

    lv_anim_t next;
    lv_anim_init(&next);
    lv_anim_set_var(&next, obj);
    lv_anim_set_exec_cb(&next, opa_anim_exec_cb);
    lv_anim_set_duration(&next, ctx->fade_out_ms);
    lv_anim_set_values(&next, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_completed_cb(&next, blink_fade_out_ready_cb);
    next.user_data = ctx;
    lv_anim_start(&next);
}

static void blink_fade_out_ready_cb(lv_anim_t *a)
{
    BlinkCtx *ctx = (BlinkCtx *)a->user_data;
    lv_obj_t *obj = (lv_obj_t *)a->var;

    lv_anim_t next;
    lv_anim_init(&next);
    lv_anim_set_var(&next, obj);
    lv_anim_set_exec_cb(&next, opa_anim_exec_cb);
    lv_anim_set_duration(&next, ctx->fade_in_ms);
    lv_anim_set_delay(&next, ctx->delay_ms);
    lv_anim_set_values(&next, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_completed_cb(&next, blink_fade_in_ready_cb);
    next.user_data = ctx;
    lv_anim_start(&next);
}

void gui_anim_blink(lv_obj_t *obj,
                    uint32_t fade_out_ms,
                    uint32_t delay_ms,
                    uint32_t fade_in_ms)
{
    /* Allocate context; freed when gui_anim_stop is called or the object
     * is deleted (LVGL cancels pending animations on deletion). */
    BlinkCtx *ctx = lv_malloc(sizeof(BlinkCtx));
    LV_ASSERT_MALLOC(ctx);
    ctx->fade_out_ms = fade_out_ms;
    ctx->delay_ms    = delay_ms;
    ctx->fade_in_ms  = fade_in_ms;

    /* Kick off with the first fade-out. */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, opa_anim_exec_cb);
    lv_anim_set_duration(&a, fade_out_ms);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_completed_cb(&a, blink_fade_out_ready_cb);
    a.user_data = ctx;
    lv_anim_start(&a);
}

/* ── Stop ────────────────────────────────────────────────────────────────── */

void gui_anim_stop(lv_obj_t *obj)
{
    /* Retrieve and free the blink context if one is running.  lv_anim_get
     * returns the first active animation on obj with the given exec_cb. */
    lv_anim_t *running = lv_anim_get(obj, opa_anim_exec_cb);
    if(running && running->user_data) {
        /* Only blink animations store a user_data context. */
        lv_free(running->user_data);
        running->user_data = NULL;
    }
    lv_anim_delete(obj, opa_anim_exec_cb);
    lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);
}
