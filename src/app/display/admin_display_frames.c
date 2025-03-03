#include "admin_display_frames.h"

lv_obj_t *display_admin_enter_id(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Enter an existing\nID Number");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_admin_id_validating(lv_display_t *disp)
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

lv_obj_t *display_admin_tap_card(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Scan a blank\nID Card");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_card_write_success(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Card Write\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_id_enter_error(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Error Entering\nID");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_card_write_error(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Error writing\nnew card");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}

lv_obj_t *display_admin_error(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Admin state\nerror");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return label1;
}