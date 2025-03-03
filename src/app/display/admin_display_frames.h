#ifndef ADMIN_DISPLAY_FRAMES_H
#define ADMIN_DISPLAY_FRAMES_H
#include "lvgl.h"

lv_obj_t *display_admin_enter_id(lv_display_t *disp);
lv_obj_t *display_admin_id_validating(lv_display_t *disp);
lv_obj_t *display_admin_tap_card(lv_display_t *disp);
lv_obj_t *display_card_write_success(lv_display_t *disp);
lv_obj_t *display_id_enter_error(lv_display_t *disp);
lv_obj_t *display_card_write_error(lv_display_t *disp);
lv_obj_t *display_admin_error(lv_display_t *disp);

#endif // DISPLAY_FRAMES_H