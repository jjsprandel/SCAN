#include "admin_frames.h"

lv_obj_t *display_admin_enter_id(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Enter an\nexisting\nID Number");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_admin_id_validating(void)
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

lv_obj_t *display_admin_tap_card(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label1, "Scan a\nblank ID\nCard");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_card_write_success(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(label1, "Card Write\nSuccess");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_id_enter_error(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Error Entering\nID");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_card_write_error(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Error\nwriting\nnew card");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}

lv_obj_t *display_admin_error(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    static lv_style_t style;

    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_32);

    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(label1, "Admin\nstate\nerror");

    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    return scr;
}