#ifndef ADMIN_FRAMES_H
#define ADMIN_FRAMES_H

#include "lvgl.h"

lv_obj_t *display_admin_enter_id();
lv_obj_t *display_admin_id_validating();
lv_obj_t *display_admin_tap_card();
lv_obj_t *display_card_write_success();
lv_obj_t *display_id_enter_error();
lv_obj_t *display_card_write_error();
lv_obj_t *display_admin_error();

#endif // ADMIN_FRAMES_H