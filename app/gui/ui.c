#include "lvgl.h"

#include "config.h"
#include "sensor/thermometer.h"
#include "sensor/battery.h"
#include "screen/set_temperature.h"
#include "screen/main.h"

#include "navigation.h"
#include "ui.h"

static UiHandle s_handle;

static lv_obj_t *bouding_circle_create(void)
{
    lv_obj_t *circle;

    /* Screen background: black outside the circular display area. */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);

    /* Circular container – clips all children to a circle.
     * The gradient is applied by the individual screen; the screen corners
     * (outside the circle) stay black. */
    circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle,
                    LV_HOR_RES - 2 * UI_SIM_MARGIN_H,
                    LV_VER_RES - 2 * UI_SIM_MARGIN_V);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(circle, true, 0);
    lv_obj_set_style_pad_all(circle, 0, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE);

#ifdef UI_SIMULATION
    /* Bezel ring – sibling of circle (child of the screen), NOT a child of
     * circle, because circle has clip_corner(true) which would crop anything
     * that extends beyond its boundary.  Placed here so that all display-edge
     * geometry is owned by bouding_circle_create.
     * Arc centre sits exactly on the display edge; width grows outward into
     * the simulator margin, leaving the 360×360 display area pixel-perfect.
     * lv_arc uses lv_draw_arc which applies SW anti-aliasing. */
    lv_obj_t *ring = lv_arc_create(lv_screen_active());
    /* Shift the ring inward by the AA zone width so that anti-aliased edge
     * pixels blend with live screen content rather than the black bounding
     * circle.  circ_calc_aa4 uses 4× oversampling, producing exactly 1
     * physical-pixel AA zone when LV_DRAW_SW_COMPLEX is enabled; 0 otherwise. */
#if LV_DRAW_SW_COMPLEX
#  define _BEZEL_AA_PX  1
#else
#  define _BEZEL_AA_PX  0
#endif
    lv_obj_set_size(ring,
                    UI_DISPLAY_WIDTH  + 2 * (UI_SIM_BORDER_WIDTH - _BEZEL_AA_PX),
                    UI_DISPLAY_HEIGHT + 2 * (UI_SIM_BORDER_WIDTH - _BEZEL_AA_PX));
#undef _BEZEL_AA_PX
    lv_obj_center(ring);
    lv_arc_set_bg_angles(ring, 0, 360);
    lv_obj_set_style_arc_width(ring, UI_SIM_BORDER_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring, UI_SIM_BORDER_COLOR, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ring, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(ring, 0, LV_PART_MAIN);
    lv_obj_remove_flag(ring, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_to_index(ring, -1); /* draw on top of circle + all its children */
#endif

    return circle;
}

UiHandle *ui_init(void)
{
    lv_obj_t *viewport = bouding_circle_create();

    /* ── Active screen ──────────────────────────────────────────────────────
     * Uncomment exactly one of the blocks below to select the screen to
     * display.  The other block should remain commented out.
     * ─────────────────────────────────────────────────────────────────────── */

    /* Main screen (sensor read-out, black background): */
    s_handle.thermometer  = thermometer_init();
    s_handle.battery      = battery_init();
    s_handle.main_screen  = main_screen_create(viewport, s_handle.thermometer,
                                               s_handle.battery);

    /* Set-temperature screen (arc temperature picker + gradient background): */
    s_handle.set_temperature = set_temperature_screen_create(viewport);

    /* Navigation: main screen is default, set-temperature is reachable
     * via gestures configured in config.h (UI_NAV_TO/FROM_SET_TEMPERATURE). */
    navigation_init((lv_obj_t *[]){ s_handle.main_screen->root,
                                    s_handle.set_temperature->root,
                                    NULL
                                  });
    /* Main screen is the "cover": it slides over the set-temperature screen,
     * which stays fixed at its final position (0,0) during transitions. */
    navigation_set_layer(s_handle.main_screen->root, 1);
    navigation_add_drag_guard(s_handle.set_temperature->picker.arc);
    navigation_set_transition_cb(set_temperature_nav_transition_cb);

    return &s_handle;
}
