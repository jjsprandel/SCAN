#include "display_frames.h"

lv_obj_t *display_idle(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    return scr;
}

lv_obj_t *display_user_detected()
{
    lv_obj_t *scr = lv_obj_create(NULL);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Scan a valid\nID Card");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_check_in_success()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Check-In\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}
lv_obj_t *display_check_in_failed()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Check-In\nFailed");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_check_out_success()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Check-Out\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_database_validation()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(label1, "Checking\nDatabase");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}
