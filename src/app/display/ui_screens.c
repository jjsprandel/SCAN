#include "ui_screens.h"

// Global variables for dynamically updated content
static char user_name_buffer[32] = "User";
static char user_id_buffer[16] = "Unknown";
// Single pointers for current screen's labels
static lv_obj_t *current_user_name_label = NULL;
static lv_obj_t *current_user_id_label = NULL;
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

    // Update only the current screen's labels
    lv_label_set_text(current_user_name_label, user_name_buffer);
    // if (current_user_name_label != NULL)
    // {
    // }

    lv_label_set_text(current_user_id_label, user_id_buffer);
    // if (current_user_id_label != NULL)
    // {
    // }
}

lv_obj_t *ui_screen_hardware_init(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Set background color gradient using the new color constants
    scan_ui_set_background_color(scr, UI_COLOR_WHITE, UI_COLOR_WHITE);

    lv_obj_t *cont = scan_ui_create_content_container(scr);
    lv_obj_t *title = scan_ui_create_title(cont, "Initializing\nHardware", UI_COLOR_BLACK, UI_FONT_MEDIUM);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, LV_PART_MAIN);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 90, UI_COLOR_BLUE);

    lv_obj_t *icon = create_image(center_container, &gear);
    lv_obj_center(icon);

    return scr;
}

lv_obj_t *ui_screen_wifi_connecting(void)
{
    lv_obj_t *scr = scan_ui_create_screen();
    scan_ui_set_background_color(scr, UI_COLOR_WHITE, UI_COLOR_WHITE);

    lv_obj_t *cont = scan_ui_create_content_container(scr);
    lv_obj_t *title = scan_ui_create_title(cont, "WiFi\nConnecting", UI_COLOR_BLACK, UI_FONT_MEDIUM);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, LV_PART_MAIN);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 90, UI_COLOR_BLUE);

    lv_obj_t *icon = create_image(center_container, &wifi);
    lv_obj_center(icon);

    return scr;
}

lv_obj_t *ui_screen_software_init(void)
{
    lv_obj_t *scr = scan_ui_create_screen();
    scan_ui_set_background_color(scr, UI_COLOR_WHITE, UI_COLOR_WHITE);
    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Initializing\nSoftware", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 90, UI_COLOR_BLUE);

    lv_obj_t *icon = create_image(center_container, &software);
    lv_obj_center(icon);
    return scr;
}

lv_obj_t *ui_screen_system_ready(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x27AE60), lv_color_hex(0x196F3D));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "System\nReady", UI_COLOR_BLACK, UI_FONT_MEDIUM);
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
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, lv_color_hex(0x2980b9), lv_color_hex(0x1a5276));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *logo = create_image(cont, &ucf_logo);

    lv_obj_t *title = scan_ui_create_title(cont, "Welcome", UI_COLOR_BLACK, UI_FONT_MEDIUM);

    lv_obj_t *info = lv_label_create(cont);
    // lv_obj_add_style(info, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(info, UI_FONT_TINY, 0);
    lv_label_set_text(info, "UCF Recreation Center");

    return scr;
}

lv_obj_t *ui_screen_user_detected(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x3498DB), lv_color_hex(0x2874A6));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Welcome!", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *msg = lv_label_create(cont);
    // lv_obj_add_style(msg, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, 130);
    lv_label_set_text(msg, "Scan your ID\nor enter ID #");

    return scr;
}

lv_obj_t *ui_screen_database_validation(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0xF39C12), lv_color_hex(0xD35400));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Checking ID\nin Database", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 90, UI_COLOR_BLUE);

    // lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_center(icon);

    // lv_obj_t *msg = lv_label_create(cont);
    // // lv_obj_add_style(msg, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    // lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    // lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    // lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    // lv_obj_set_width(msg, 130);
    // lv_label_set_text(msg, "Checking database");

    return scr;
}

lv_obj_t *ui_screen_check_in_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x27AE60), lv_color_hex(0x196F3D));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Check-In", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); // Use smaller font for title

    lv_obj_t *icon = create_image(cont, &check);

    lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 65); // Smaller card size // changed from 70 to 80
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    current_user_name_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_name_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_name_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_name_label, user_name_buffer);

    current_user_id_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_id_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_id_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_id_label, user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_check_out_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x27AE60), lv_color_hex(0x196F3D));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Check-Out", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); // Use smaller font for title

    lv_obj_t *icon = create_image(cont, &check);

    lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 65); // changed from 70 to 100
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    current_user_name_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_name_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_name_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_name_label, user_name_buffer);

    current_user_id_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_id_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_id_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_id_label, user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_validation_failure(void)
{
    lv_obj_t *scr = scan_ui_create_screen();
    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Validation\nFailed", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "User not found\nor access denied");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0xE74C3C), lv_color_hex(0xB03A2E));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "System Error", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    return scr;
}

lv_obj_t *ui_screen_admin_enter_id(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x8E44AD), lv_color_hex(0x6C3483));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Admin Mode", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &admin);

    lv_obj_t *instr = lv_label_create(cont);
    lv_obj_set_style_text_color(instr, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(instr, UI_FONT_SMALL, 0);
    lv_label_set_text(instr, "Enter student ID\nto program card");
    lv_obj_center(instr);

    return scr;
}

lv_obj_t *ui_screen_admin_id_validating(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, lv_color_hex(0x8E44AD), lv_color_hex(0x6C3483));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Validating ID", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 75, UI_COLOR_BLUE);

    // lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_center(icon);

    lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 170, 50);

    current_user_id_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_id_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_id_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_id_label, user_id_buffer);
    lv_obj_center(current_user_id_label);

    return scr;
}

lv_obj_t *ui_screen_admin_tap_card(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x8E44AD), lv_color_hex(0x6C3483));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Tap Card", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 150, 65);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    current_user_name_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_name_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_name_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_name_label, user_name_buffer);

    current_user_id_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_id_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_id_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_id_label, user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_card_write_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0x27AE60), lv_color_hex(0x196F3D));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Card\nWritten", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *card = lv_obj_create(cont);
    // lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 150, 70);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    current_user_name_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_name_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_name_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_name_label, user_name_buffer);

    current_user_id_label = lv_label_create(card);
    lv_obj_set_style_text_color(current_user_id_label, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(current_user_id_label, UI_FONT_TINY, 0);
    lv_label_set_text(current_user_id_label, user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_id_enter_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0xE74C3C), lv_color_hex(0xB03A2E));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Invalid ID", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_TINY, 0);
    lv_label_set_text(msg, "ID not found\nor inactive user.");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_card_write_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0xE74C3C), lv_color_hex(0xB03A2E));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Card Write\nFailed", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Try Again");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_admin_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    // Add background color gradient
    scan_ui_set_background_color(scr, lv_color_hex(0xE74C3C), lv_color_hex(0xB03A2E));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Admin Error", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Try again");
    lv_obj_center(msg);

    return scr;
}

// Admin Card Write Error Screen
lv_obj_t *ui_screen_admin_card_write_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, lv_color_hex(0xE74C3C), lv_color_hex(0xB03A2E));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Write Error", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Failed to write to card.\nCard may be damaged\nor unsupported.");
    lv_obj_center(msg);

    return scr;
}
