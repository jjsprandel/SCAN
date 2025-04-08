#include "ui_styles.h"

lv_obj_t *scan_ui_create_screen(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    return screen;
}

lv_obj_t *create_image(lv_obj_t *parent, const void *src)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, src);
    return img;
}

lv_obj_t *scan_ui_create_content_container(lv_obj_t *parent)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_PCT(90), LV_PCT(90));
    lv_obj_set_style_pad_all(container, 8, 0); 
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
    lv_obj_center(container);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(container, 4, 0);
    return container;
}

lv_obj_t *scan_ui_create_title(lv_obj_t *parent, const char *text, lv_color_t text_color, const lv_font_t *font)
{
    lv_obj_t *title = lv_label_create(parent);
    scan_ui_style_text(title, text_color, font, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(title, text);
    return title;
}

void scan_ui_create_spinner(lv_obj_t *spinner, int diameter, lv_color_t color)
{
    lv_obj_set_size(spinner, diameter, diameter);

    lv_style_t spinner_style;
    lv_style_init(&spinner_style);
    lv_style_set_radius(&spinner_style, 10);
    lv_style_set_border_width(&spinner_style, 0);
    lv_style_set_bg_color(&spinner_style, color);

    lv_obj_add_style(spinner, &spinner_style, LV_PART_MAIN);
    lv_obj_add_style(spinner, &spinner_style, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);
}

void scan_ui_set_background_color(lv_obj_t *scr, lv_color_t color1, lv_color_t color2)
{
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, color1, 0);
    lv_obj_set_style_bg_grad_color(scr, color2, 0);
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);
}

void scan_ui_style_card(lv_obj_t *card, lv_color_t bg_color, uint8_t radius, uint8_t padding)
{
    lv_obj_set_style_radius(card, radius, 0);
    lv_obj_set_style_bg_color(card, bg_color, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_80, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, padding, 0);
    lv_obj_set_style_shadow_width(card, 8, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
}

void scan_ui_style_text(lv_obj_t *text, lv_color_t color, const lv_font_t *font, lv_text_align_t align)
{
    lv_obj_set_style_text_color(text, color, 0);
    lv_obj_set_style_text_font(text, font, 0);
    lv_obj_set_style_text_align(text, align, 0);
}

void scan_ui_set_screen_transition(lv_obj_t *new_screen)
{
    lv_obj_t *current_screen = lv_scr_act();
    if (current_screen == new_screen)
    {
        return;
    }
    lv_scr_load(new_screen);
}