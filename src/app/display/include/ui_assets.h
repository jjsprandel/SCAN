#ifndef UI_ASSETS_H
#define UI_ASSETS_H

#include "lvgl.h"

// Declare image descriptors for assets - use naming convention from the image files
LV_IMG_DECLARE(id_card_icon);  // From id-card-icon.c
LV_IMG_DECLARE(error_icon);    // From error-icon.c
LV_IMG_DECLARE(database_icon); // From database-icon.c
LV_IMG_DECLARE(admin_icon);    // From admin-icon.c
LV_IMG_DECLARE(keypad_icon);   // From keypad-icon.c
LV_IMG_DECLARE(write_id_icon); // From write-icon.c
LV_IMG_DECLARE(wifi_icon);     // From wifi-icon.c
LV_IMG_DECLARE(arrow_icon);    // From arrow-icon.c
LV_IMG_DECLARE(check_icon);    // From check-icon.c
LV_IMG_DECLARE(ucf_icon);      // From ucf-logo-icon.c
LV_IMG_DECLARE(gear_icon);     // From gear-icon.c
LV_IMG_DECLARE(software_icon); // From software-icon.c

// Map image variables to the expected names used in the UI code
#define id_card id_card_icon     // Used in ui_screen_user_detected
#define check check_icon         // Used in ui_screen_check_in_success
#define error error_icon         // Used in error screens
#define database database_icon   // Used in ui_screen_database_validation
#define admin admin_icon         // Used in admin screens
#define keypad keypad_icon       // Used in keypad screens
#define card_write write_id_icon // Used in card write screens
#define wifi wifi_icon           // Used for WiFi status
#define arrow arrow_icon         // Used for navigation
#define ucf_logo ucf_icon        // Used in ui_screen_idle
#define gear gear_icon           // Used in gear icon
#define software software_icon   // Used in software icon
#endif                           // UI_ASSETS_H