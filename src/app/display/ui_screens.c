#include "ui_screens.h"

// Global variables for dynamically updated content
static char user_name_buffer[32] = "User";
static char user_id_buffer[16] = "Unknown";
// Arrays to store label pointers for each screen
static lv_obj_t *user_name_labels[10] = {NULL}; // Array size should match number of screens
static lv_obj_t *user_id_labels[10] = {NULL};   // Array size should match number of screens
static lv_obj_t *error_label = NULL;

void ui_init(void)
{
    ui_styles_init();
}

void ui_update_user_info(const char *name, const char *id)
{
    if (name != NULL)
    {
        strncpy(user_name_buffer, name, sizeof(user_name_buffer) - 1);
        user_name_buffer[sizeof(user_name_buffer) - 1] = '\0';
    }

    if (id != NULL)
    {
        strncpy(user_id_buffer, id, sizeof(user_id_buffer) - 1);
        user_id_buffer[sizeof(user_id_buffer) - 1] = '\0';
    }

    for (int i = 0; i < 10; i++)
    {
        if (user_name_labels[i] != NULL)
        {
            lv_label_set_text(user_name_labels[i], user_name_buffer);
        }

        if (user_id_labels[i] != NULL)
        {
            lv_label_set_text(user_id_labels[i], user_id_buffer);
        }
    }
}

static lv_obj_t *create_image(lv_obj_t *parent, const void *src)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, src);
    return img;
}

lv_obj_t *ui_screen_hardware_init(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Initializing\nHardware", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 75, 75);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);

    lv_obj_t *icon = create_image(center_container, &gear);
    lv_obj_center(icon);

    return scr;
}

lv_obj_t *ui_screen_wifi_connecting(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xF39C12), 0);      // Lighter orange
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xD35400), 0); // Darker orange
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "WiFi\nConnecting", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    // Create a center container for the icon
    lv_obj_t *icon_container = lv_obj_create(cont);
    lv_obj_remove_style_all(icon_container);
    lv_obj_set_size(icon_container, 100, 100);
    lv_obj_center(icon_container);

    // Create the icon inside the center container
    lv_obj_t *icon = create_image(icon_container, &wifi);
    lv_obj_center(icon);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 75, 75);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);

    return scr;
}

lv_obj_t *ui_screen_software_init(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Initializing\nSoftware", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 75, 75);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);

    lv_obj_t *icon = create_image(center_container, &software);
    lv_obj_center(icon);
    return scr;
}

lv_obj_t *ui_screen_system_ready(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x27AE60), 0);      // Lighter green
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x196F3D), 0); // Darker green
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "System Ready", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    // Create a center container for the icon
    lv_obj_t *icon_container = lv_obj_create(cont);
    lv_obj_remove_style_all(icon_container);
    lv_obj_set_size(icon_container, 100, 100);
    lv_obj_center(icon_container);

    // Create the icon inside the center container
    lv_obj_t *icon = create_image(icon_container, &check);
    lv_obj_center(icon);

    return scr;
}

lv_obj_t *ui_screen_idle(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2980b9), 0);      // Lighter blue
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x1a5276), 0); // Darker blue
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *logo = create_image(cont, &ucf_logo);

    lv_obj_t *title = ui_create_title(cont, "Welcome", UI_STATE_NORMAL);

    lv_obj_t *info = lv_label_create(cont);
    lv_obj_add_style(info, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(info, UI_FONT_TINY, 0);
    lv_label_set_text(info, "UCF Recreation Center");

    return scr;
}

lv_obj_t *ui_screen_user_detected(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x3498DB), 0);      // Lighter blue
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x2874A6), 0); // Darker blue
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Welcome!", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_add_style(msg, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, 130);
    lv_label_set_text(msg, "Scan your ID\nor enter ID #");

    return scr;
}

lv_obj_t *ui_screen_database_validation(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xF39C12), 0);      // Lighter orange
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xD35400), 0); // Darker orange
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Validating", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_NORMAL), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);

    // lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_center(icon);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_add_style(msg, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, 130);
    lv_label_set_text(msg, "Checking database");

    return scr;
}

lv_obj_t *ui_screen_check_in_success(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x27AE60), 0);      // Lighter green
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x196F3D), 0); // Darker green
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Check-In\nSuccessful", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); // Use smaller font for title

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 80); // Smaller card size // changed from 70 to 80
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[0] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[0], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[0], UI_FONT_MEDIUM, 0);
    lv_label_set_text(user_name_labels[0], user_name_buffer);

    user_id_labels[0] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[0], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[0], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[0], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_check_out_success(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x27AE60), 0);      // Lighter green
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x196F3D), 0); // Darker green
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Check-Out\nSuccessful", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); // Use smaller font for title

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 80); // changed from 70 to 100
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[1], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[1], UI_FONT_MEDIUM, 0);
    lv_label_set_text(user_name_labels[1], user_name_buffer);

    user_id_labels[1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[1], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[1], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[1], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_validation_failure(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Validation\nFailed", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "User not found\nor access denied");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_error(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "System Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    // error_label = lv_label_create(cont);
    // lv_obj_set_style_text_color(error_label, UI_BLACK_TEXT, 0);
    // lv_obj_set_style_text_font(error_label, UI_FONT_SMALL, 0);
    // lv_label_set_text(error_label, "An error occurred");
    // lv_obj_center(error_label);

    return scr;
}

lv_obj_t *ui_screen_admin_enter_id(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x8E44AD), 0);      // Lighter purple
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x6C3483), 0); // Darker purple
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Admin Mode", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &admin);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *instr = lv_label_create(cont);
    lv_obj_set_style_text_color(instr, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(instr, UI_FONT_SMALL, 0);
    lv_label_set_text(instr, "Enter student ID\nto program card");
    lv_obj_center(instr);

    return scr;
}

lv_obj_t *ui_screen_admin_id_validating(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x8E44AD), 0);      // Lighter purple
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x6C3483), 0); // Darker purple
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Validating ID", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 75, 75);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_ADMIN), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_ADMIN), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);

    // lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_center(icon);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 170, 50);

    user_id_labels[5] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[5], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[5], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[5], user_id_buffer);
    lv_obj_center(user_id_labels[5]);

    return scr;
}

lv_obj_t *ui_screen_admin_tap_card(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x8E44AD), 0);      // Lighter purple
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x6C3483), 0); // Darker purple
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Tap Card", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 150, 80);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[3] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[3], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[3], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[3], user_name_buffer);

    user_id_labels[3] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[3], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[3], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[3], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_card_write_success(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x27AE60), 0);      // Lighter green
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x196F3D), 0); // Darker green
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Card\nWritten", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 150, 70);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[4] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[4], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[4], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[4], user_name_buffer);

    user_id_labels[4] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[4], UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[4], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[4], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_id_enter_error(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Invalid ID", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "ID not found\nor inactive user.");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_card_write_error(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Card Write\nFailed", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Failed to write to card");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_admin_error(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Admin Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "An error occurred in\nadmin mode. Please\ntry again.");
    lv_obj_center(msg);

    return scr;
}

// Admin Card Write Error Screen
lv_obj_t *ui_screen_admin_card_write_error(void)
{
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);

    // Add background color gradient
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE74C3C), 0);      // Lighter red
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0xB03A2E), 0); // Darker red
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);

    lv_obj_t *cont = ui_create_content_container(scr);

    lv_obj_t *title = ui_create_title(cont, "Write Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    // lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    // lv_obj_set_size(card, 170, 60);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_BLACK_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Failed to write to card.\nCard may be damaged\nor unsupported.");
    lv_obj_center(msg);

    return scr;
}
