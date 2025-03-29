#include "ui_screens.h"
#include "ui_styles.h"
#include "ui_symbols.h"
#include "ui_assets.h"
#include "string.h"

// Global variables for dynamically updated content
static char user_name_buffer[32] = "User";
static char user_id_buffer[16] = "Unknown";
// Arrays to store label pointers for each screen
static lv_obj_t *user_name_labels[10] = {NULL}; // Array size should match number of screens
static lv_obj_t *user_id_labels[10] = {NULL};   // Array size should match number of screens
static lv_obj_t *error_label = NULL;

// Initialize all UI components
void ui_init(void) {
    // Initialize the styles
    ui_styles_init();
}

// Update user information on screens
void ui_update_user_info(const char* name, const char* id) {
    if (name != NULL) {
        strncpy(user_name_buffer, name, sizeof(user_name_buffer) - 1);
        user_name_buffer[sizeof(user_name_buffer) - 1] = '\0';
    }
    
    if (id != NULL) {
        strncpy(user_id_buffer, id, sizeof(user_id_buffer) - 1);
        user_id_buffer[sizeof(user_id_buffer) - 1] = '\0';
    }
    
    // Update ALL labels with new data
    for (int i = 0; i < 10; i++) {
        if (user_name_labels[i] != NULL) {
            lv_label_set_text(user_name_labels[i], user_name_buffer);
        }
        
        if (user_id_labels[i] != NULL) {
            lv_label_set_text(user_id_labels[i], user_id_buffer);
        }
    }
}

// Helper to create an image
static lv_obj_t* create_image(lv_obj_t *parent, const void *src) {
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, src);
    return img;
}

// Idle screen - pulsing prompt to approach
lv_obj_t *ui_screen_idle(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);
    
    // Create a radial gradient as background
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2980b9), 0);  // Lighter blue
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x1a5276), 0); // Darker blue
    lv_obj_set_style_bg_main_stop(scr, 100, 0);
    lv_obj_set_style_bg_grad_stop(scr, 255, 0);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add UCF logo at the top
    lv_obj_t *logo = create_image(cont, &ucf_logo);
    
    // Add title
    lv_obj_t *title = ui_create_title(cont, "Welcome", UI_STATE_NORMAL);
    
    // Add small info text at the bottom
    lv_obj_t *info = lv_label_create(cont);
    lv_obj_add_style(info, ui_get_style(UI_STYLE_TEXT, UI_STATE_NORMAL), 0);
    lv_obj_set_style_text_font(info, UI_FONT_TINY, 0);
    lv_label_set_text(info, "UCF Recreation Center");
    
    return scr;
}

// User detected screen - scan ID card instruction
lv_obj_t *ui_screen_user_detected(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_NORMAL);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add title - make it more compact
    lv_obj_t *title = ui_create_title(cont, "Welcome!", UI_STATE_NORMAL);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0); // Smaller font for title
    
    // Add ID card icon - centered and slightly smaller
    lv_obj_t *icon = create_image(cont, &id_card);
    // lv_obj_set_size(icon, 48, 48);
    
    // Create a card with instruction - make it narrower to fit better
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_NORMAL), 0);
    lv_obj_set_size(card, 160, 60);
    
    lv_obj_t *instr = lv_label_create(card);
    lv_obj_set_style_text_color(instr, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(instr, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(instr, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(instr, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(instr, 140);
    lv_label_set_text(instr, "Scan your ID or enter ID number");
    lv_obj_center(instr);
    
    // Create a glowing indicator for the NFC area - make it slightly smaller
    lv_obj_t *nfc_area = lv_obj_create(cont);
    lv_obj_remove_style_all(nfc_area);
    lv_obj_set_size(nfc_area, 90, 5);
    lv_obj_set_style_radius(nfc_area, 3, 0);
    lv_obj_set_style_bg_color(nfc_area, lv_color_hex(0xaed6f1), 0);
    lv_obj_set_style_bg_opa(nfc_area, LV_OPA_70, 0);
    lv_obj_set_style_shadow_width(nfc_area, 10, 0);
    lv_obj_set_style_shadow_color(nfc_area, lv_color_hex(0x5dade2), 0);
    lv_obj_set_style_shadow_opa(nfc_area, LV_OPA_50, 0);
    
    // Start pulse animation on NFC area
    // ui_start_pulse_animation(nfc_area);
    
    return scr;
}

// Database validation screen - animated loading
lv_obj_t *ui_screen_database_validation(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_WARNING);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add title with smaller font and more compact text
    lv_obj_t *title = ui_create_title(cont, "Validating", UI_STATE_WARNING);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Container for spinner and database icon
    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);
    
    // Add spinner/loading indicator - make it smaller
    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 85, 85);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_WARNING), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_WARNING), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);
    
    // Add database icon in the middle of the spinner
    lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_set_size(icon, 32, 32); 
    lv_obj_center(icon);
    
    // Add message below with smaller font and width constraint
    lv_obj_t *msg = lv_label_create(cont);
    lv_obj_add_style(msg, ui_get_style(UI_STYLE_TEXT, UI_STATE_WARNING), 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, 130);
    lv_label_set_text(msg, "Checking database");
    
    return scr;
}

// Check-in success screen - animated checkmark with confirmation
lv_obj_t *ui_screen_check_in_success(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add large checkmark icon
    lv_obj_t *check = create_image(cont, &check);
    // lv_obj_set_size(check, 48, 48);  // Make icon a bit smaller
    
    // Add success message
    lv_obj_t *title = ui_create_title(cont, "Check-In\nSuccessful", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);  // Use smaller font for title
    
    // Create a card with user info
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 70);  // Smaller card size
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add user name (if available)
    user_name_labels[0] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[0], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[0], UI_FONT_MEDIUM, 0);
    lv_label_set_text(user_name_labels[0], user_name_buffer);
    
    // Add user ID
    user_id_labels[0] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[0], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[0], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[0], user_id_buffer);
    
    return scr;
}

// Check-out success screen
lv_obj_t *ui_screen_check_out_success(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add success message
    lv_obj_t *title = ui_create_title(cont, "Check-Out\nSuccessful", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);  // Use smaller font for title
    
    // Create a card with user info
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 70);  // Smaller card size
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add user name (if available)
    user_name_labels[1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[1], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[1], UI_FONT_MEDIUM, 0);
    lv_label_set_text(user_name_labels[1], user_name_buffer);
    
    // Add user ID
    user_id_labels[1] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[1], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[1], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[1], user_id_buffer);
    
    return scr;
}

// Validation failure screen
lv_obj_t *ui_screen_validation_failure(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Validation\nFailed", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    lv_obj_t *msg = lv_label_create(card);
    lv_obj_set_style_text_color(msg, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "User not found or\naccess denied");
    lv_obj_center(msg);
    
    // Add Try Again text with button
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create Try Again text first, then arrow icon
    // lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    // lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    
    return scr;
}

// Error screen - generic system error
lv_obj_t *ui_screen_error(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "System Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    error_label = lv_label_create(card);
    lv_obj_set_style_text_color(error_label, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(error_label, UI_FONT_SMALL, 0);
    lv_label_set_text(error_label, "An error occurred");
    lv_obj_center(error_label);
    
    // Add Try Again text with button
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // // Create Try Again text first, then arrow icon
    // lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    // lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    
    return scr;
}

// Admin enter ID screen
lv_obj_t *ui_screen_admin_enter_id(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add title with smaller font
    lv_obj_t *title = ui_create_title(cont, "Admin Mode", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Add admin icon (slightly smaller)
    lv_obj_t *icon = create_image(cont, &admin);
    // lv_obj_set_size(icon, 48, 48);
    
    // Create a card with instruction
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 170, 70);
    
    lv_obj_t *instr = lv_label_create(card);
    lv_obj_set_style_text_color(instr, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(instr, UI_FONT_SMALL, 0);
    lv_label_set_text(instr, "Enter student ID\nto program card");
    lv_obj_center(instr);
    
    return scr;
}

// Admin ID validating screen
lv_obj_t *ui_screen_admin_id_validating(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add title with smaller font
    lv_obj_t *title = ui_create_title(cont, "Validating ID", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Container for spinner and database icon
    lv_obj_t *center_container = lv_obj_create(cont);
    lv_obj_remove_style_all(center_container);
    lv_obj_set_size(center_container, 100, 80);
    lv_obj_set_style_pad_all(center_container, 0, 0);
    
    // Add spinner/loading indicator
    lv_obj_t *spinner = lv_spinner_create(center_container);
    lv_obj_set_size(spinner, 85, 85);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_ADMIN), LV_PART_MAIN);
    lv_obj_add_style(spinner, ui_get_style(UI_STYLE_PROGRESS, UI_STATE_ADMIN), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_center(spinner);
    
    // Add database icon in the middle of the spinner
    lv_obj_t *icon = create_image(center_container, &database);
    // lv_obj_set_size(icon, 32, 32); 
    lv_obj_center(icon);
    
    // Add message with ID being validated
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 170, 40);
    
    // Add ID text with smaller font
    user_id_labels[5] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[5], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[5], UI_FONT_SMALL, 0);
    lv_label_set_text(user_id_labels[5], user_id_buffer);
    lv_obj_center(user_id_labels[5]);
    
    return scr;
}

// More admin screens...

// Admin tap card screen
lv_obj_t *ui_screen_admin_tap_card(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ADMIN);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add title with smaller font
    lv_obj_t *title = ui_create_title(cont, "Tap Card", UI_STATE_ADMIN);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Add ID card icon (smaller)
    lv_obj_t *icon = create_image(cont, &id_card);
    // lv_obj_set_size(icon, 48, 48);
    
    // Create a card with user info
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ADMIN), 0);
    lv_obj_set_size(card, 170, 70);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add user name (if available) with smaller font
    user_name_labels[3] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[3], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[3], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[3], user_name_buffer);
    
    // Add user ID
    user_id_labels[3] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[3], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[3], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[3], user_id_buffer);
    
    return scr;
}

// Card write success screen
lv_obj_t *ui_screen_card_write_success(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_SUCCESS);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add success message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Card\nProgrammed", UI_STATE_SUCCESS);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with user info
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_SUCCESS), 0);
    lv_obj_set_size(card, 170, 70);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add user name (if available) with smaller font
    user_name_labels[4] = lv_label_create(card);
    lv_obj_set_style_text_color(user_name_labels[4], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_name_labels[4], UI_FONT_SMALL, 0);
    lv_label_set_text(user_name_labels[4], user_name_buffer);
    
    // Add user ID with smaller font
    user_id_labels[4] = lv_label_create(card);
    lv_obj_set_style_text_color(user_id_labels[4], UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(user_id_labels[4], UI_FONT_TINY, 0);
    lv_label_set_text(user_id_labels[4], user_id_buffer);
    
    return scr;
}

// ID Enter Error Screen
lv_obj_t *ui_screen_id_enter_error(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    // lv_obj_set_size(icon, 48, 48);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Invalid ID", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    lv_obj_t *msg = lv_label_create(card);
    lv_obj_set_style_text_color(msg, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "ID not found\nor inactive user.");
    lv_obj_center(msg);
    
    // Add retry button with arrow icon
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    // lv_obj_set_size(arrow_icon, 16, 16);
    
    lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    
    return scr;
}

// Card Write Error Screen
lv_obj_t *ui_screen_card_write_error(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    // lv_obj_set_size(icon, 48, 48);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Card Write\nFailed", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    lv_obj_t *msg = lv_label_create(card);
    lv_obj_set_style_text_color(msg, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Failed to write to card.\nCard may be damaged\nor unsupported.");
    lv_obj_center(msg);
    
    // Add retry button with arrow icon
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    // lv_obj_set_size(arrow_icon, 16, 16);
    
    // lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    
    return scr;
}

// Admin Error Screen
lv_obj_t *ui_screen_admin_error(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Add WiFi status icon at the top center (positioned to avoid cropping)
    // lv_obj_t *wifi_status = create_image(scr, &wifi);
    // lv_obj_set_size(wifi_status, 24, 24);
    // lv_obj_align(wifi_status, LV_ALIGN_TOP_MID, 0, 25);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    // lv_obj_set_size(icon, 48, 48);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Admin Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    lv_obj_t *msg = lv_label_create(card);
    lv_obj_set_style_text_color(msg, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "An error occurred in\nadmin mode. Please try\nagain.");
    lv_obj_center(msg);
    
    // Add retry button with arrow icon
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    // lv_obj_set_size(arrow_icon, 16, 16);
    
    // lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    
    return scr;
}

// Admin Card Write Error Screen
lv_obj_t *ui_screen_admin_card_write_error(void) {
    lv_obj_t *scr = ui_create_screen(UI_STATE_ERROR);
    
    // Create content container
    lv_obj_t *cont = ui_create_content_container(scr);
    
    // Add error icon (smaller size)
    lv_obj_t *icon = create_image(cont, &error);
    
    // Add error message with smaller font
    lv_obj_t *title = ui_create_title(cont, "Write Error", UI_STATE_ERROR);
    lv_obj_set_style_text_font(title, UI_FONT_MEDIUM, 0);
    
    // Create a card with error details
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, ui_get_style(UI_STYLE_CARD, UI_STATE_ERROR), 0);
    lv_obj_set_size(card, 170, 60);
    
    lv_obj_t *msg = lv_label_create(card);
    lv_obj_set_style_text_color(msg, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg, UI_FONT_SMALL, 0);
    lv_label_set_text(msg, "Failed to write to card.\nCard may be damaged\nor unsupported.");
    lv_obj_center(msg);
    
    // Add Try Again text with button
    lv_obj_t *btn_container = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, 170, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create Try Again text first, then arrow icon
    // lv_obj_t *btn = ui_create_button(btn_container, "Try Again", UI_STATE_ERROR);
    // lv_obj_t *arrow_icon = create_image(btn_container, &arrow);
    
    return scr;
}