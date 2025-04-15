#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "lvgl.h"
#include "ui_styles.h"
#include "ui_assets.h"
#include "state_enum.h"
#include "admin_mode.h"
#include "string.h"
//#define UI_DEBUG 1

#define NUM_SCREENS (STATE_ERROR + ADMIN_STATE_ERROR + 2)
// Function declarations
void ui_init(void);
void ui_update_user_info(const char *name, const char *id);

// Screen creation functions
lv_obj_t *ui_screen_hardware_init(void);
lv_obj_t *ui_screen_wifi_connecting(void);
lv_obj_t *ui_screen_software_init(void);
lv_obj_t *ui_screen_system_ready(void);
lv_obj_t *ui_screen_idle(void);
lv_obj_t *ui_screen_user_detected(void);
lv_obj_t *ui_screen_database_validation(void);
lv_obj_t *ui_screen_keypad_entry_error(void);
lv_obj_t *ui_screen_check_in_success(void);
lv_obj_t *ui_screen_check_out_success(void);
lv_obj_t *ui_screen_validation_failure(void);
lv_obj_t *ui_screen_error(void);
lv_obj_t *ui_screen_admin_enter_id(void);
lv_obj_t *ui_screen_admin_id_validating(void);
lv_obj_t *ui_screen_admin_tap_card(void);
lv_obj_t *ui_screen_card_write_success(void);
lv_obj_t *ui_screen_id_enter_error(void);
lv_obj_t *ui_screen_card_write_error(void);
lv_obj_t *ui_screen_admin_error(void);
lv_obj_t *ui_screen_admin_card_write_error(void);
extern _lock_t lvgl_api_lock;

#endif // UI_SCREENS_H