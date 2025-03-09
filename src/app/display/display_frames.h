#ifndef DISPLAY_FRAMES_H
#define DISPLAY_FRAMES_H
#include "lvgl.h"

lv_obj_t *display_idle();
lv_obj_t *display_user_detected();
lv_obj_t *display_check_in_failed();
lv_obj_t *display_check_in_success();
lv_obj_t *display_check_out_success();
lv_obj_t *display_database_validation();

#endif // DISPLAY_FRAMES_H