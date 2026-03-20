#include "lvgl.h"

#include "config.h"

#include "navigation.h"

/* ── Compile-time gesture availability ───────────────────────────────────── */

#define _NAV_ALL      ((UI_NAV_TO_SET_TEMPERATURE) | (UI_NAV_FROM_SET_TEMPERATURE))
#define _NAV_SWIPE    (UI_NAV_SWIPE_LEFT | UI_NAV_SWIPE_RIGHT | UI_NAV_SWIPE_UP | UI_NAV_SWIPE_DOWN)
#define _NAV_HAS_SWIPE   ((_NAV_ALL) & _NAV_SWIPE)
#define _NAV_HAS_TAP     ((_NAV_ALL) & UI_NAV_TAP)
#define _NAV_HAS_DTAP    ((_NAV_ALL) & UI_NAV_DOUBLE_TAP)
#define _NAV_HAS_LPRESS  ((_NAV_ALL) & UI_NAV_LONG_PRESS)

/* ── Internal types ──────────────────────────────────────────────────────── */

#define NAV_SCREENS_MAX  8

typedef enum {
    NAV_IDLE,         /* no touch active                        */
    NAV_PRESSED,      /* press recorded, waiting for movement   */
    NAV_DRAGGING,     /* actively dragging – both screens move  */
    NAV_ANIMATING,    /* snap / completion anim in progress     */
} navigation_state_t;

typedef struct {
    lv_obj_t    *screens[NAV_SCREENS_MAX];
    uint8_t      screen_count;

    uint8_t      current;       /* index of the currently visible screen   */
    navigation_state_t  state;

    lv_point_t   press_start;   /* absolute touch point at initial press   */
    int32_t      drag_offset;   /* signed px travel along the drag axis    */
    int8_t       drag_axis;     /* 0 = horizontal, 1 = vertical            */
    int8_t       drag_dir_sign; /* +1 or -1 (determines incoming position) */

    bool         drag_attempted; /* true once finger moved > 8 px          */

    uint8_t      anim_pending;  /* number of running lv_anim instances     */
    uint8_t      anim_next;     /* target screen index for the running anim */

    uint8_t      cover_idx;     /* which of current/next is the cover (slides) */
    int8_t       layers[NAV_SCREENS_MAX]; /* z-order per screen; higher = in front */

    nav_gesture_t        last_gesture;     /* gesture that triggered last transition */
    nav_transition_cb_t  transition_cb;

    lv_obj_t   *drag_guards[NAV_SCREENS_MAX]; /* objects frozen during swipe  */
    uint8_t     drag_guard_count;
} NavigationState;

static NavigationState s_nav;

/* ── Helpers ──────────────────────────────────────────────────────────────── */

static inline lv_obj_t *cur_root(void)
{
    return s_nav.screens[s_nav.current];
}

static inline uint8_t next_screen(void)
{
    return (uint8_t)((s_nav.current + 1) % s_nav.screen_count);
}

static inline lv_obj_t *nxt_root(void)
{
    return s_nav.screens[next_screen()];
}

static inline uint32_t active_mask(void)
{
    return (s_nav.current == 0) ? UI_NAV_TO_SET_TEMPERATURE : UI_NAV_FROM_SET_TEMPERATURE;
}

static inline uint32_t dir_to_flag(lv_dir_t d)
{
    if(d == LV_DIR_LEFT)   return UI_NAV_SWIPE_LEFT;
    if(d == LV_DIR_RIGHT)  return UI_NAV_SWIPE_RIGHT;
    if(d == LV_DIR_TOP)    return UI_NAV_SWIPE_UP;
    return UI_NAV_SWIPE_DOWN;
}

static void guards_freeze(void)
{
    for(uint8_t i = 0; i < s_nav.drag_guard_count; i++)
        lv_obj_remove_flag(s_nav.drag_guards[i], LV_OBJ_FLAG_CLICKABLE);
}

static void guards_unfreeze(void)
{
    for(uint8_t i = 0; i < s_nav.drag_guard_count; i++)
        lv_obj_add_flag(s_nav.drag_guards[i], LV_OBJ_FLAG_CLICKABLE);
}

/* ── Animation ────────────────────────────────────────────────────────────── */

static void anim_completed_cb(lv_anim_t *a)
{
    if(--s_nav.anim_pending > 0) return;

    if(s_nav.anim_next != s_nav.current) {
        /* Completed transition: hide outgoing, make incoming the current. */
        lv_obj_t *out = cur_root();      /* still the old current */
        lv_obj_add_flag(out, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(out, 0, 0);       /* reset for future use */
        s_nav.current = s_nav.anim_next;
        /* Notify listener (e.g. save settings on confirmed swipe). */
        if(s_nav.transition_cb)
            s_nav.transition_cb(out, cur_root(), s_nav.last_gesture);
        s_nav.last_gesture = NAV_GESTURE_NONE;
    } else {
        /* Canceled: snap incoming back off-screen and hide it. */
        lv_obj_t *in = nxt_root();
        lv_obj_add_flag(in, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(in, 0, 0);
        /* cur_root was already animated back to (0,0) by cancel_transition(). */
    }

    s_nav.state          = NAV_IDLE;
    guards_unfreeze();
    s_nav.drag_axis      = -1;
    s_nav.drag_dir_sign  = 0;
    s_nav.drag_attempted = false;
}

static void launch_anim(lv_obj_t *obj, int32_t start, int32_t end, bool is_x)
{
    /* Cancel any running animation on this axis for this object first. */
    lv_anim_delete(obj, is_x
                   ? (lv_anim_exec_xcb_t)lv_obj_set_x
                   : (lv_anim_exec_xcb_t)lv_obj_set_y);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, is_x
                        ? (lv_anim_exec_xcb_t)lv_obj_set_x
                        : (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_duration(&a, UI_SWIPE_ANIM_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&a, anim_completed_cb);
    lv_anim_start(&a);
    s_nav.anim_pending++;
}

/* Finish a swipe: only the cover screen animates to its final position.
 * The base screen is already at (0,0) and stays there. */
static void complete_transition(void)
{
    /* Report the exact swipe direction so callbacks can act on it. */
    if(s_nav.drag_axis == 0)
        s_nav.last_gesture = (s_nav.drag_dir_sign > 0)
                             ? NAV_GESTURE_SWIPE_RIGHT : NAV_GESTURE_SWIPE_LEFT;
    else
        s_nav.last_gesture = (s_nav.drag_dir_sign > 0)
                             ? NAV_GESTURE_SWIPE_DOWN : NAV_GESTURE_SWIPE_UP;
    bool    is_x = (s_nav.drag_axis == 0);
    int32_t dim  = is_x ? UI_DISPLAY_WIDTH : UI_DISPLAY_HEIGHT;

    s_nav.anim_next = next_screen();
    s_nav.state     = NAV_ANIMATING;

    lv_obj_t *cover   = s_nav.screens[s_nav.cover_idx];
    int32_t   cur_pos = is_x ? lv_obj_get_x(cover) : lv_obj_get_y(cover);
    /* Cover exits if it was current; settles at centre if it was incoming. */
    int32_t   end_pos = (s_nav.cover_idx == s_nav.current)
                        ?  s_nav.drag_dir_sign * dim
                        :  0;
    launch_anim(cover, cur_pos, end_pos, is_x);
}

/* Snap back: only the cover returns to its off-screen resting position.
 * The base screen stays at (0,0) unchanged. */
static void cancel_transition(void)
{
    bool    is_x = (s_nav.drag_axis == 0);
    int32_t dim  = is_x ? UI_DISPLAY_WIDTH : UI_DISPLAY_HEIGHT;

    s_nav.anim_next = s_nav.current;
    s_nav.state     = NAV_ANIMATING;

    lv_obj_t *cover   = s_nav.screens[s_nav.cover_idx];
    int32_t   cur_pos = is_x ? lv_obj_get_x(cover) : lv_obj_get_y(cover);
    /* Cover returns to centre if it was current; exits if it was incoming. */
    int32_t   end_pos = (s_nav.cover_idx == s_nav.current)
                        ?  0
                        : -s_nav.drag_dir_sign * dim;
    launch_anim(cover, cur_pos, end_pos, is_x);
}

/* Navigate without drag (tap / double-tap / long-press): horizontal slide.
 * Going to a higher screen index slides left; returning slides right.
 * Only the cover screen moves; the base stays fixed at (0,0). */
static void instant_transition(void)
{
    uint8_t nxt  = next_screen();
    int8_t  sign = (nxt > s_nav.current) ? -1 : 1;
    int32_t dim  = UI_DISPLAY_WIDTH;

    s_nav.cover_idx     = (s_nav.layers[s_nav.current] >= s_nav.layers[nxt])
                          ? s_nav.current : nxt;
    s_nav.drag_offset   = 0;
    s_nav.drag_axis     = 0;
    s_nav.drag_dir_sign = sign;
    s_nav.anim_pending  = 0;
    s_nav.anim_next     = nxt;
    s_nav.state         = NAV_ANIMATING;
    guards_freeze();

    lv_obj_t *cover = s_nav.screens[s_nav.cover_idx];
    if(s_nav.cover_idx == s_nav.current) {
        /* Cover (current) slides off; reveal base (next) underneath. */
        lv_obj_set_pos(nxt_root(), 0, 0);
        lv_obj_clear_flag(nxt_root(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_to_index(cover, -1);
        launch_anim(cover, 0, sign * dim, true);
    } else {
        /* Cover (next) slides in from edge; base (current) stays put. */
        lv_obj_set_pos(cover, -sign * dim, 0);
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_to_index(cover, -1);
        launch_anim(cover, -sign * dim, 0, true);
    }
}

/* ── Touch event handlers ─────────────────────────────────────────────────── */

#if _NAV_HAS_SWIPE
static void on_pressed_cb(lv_event_t *e)
{
    if(s_nav.state != NAV_IDLE) return;

    lv_indev_t *indev = lv_indev_active();
    lv_indev_get_point(indev, &s_nav.press_start);

    s_nav.state          = NAV_PRESSED;
    s_nav.drag_offset    = 0;
    s_nav.drag_axis      = -1;
    s_nav.drag_dir_sign  = 0;
    s_nav.drag_attempted = false;
}

static void on_pressing_cb(lv_event_t *e)
{
    if(s_nav.state == NAV_IDLE || s_nav.state == NAV_ANIMATING) return;

    lv_indev_t *indev = lv_indev_active();
    lv_point_t  pt;
    lv_indev_get_point(indev, &pt);

    int32_t dx = pt.x - s_nav.press_start.x;
    int32_t dy = pt.y - s_nav.press_start.y;

    /* Determine axis on first movement beyond the axis-lock threshold. */
    if(s_nav.drag_axis < 0) {
        int32_t adx = dx < 0 ? -dx : dx;
        int32_t ady = dy < 0 ? -dy : dy;
        if(adx < UI_SWIPE_AXIS_LOCK_PX && ady < UI_SWIPE_AXIS_LOCK_PX) return;
        s_nav.drag_axis      = (adx >= ady) ? 0 : 1;
        s_nav.drag_attempted = true;
    }

    int32_t offset = (s_nav.drag_axis == 0) ? dx : dy;
    int32_t dim    = (s_nav.drag_axis == 0) ? UI_DISPLAY_WIDTH : UI_DISPLAY_HEIGHT;
    bool    is_x   = (s_nav.drag_axis == 0);

    /* When the finger returns to the press origin, hide the incoming screen
     * and reset direction so the next movement re-evaluates cleanly. */
    if(offset == 0) {
        if(s_nav.drag_dir_sign != 0) {
            lv_obj_t *in = nxt_root();
            lv_obj_add_flag(in, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(in, 0, 0);
            s_nav.drag_dir_sign = 0;
        }
        if(is_x) lv_obj_set_x(cur_root(), 0);
        else     lv_obj_set_y(cur_root(), 0);
        s_nav.drag_offset = 0;
        return;
    }

    int8_t new_sign = (offset > 0) ? 1 : -1;

    /* On first movement or when the drag direction reverses: verify the new
     * direction is enabled and (re-)position the incoming screen on the
     * correct edge.  The repositioning happens while the incoming screen is
     * still off-screen (|offset| is small at the moment of sign change), so
     * no visual glitch occurs. */
    if(s_nav.drag_dir_sign != new_sign) {
        lv_dir_t dir;
        if(is_x) dir = (new_sign > 0) ? LV_DIR_RIGHT : LV_DIR_LEFT;
        else     dir = (new_sign > 0) ? LV_DIR_BOTTOM : LV_DIR_TOP;

        if(!(active_mask() & dir_to_flag(dir))) {
            /* New direction not configured – snap back and go idle. */
            if(s_nav.drag_dir_sign != 0) {
                lv_obj_t *in = nxt_root();
                lv_obj_add_flag(in, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(in, 0, 0);
            }
            if(is_x) lv_obj_set_x(cur_root(), 0);
            else     lv_obj_set_y(cur_root(), 0);
            s_nav.drag_dir_sign = 0;
            s_nav.state = NAV_IDLE;
            guards_unfreeze();
            return;
        }

        /* Determine which screen is the cover (moves) and which is the base (fixed). */
        uint8_t nxt = next_screen();
        s_nav.cover_idx = (s_nav.layers[s_nav.current] >= s_nav.layers[nxt])
                          ? s_nav.current : nxt;
        lv_obj_t *cover = s_nav.screens[s_nav.cover_idx];
        if(s_nav.cover_idx == nxt) {
            /* Cover is the incoming screen: position it at the correct edge. */
            if(is_x) lv_obj_set_pos(cover, -new_sign * dim, 0);
            else     lv_obj_set_pos(cover, 0, -new_sign * dim);
            lv_obj_clear_flag(cover, LV_OBJ_FLAG_HIDDEN);
        } else {
            /* Cover is current: reveal the base (next) fixed at (0,0). */
            lv_obj_set_pos(nxt_root(), 0, 0);
            lv_obj_clear_flag(nxt_root(), LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_move_to_index(cover, -1);   /* cover always rendered on top */
        s_nav.drag_dir_sign = new_sign;
        s_nav.state = NAV_DRAGGING;
        guards_freeze();
    }

    s_nav.drag_offset = offset;

    /* Move only the cover screen; the base stays fixed at (0,0). */
    lv_obj_t *cover = s_nav.screens[s_nav.cover_idx];
    if(is_x)
        lv_obj_set_x(cover, (s_nav.cover_idx == s_nav.current)
                     ? offset : offset - s_nav.drag_dir_sign * dim);
    else
        lv_obj_set_y(cover, (s_nav.cover_idx == s_nav.current)
                     ? offset : offset - s_nav.drag_dir_sign * dim);
}

static void on_released_cb(lv_event_t *e)
{
    if(s_nav.state == NAV_ANIMATING) return;

    if(s_nav.state != NAV_DRAGGING) {
        s_nav.state = NAV_IDLE;
        guards_unfreeze();
        return;
    }

    int32_t dim       = (s_nav.drag_axis == 0) ? UI_DISPLAY_WIDTH : UI_DISPLAY_HEIGHT;
    int32_t threshold = dim * UI_SWIPE_THRESHOLD_PERCENT / 100;
    int32_t abs_off   = s_nav.drag_offset < 0 ? -s_nav.drag_offset : s_nav.drag_offset;

    if(abs_off >= threshold) complete_transition();
    else                     cancel_transition();
}
#endif /* _NAV_HAS_SWIPE */

/* ── Point gesture handlers ───────────────────────────────────────────────── */

#if _NAV_HAS_TAP
static void on_clicked_cb(lv_event_t *e)
{
    if(s_nav.state != NAV_IDLE) return;
    if(s_nav.drag_attempted) return;          /* was actually a drag */
    if(!(active_mask() & UI_NAV_TAP)) return;
    s_nav.last_gesture = NAV_GESTURE_TAP;
    instant_transition();
}
#endif

#if _NAV_HAS_DTAP
static void on_double_clicked_cb(lv_event_t *e)
{
    if(s_nav.state != NAV_IDLE) return;
    if(!(active_mask() & UI_NAV_DOUBLE_TAP)) return;
    s_nav.last_gesture = NAV_GESTURE_DOUBLE_TAP;
    instant_transition();
}
#endif

#if _NAV_HAS_LPRESS
static void on_long_pressed_cb(lv_event_t *e)
{
    if(s_nav.state != NAV_IDLE) return;
    if(!(active_mask() & UI_NAV_LONG_PRESS)) return;
    s_nav.last_gesture = NAV_GESTURE_LONG_PRESS;
    instant_transition();
}
#endif

/* ── Public API ───────────────────────────────────────────────────────────── */

void navigation_init(lv_obj_t *screens[])
{
    s_nav.drag_guard_count = 0;
    s_nav.screen_count  = 0;
    s_nav.current       = 0;
    s_nav.state         = NAV_IDLE;
    s_nav.drag_axis     = -1;
    s_nav.drag_dir_sign = 0;
    s_nav.anim_pending  = 0;
    s_nav.cover_idx     = 0;
    s_nav.last_gesture  = NAV_GESTURE_NONE;
    s_nav.transition_cb = NULL;

    /* Copy the NULL-terminated array into the NavigationState. */
    for(uint8_t i = 0; screens[i] != NULL && i < NAV_SCREENS_MAX; i++) {
        s_nav.screens[i] = screens[i];
        s_nav.screen_count++;
    }

    /* Default all layers to 0; caller may override with navigation_set_layer(). */
    for(uint8_t i = 0; i < s_nav.screen_count; i++)
        s_nav.layers[i] = 0;

    /* Layout: screen[0] visible at (0,0); all others hidden to the right. */
    for(uint8_t i = 0; i < s_nav.screen_count; i++) {
        lv_obj_set_pos(s_nav.screens[i], i == 0 ? 0 : UI_DISPLAY_WIDTH, 0);
        if(i != 0) lv_obj_add_flag(s_nav.screens[i], LV_OBJ_FLAG_HIDDEN);
    }

    /* Register input handlers on every screen root.
     * Only gesture categories enabled in at least one navigation mask are
     * registered – checked at compile time via the _NAV_HAS_* macros. */
    for(uint8_t i = 0; i < s_nav.screen_count; i++) {
#if _NAV_HAS_SWIPE
        lv_obj_add_event_cb(s_nav.screens[i], on_pressed_cb,  LV_EVENT_PRESSED,  &s_nav);
        lv_obj_add_event_cb(s_nav.screens[i], on_pressing_cb, LV_EVENT_PRESSING, &s_nav);
        lv_obj_add_event_cb(s_nav.screens[i], on_released_cb, LV_EVENT_RELEASED, &s_nav);
#endif
#if _NAV_HAS_TAP
        lv_obj_add_event_cb(s_nav.screens[i], on_clicked_cb,  LV_EVENT_CLICKED,  &s_nav);
#endif
#if _NAV_HAS_DTAP
        lv_obj_add_event_cb(s_nav.screens[i], on_double_clicked_cb,
                            LV_EVENT_DOUBLE_CLICKED, &s_nav);
#endif
#if _NAV_HAS_LPRESS
        lv_obj_add_event_cb(s_nav.screens[i], on_long_pressed_cb,
                            LV_EVENT_LONG_PRESSED, &s_nav);
#endif
    }
}

void navigation_add_drag_guard(lv_obj_t *obj)
{
    if(s_nav.drag_guard_count < NAV_SCREENS_MAX)
        s_nav.drag_guards[s_nav.drag_guard_count++] = obj;
}

/* Assign a z-order layer to a registered screen.  During any transition the
 * screen with the higher layer is the "cover": it slides in/out above the
 * other screen which stays fixed at (0,0).  Call after navigation_init(). */
void navigation_set_layer(lv_obj_t *screen, int8_t layer)
{
    for(uint8_t i = 0; i < s_nav.screen_count; i++) {
        if(s_nav.screens[i] == screen) {
            s_nav.layers[i] = layer;
            return;
        }
    }
}

void navigation_set_transition_cb(nav_transition_cb_t cb)
{
    s_nav.transition_cb = cb;
}
