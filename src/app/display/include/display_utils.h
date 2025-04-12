#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include "lvgl.h"
#include "admin_mode.h"
#include "state_enum.h"
#include "firebase_utils.h"
#include "ui_screens.h"
#include "keypad.h"

#define DISPLAY_UTILS_DEBUG 1

extern lv_obj_t *screen_objects[STATE_ERROR + 1];
extern lv_obj_t *admin_screen_objects[ADMIN_STATE_ERROR + 1];
extern _lock_t lvgl_api_lock;
extern char user_id[ID_LEN + 1];
extern char user_id_to_write[ID_LEN+1];
extern keypad_buffer_t keypad_buffer;

void create_screens();
void display_screen(state_t display_state, admin_state_t display_admin_state);
void display_test_task(void *pvParameter);
#endif
