#include "ui_screens.h"
#include "../main/include/global.h"

static char user_name_buffer[32] = "User";
static char user_id_buffer[16] = "Unknown";
static lv_obj_t *user_name_labels[NUM_SCREENS] = {NULL}; 
static lv_obj_t *user_id_labels[NUM_SCREENS] = {NULL};   
static lv_obj_t *error_label = NULL;

void ui_init(void)
{
    ui_styles_init();
}

void ui_update_user_info(const char* name, const char* id) {
    #ifdef UI_DEBUG
    ESP_LOGI("UI", "Updating user info - Name: %s, ID: %s", name ? name : "NULL", id ? id : "NULL");
    ESP_LOGI("UI", "Free heap: %lu", esp_get_free_heap_size());
    #endif
    
    if (name != NULL) {
        strncpy(user_name_buffer, name, sizeof(user_name_buffer) - 1);
        user_name_buffer[sizeof(user_name_buffer) - 1] = '\0';
    }
    
    if (id != NULL) {
        strncpy(user_id_buffer, id, sizeof(user_id_buffer) - 1);
        user_id_buffer[sizeof(user_id_buffer) - 1] = '\0';
    }
    #ifdef UI_DEBUG
    ESP_LOGI("UI", "After string copies");
    #endif
    for (int i = 0; i < NUM_SCREENS; i++) {
        if (user_name_labels[i] != NULL) {
            lv_label_set_text(user_name_labels[i], user_name_buffer);
        }

        if (user_id_labels[i] != NULL) {
            lv_label_set_text(user_id_labels[i], user_id_buffer);
        }
    }
    #ifdef UI_DEBUG
    ESP_LOGI("UI", "End of ui_update_user_info");
    ESP_LOGI("UI", "Free heap: %lu", esp_get_free_heap_size());
    #endif
}

lv_obj_t *ui_screen_hardware_init(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

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

    scan_ui_set_background_color(scr, lv_color_hex(0x27AE60), lv_color_hex(0x196F3D));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "System\nReady", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon_container = lv_obj_create(cont);
    lv_obj_remove_style_all(icon_container);
    lv_obj_set_size(icon_container, 100, 100);
    lv_obj_center(icon_container);

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
    lv_obj_set_style_text_font(info, UI_FONT_TINY, 0);
    lv_label_set_text(info, device_info.kiosk_location);

    return scr;
}

lv_obj_t *ui_screen_user_detected(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, lv_color_hex(0x3498DB), lv_color_hex(0x2874A6));

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Welcome!", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, 130);
    lv_label_set_text(msg, "Scan your ID\nor enter ID #");

    return scr;
}

lv_obj_t *ui_screen_keypad_entry_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();
    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);
    
    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Invalid ID\nLength", UI_COLOR_BLACK, UI_FONT_SMALL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 40);
    user_id_labels[STATE_KEYPAD_ENTRY_ERROR] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[STATE_KEYPAD_ENTRY_ERROR], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[STATE_KEYPAD_ENTRY_ERROR], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[STATE_KEYPAD_ENTRY_ERROR], user_id_buffer);
    lv_obj_center(user_id_labels[STATE_KEYPAD_ENTRY_ERROR]);

    return scr;
    
}

lv_obj_t *ui_screen_database_validation(void)
{
    lv_obj_t *scr = scan_ui_create_screen();   
    scan_ui_set_background_color(scr, UI_COLOR_ORANGE, UI_COLOR_ORANGE_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Checking ID", UI_COLOR_BLACK, UI_FONT_SMALL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 100);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 75, UI_COLOR_BLUE);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 40);
    user_id_labels[STATE_DATABASE_VALIDATION] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[STATE_DATABASE_VALIDATION], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[STATE_DATABASE_VALIDATION], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[STATE_DATABASE_VALIDATION], user_id_buffer);
    lv_obj_center(user_id_labels[STATE_DATABASE_VALIDATION]);

    return scr;
}

lv_obj_t *ui_screen_check_in_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_GREEN, UI_COLOR_GREEN_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Check-In", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &check);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 65);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[STATE_CHECK_IN] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[STATE_CHECK_IN], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_name_labels[STATE_CHECK_IN], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[STATE_CHECK_IN], user_name_buffer);
    ESP_LOGI("SCREEN", "Created check-in success name label with: %s", user_name_buffer);

    user_id_labels[STATE_CHECK_IN] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[STATE_CHECK_IN], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[STATE_CHECK_IN], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[STATE_CHECK_IN], user_id_buffer);
    ESP_LOGI("SCREEN", "Created check-in success ID label with: %s", user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_check_out_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_GREEN, UI_COLOR_GREEN_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Check-Out", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); 

    lv_obj_t *icon = create_image(cont, &check);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 65); 
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[STATE_CHECK_OUT] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[STATE_CHECK_OUT], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_name_labels[STATE_CHECK_OUT], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[STATE_CHECK_OUT], user_name_buffer);

    user_id_labels[STATE_CHECK_OUT] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[STATE_CHECK_OUT], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[STATE_CHECK_OUT], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[STATE_CHECK_OUT], user_id_buffer);

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

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 40); 
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_id_labels[STATE_VALIDATION_FAILURE] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[STATE_VALIDATION_FAILURE], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[STATE_VALIDATION_FAILURE], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[STATE_VALIDATION_FAILURE], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "System Error", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    return scr;
}

lv_obj_t *ui_screen_admin_enter_id(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_PURPLE, UI_COLOR_PURPLE_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Admin Mode", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &admin);

    lv_obj_t *instr = lv_label_create(cont);
    lv_obj_set_style_text_color(instr, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(instr, UI_FONT_SMALL, 0);
    lv_label_set_text(instr, "Enter user ID\nto program card");
    lv_obj_center(instr);

    return scr;
}

lv_obj_t *ui_screen_admin_id_validating(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_PURPLE, UI_COLOR_PURPLE_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Checking ID", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);

    lv_obj_t *spinner = lv_spinner_create(center_container);
    scan_ui_create_spinner(spinner, 75, UI_COLOR_BLUE);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 170, 50);

    user_id_labels[ADMIN_STATE_VALIDATE_ID+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[ADMIN_STATE_VALIDATE_ID+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[ADMIN_STATE_VALIDATE_ID+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[ADMIN_STATE_VALIDATE_ID+STATE_ERROR+1], user_id_buffer);
    lv_obj_center(user_id_labels[ADMIN_STATE_VALIDATE_ID+STATE_ERROR+1]);

    return scr;
}

lv_obj_t *ui_screen_admin_tap_card(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_PURPLE, UI_COLOR_PURPLE_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Tap & Hold ID", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &id_card);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 150, 65);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_name_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], user_name_buffer);

    user_id_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[ADMIN_STATE_TAP_CARD+STATE_ERROR+1], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_card_write_success(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_GREEN, UI_COLOR_GREEN_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Card\nWritten", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &write_id_icon);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 150, 70);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_name_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_name_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], user_name_buffer);

    user_id_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[ADMIN_STATE_CARD_WRITE_SUCCESS+STATE_ERROR+1], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_id_enter_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Invalid ID", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_set_size(card, 150, 40);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    user_id_labels[ADMIN_STATE_ENTER_ID_ERROR+STATE_ERROR+1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[ADMIN_STATE_ENTER_ID_ERROR+STATE_ERROR+1], UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(user_id_labels[ADMIN_STATE_ENTER_ID_ERROR+STATE_ERROR+1], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[ADMIN_STATE_ENTER_ID_ERROR+STATE_ERROR+1], user_id_buffer);

    return scr;
}

lv_obj_t *ui_screen_card_write_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

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

    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

    lv_obj_t *cont = scan_ui_create_content_container(scr);

    lv_obj_t *title = scan_ui_create_title(cont, "Admin Error", UI_COLOR_BLACK, UI_FONT_MEDIUM);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);

    lv_obj_t *icon = create_image(cont, &error);

    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_set_style_text_color(msg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Exiting admin mode");
    lv_obj_center(msg);

    return scr;
}

lv_obj_t *ui_screen_admin_card_write_error(void)
{
    lv_obj_t *scr = scan_ui_create_screen();

    scan_ui_set_background_color(scr, UI_COLOR_RED, UI_COLOR_RED_DARK);

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
