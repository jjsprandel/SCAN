#ifndef DISPLAY_FRAMES_H
#define DISPLAY_FRAMES_H
#include "lvgl.h"

lv_obj_t *display_idle(lv_display_t *disp);
lv_obj_t *display_check_in_failed(lv_display_t *disp);
lv_obj_t *display_check_in_success(lv_display_t *disp);
lv_obj_t *display_transmitting(lv_display_t *disp);

#endif // DISPLAY_FRAMES_H