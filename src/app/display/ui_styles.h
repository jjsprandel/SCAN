#ifndef UI_STYLES_H
#define UI_STYLES_H

#include "lvgl.h"

#define UI_COLOR_ORANGE lv_color_hex(0xF39C12)     // Orange
#define UI_COLOR_ORANGE_DARK lv_color_hex(0xD35400) // Dark Orange
#define UI_COLOR_GREEN lv_color_hex(0x27AE60)      // Green
#define UI_COLOR_GREEN_DARK lv_color_hex(0x196F3D) // Dark Green
#define UI_COLOR_RED lv_color_hex(0xE74C3C)        // Red
#define UI_COLOR_RED_DARK lv_color_hex(0xB03A2E)   // Dark Red
#define UI_COLOR_PURPLE lv_color_hex(0x8E44AD)     // Purple
#define UI_COLOR_PURPLE_DARK lv_color_hex(0x6C3483) // Dark Purple
#define UI_COLOR_BLUE lv_color_hex(0x3498DB)       // Blue
#define UI_COLOR_BLUE_DARK lv_color_hex(0x2874A6)  // Dark Blue
#define UI_COLOR_BLACK lv_color_hex(0x000000)
#define UI_COLOR_WHITE lv_color_hex(0xFFFFFF)

// Screen size constants
#define UI_SCREEN_WIDTH 240
#define UI_SCREEN_HEIGHT 240
#define UI_SCREEN_CENTER_X (UI_SCREEN_WIDTH / 2)
#define UI_SCREEN_CENTER_Y (UI_SCREEN_HEIGHT / 2)

// Font definitions
#define UI_FONT_LARGE &lv_font_montserrat_32
#define UI_FONT_MEDIUM &lv_font_montserrat_24
#define UI_FONT_SMALL &lv_font_montserrat_16
#define UI_FONT_TINY &lv_font_montserrat_14

void ui_styles_init(void);

// Screen creation and styling functions
lv_obj_t *scan_ui_create_screen(void);
lv_obj_t *scan_ui_create_content_container(lv_obj_t *parent);
lv_obj_t *scan_ui_create_title(lv_obj_t *parent, const char *text, lv_color_t text_color, const lv_font_t *font);
void scan_ui_set_background_color(lv_obj_t *scr, lv_color_t color1, lv_color_t color2);
void scan_ui_create_spinner(lv_obj_t *spinner, int diameter, lv_color_t color);
void scan_ui_style_card(lv_obj_t *card, lv_color_t bg_color, uint8_t radius, uint8_t padding);
void scan_ui_style_text(lv_obj_t *text, lv_color_t color, const lv_font_t *font, lv_text_align_t align);
void scan_ui_set_screen_transition(lv_obj_t *new_screen);
lv_obj_t *create_image(lv_obj_t *parent, const void *src);

#endif // UI_STYLES_H