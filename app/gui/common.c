#include "lvgl.h"

#include "config.h"

#include "common.h"

void gui_label_style(lv_obj_t *lbl, int32_t w, lv_text_align_t align, lv_color_t color)
{
    lv_obj_set_width(lbl, w);
    lv_obj_set_style_text_align(lbl, align, 0);
    lv_obj_set_style_text_font(lbl, &UI_FONT, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
}

int32_t gui_measure_max_width(const char *strings[], int count)
{
    int32_t max_w = 0;
    for(int i = 0; i < count; i++) {
        lv_point_t sz;
        lv_text_get_size(&sz, strings[i], &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

int32_t gui_max_int_part_width(int min_celsius, int max_celsius)
{
    int32_t max_w = 0;
    for(int t = min_celsius; t <= max_celsius; t++) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", t);
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > max_w) max_w = sz.x;
    }
    return max_w;
}

void gui_temp_label_update(TempLabels *labels, int32_t tenths)
{
    char buf[24];

#if UI_TEMP_CENTER_EXACT
    /* Single centred label – format the whole string. */
#  if UI_TEMP_ENABLE_DECIMALS
    lv_snprintf(buf, sizeof(buf), "%d%c%d \xc2\xb0" "C",
                (int)(tenths / 10), UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  else
    lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", (int)(tenths / 10));
#  endif
    lv_label_set_text(labels->int_lbl, buf);

#elif UI_TEMP_ENABLE_DECIMALS
    /* Split-label: integer part. */
    lv_snprintf(buf, sizeof(buf), "%d", (int)(tenths / 10));
    lv_label_set_text(labels->int_lbl, buf);

    /* Decimal part (with or without appended unit). */
#  if UI_TEMP_FIXED_UNIT
    lv_snprintf(buf, sizeof(buf), "%c%d", UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  else
    lv_snprintf(buf, sizeof(buf), "%c%d \xc2\xb0" "C",
                UI_TEMP_DECIMAL_SEP, (int)(tenths % 10));
#  endif
    lv_label_set_text(labels->frac_lbl, buf);

#else
    /* Whole-degrees mode: single right-aligned label. */
    lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", (int)(tenths / 10));
    lv_label_set_text(labels->int_lbl, buf);
#endif
}

void gui_temp_labels_create(lv_obj_t *parent, TempLabels *out, lv_color_t color)
{
    out->frac_lbl = NULL;
    out->unit_lbl = NULL;

#if UI_TEMP_CENTER_EXACT
    /* Single label – measure the widest string over the configured range. */
    int32_t label_w = 0;
    for(int t = UI_TEMP_MIN; t <= UI_TEMP_MAX; t++) {
        char buf[24];
#  if UI_TEMP_ENABLE_DECIMALS
        for(int d = 0; d <= 9; d++) {
            lv_snprintf(buf, sizeof(buf), "%d%c%d \xc2\xb0" "C",
                        t, UI_TEMP_DECIMAL_SEP, d);
            lv_point_t sz;
            lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            if(sz.x > label_w) label_w = sz.x;
        }
#  else
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", t);
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > label_w) label_w = sz.x;
#  endif
    }
    out->int_lbl = lv_label_create(parent);
    gui_label_style(out->int_lbl, label_w, LV_TEXT_ALIGN_CENTER, color);
    lv_obj_center(out->int_lbl);

#elif UI_TEMP_ENABLE_DECIMALS
    int32_t int_w = gui_max_int_part_width(UI_TEMP_MIN, UI_TEMP_MAX);

#  if UI_TEMP_FIXED_UNIT
    /* Three-label layout: [ integer | ,X | °C ]
     * Block layout (centred at parent centre c):
     *   centre(int_lbl)  = c - (dec_w + unit_w) / 2
     *   centre(frac_lbl) = c + (int_w - unit_w) / 2
     *   centre(unit_lbl) = c + (int_w + dec_w)  / 2  */
    char dec_strs[10][8];
    const char *dec_ptrs[10];
    for(int d = 0; d <= 9; d++) {
        lv_snprintf(dec_strs[d], sizeof(dec_strs[d]), "%c%d", UI_TEMP_DECIMAL_SEP, d);
        dec_ptrs[d] = dec_strs[d];
    }
    int32_t dec_w  = gui_measure_max_width(dec_ptrs, 10);
    const char *unit_str[] = { " \xc2\xb0" "C" };
    int32_t unit_w = gui_measure_max_width(unit_str, 1);

    out->int_lbl = lv_label_create(parent);
    gui_label_style(out->int_lbl, int_w, LV_TEXT_ALIGN_RIGHT, color);
    lv_obj_align(out->int_lbl, LV_ALIGN_CENTER, -(dec_w + unit_w) / 2, 0);

    out->frac_lbl = lv_label_create(parent);
    gui_label_style(out->frac_lbl, dec_w, LV_TEXT_ALIGN_LEFT, color);
    lv_obj_align(out->frac_lbl, LV_ALIGN_CENTER, (int_w - unit_w) / 2, 0);

    out->unit_lbl = lv_label_create(parent);
    gui_label_style(out->unit_lbl, unit_w, LV_TEXT_ALIGN_LEFT, color);
    lv_obj_align(out->unit_lbl, LV_ALIGN_CENTER, (int_w + dec_w) / 2, 0);
    lv_label_set_text(out->unit_lbl, " \xc2\xb0" "C");

#  else /* UI_TEMP_ENABLE_DECIMALS && !UI_TEMP_FIXED_UNIT */
    /* Two-label layout: [ integer | ,X °C ]
     * Decimal separator stays at a fixed horizontal position. */
    char frac_strs[10][16];
    const char *frac_ptrs[10];
    for(int d = 0; d <= 9; d++) {
        lv_snprintf(frac_strs[d], sizeof(frac_strs[d]),
                    "%c%d \xc2\xb0" "C", UI_TEMP_DECIMAL_SEP, d);
        frac_ptrs[d] = frac_strs[d];
    }
    int32_t frac_w = gui_measure_max_width(frac_ptrs, 10);

    out->int_lbl = lv_label_create(parent);
    gui_label_style(out->int_lbl, int_w, LV_TEXT_ALIGN_RIGHT, color);
    lv_obj_align(out->int_lbl, LV_ALIGN_CENTER, -(frac_w / 2), 0);

    out->frac_lbl = lv_label_create(parent);
    gui_label_style(out->frac_lbl, frac_w, LV_TEXT_ALIGN_LEFT, color);
    lv_obj_align(out->frac_lbl, LV_ALIGN_CENTER, (int_w / 2), 0);
#  endif

#else /* !UI_TEMP_CENTER_EXACT && !UI_TEMP_ENABLE_DECIMALS */
    /* Single right-aligned label; whole degrees only. */
    int32_t label_w = 0;
    for(int t = UI_TEMP_MIN; t <= UI_TEMP_MAX; t++) {
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d \xc2\xb0" "C", t);
        lv_point_t sz;
        lv_text_get_size(&sz, buf, &UI_FONT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if(sz.x > label_w) label_w = sz.x;
    }
    out->int_lbl = lv_label_create(parent);
    gui_label_style(out->int_lbl, label_w, LV_TEXT_ALIGN_RIGHT, color);
    lv_obj_center(out->int_lbl);
#endif
}

lv_obj_t *gui_screen_root_create(lv_obj_t *parent)
{
    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_size(root, UI_DISPLAY_WIDTH, UI_DISPLAY_HEIGHT);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_style_radius(root, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(root, true, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    return root;
}
