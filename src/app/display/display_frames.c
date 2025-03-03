#include "display_frames.h"

lv_obj_t *display_idle(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Scan a valid\nID Card");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}
lv_obj_t *display_check_in_failed(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Check-In\nFailed");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}
lv_obj_t *display_check_in_success(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Check-In\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_check_out_success(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Check-Out\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_transmitting(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(label1, "Checking\nDatabase");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}
