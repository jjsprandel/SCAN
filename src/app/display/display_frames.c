// #include "display_frames.h"

// lv_obj_t *display_idle(lv_display_t *disp)
// {
//     lv_obj_t *scr = lv_display_get_screen_active(disp);

//     static lv_style_t style;
//     lv_style_init(&style);
//     lv_style_set_text_font(&style, &lv_font_montserrat_32);

//     lv_obj_t *label1 = lv_label_create(scr);
//     lv_obj_add_style(label1, &style, 0);
//     lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_BLUE), 0);
//     lv_label_set_text(label1, "Scan a valid\nID Card");

//     lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

//     return label1;
// }
// lv_obj_t *display_check_in_failed(lv_display_t *disp)
// {
//     lv_obj_t *scr = lv_display_get_screen_active(disp);
//     static lv_style_t style;

//     lv_style_init(&style);
//     lv_style_set_text_font(&style, &lv_font_montserrat_32);

//     lv_obj_t *label1 = lv_label_create(scr);
//     lv_obj_add_style(label1, &style, 0);
//     lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_RED), 0);
//     lv_label_set_text(label1, "Check-In\nFailed");

//     lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

//     return label1;
// }
// lv_obj_t *display_check_in_success(lv_display_t *disp)
// {
//     lv_obj_t *scr = lv_display_get_screen_active(disp);
//     static lv_style_t style;
//     lv_style_init(&style);
//     lv_style_set_text_font(&style, &lv_font_montserrat_32);

//     lv_obj_t *label1 = lv_label_create(scr);
//     lv_obj_add_style(label1, &style, 0);
//     lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_GREEN), 0);
//     lv_label_set_text(label1, "Check-In\nSuccess");

//     lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

//     return label1;
// }

// lv_obj_t *display_transmitting(lv_display_t *disp)
// {
//     lv_obj_t *scr = lv_display_get_screen_active(disp);
//     static lv_style_t style;
//     lv_style_init(&style);
//     lv_style_set_text_font(&style, &lv_font_montserrat_32);

//     lv_obj_t *label1 = lv_label_create(scr);
//     lv_obj_add_style(label1, &style, 0);
//     lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_YELLOW), 0);
//     lv_label_set_text(label1, "Checking\nDatabase");

//     lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

//     return label1;
// }

#include "display_frames.h"

lv_obj_t *display_idle(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    /* Create a circular container for the frame */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 200, 200); // A 200x200 container fits nicely within 240x240
    lv_obj_center(cont);

    /* Initialize and apply container style */
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_bg_color(&cont_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_radius(&cont_style, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_color(&cont_style, lv_color_black());
    lv_style_set_shadow_opa(&cont_style, LV_OPA_30);
    lv_style_set_shadow_width(&cont_style, 10);
    lv_style_set_shadow_spread(&cont_style, 5);
    lv_obj_add_style(cont, &cont_style, 0);

    /* Create a label for the message */
    lv_obj_t *label = lv_label_create(cont);
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, &lv_font_montserrat_32);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_obj_add_style(label, &label_style, 0);
    lv_label_set_text(label, "Scan a valid\nID Card");
    lv_obj_center(label);

    return cont;
}

lv_obj_t *display_check_in_failed(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    /* Create circular container */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 200, 200);
    lv_obj_center(cont);

    /* Style container with a red background */
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_bg_color(&cont_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_radius(&cont_style, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_color(&cont_style, lv_color_black());
    lv_style_set_shadow_opa(&cont_style, LV_OPA_30);
    lv_style_set_shadow_width(&cont_style, 10);
    lv_style_set_shadow_spread(&cont_style, 5);
    lv_obj_add_style(cont, &cont_style, 0);

    /* Create and style the label */
    lv_obj_t *label = lv_label_create(cont);
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, &lv_font_montserrat_32);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_obj_add_style(label, &label_style, 0);
    lv_label_set_text(label, "Check-In\nFailed");
    lv_obj_center(label);

    return cont;
}

lv_obj_t *display_check_in_success(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    /* Create circular container */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 200, 200);
    lv_obj_center(cont);

    /* Style container with a green background */
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_bg_color(&cont_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_radius(&cont_style, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_color(&cont_style, lv_color_black());
    lv_style_set_shadow_opa(&cont_style, LV_OPA_30);
    lv_style_set_shadow_width(&cont_style, 10);
    lv_style_set_shadow_spread(&cont_style, 5);
    lv_obj_add_style(cont, &cont_style, 0);

    /* Create and style the label */
    lv_obj_t *label = lv_label_create(cont);
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, &lv_font_montserrat_32);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_obj_add_style(label, &label_style, 0);
    lv_label_set_text(label, "Check-In\nSuccess");
    lv_obj_center(label);

    return cont;
}

lv_obj_t *display_transmitting(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    /* Create circular container */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 200, 200);
    lv_obj_center(cont);

    /* Style container with a yellow background */
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_bg_color(&cont_style, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_radius(&cont_style, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_color(&cont_style, lv_color_black());
    lv_style_set_shadow_opa(&cont_style, LV_OPA_30);
    lv_style_set_shadow_width(&cont_style, 10);
    lv_style_set_shadow_spread(&cont_style, 5);
    lv_obj_add_style(cont, &cont_style, 0);

    /* Create and style the label */
    lv_obj_t *label = lv_label_create(cont);
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, &lv_font_montserrat_32);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_obj_add_style(label, &label_style, 0);
    lv_label_set_text(label, "Checking\nDatabase");
    lv_obj_center(label);

    return cont;
}
