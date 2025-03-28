#ifndef UI_STYLES_H
#define UI_STYLES_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Color palette
#define UI_COLOR_PRIMARY      lv_color_hex(0x3498db) // Blue
#define UI_COLOR_SUCCESS      lv_color_hex(0x2ecc71) // Green
#define UI_COLOR_WARNING      lv_color_hex(0xe67e22) // More muted orange (was 0xf39c12)
#define UI_COLOR_ERROR        lv_color_hex(0xe74c3c) // Red
#define UI_COLOR_NEUTRAL      lv_color_hex(0xecf0f1) // Light Gray
#define UI_COLOR_TEXT         lv_color_hex(0x2c3e50) // Dark Blue/Gray
#define UI_COLOR_TEXT_LIGHT   lv_color_hex(0xffffff) // White
#define UI_COLOR_BACKGROUND   lv_color_hex(0x34495e) // Dark Blue/Gray

// Font sizes
#define UI_FONT_LARGE         &lv_font_montserrat_32
#define UI_FONT_MEDIUM        &lv_font_montserrat_24
#define UI_FONT_SMALL         &lv_font_montserrat_16
#define UI_FONT_TINY          &lv_font_montserrat_14

// Animation times
#define UI_ANIM_TIME_FAST     300  // ms
#define UI_ANIM_TIME_NORMAL   500  // ms
#define UI_ANIM_TIME_SLOW     800  // ms

// Screen size constants
#define UI_SCREEN_WIDTH       240
#define UI_SCREEN_HEIGHT      240
#define UI_SCREEN_CENTER_X    (UI_SCREEN_WIDTH / 2)
#define UI_SCREEN_CENTER_Y    (UI_SCREEN_HEIGHT / 2)

// Common style types
typedef enum {
    UI_STYLE_BACKGROUND,
    UI_STYLE_BUTTON,
    UI_STYLE_TEXT,
    UI_STYLE_CARD,
    UI_STYLE_ICON,
    UI_STYLE_PROGRESS
} ui_style_type_t;

// Screen state types for design consistency
typedef enum {
    UI_STATE_NORMAL,
    UI_STATE_SUCCESS,
    UI_STATE_WARNING,
    UI_STATE_ERROR,
    UI_STATE_ADMIN
} ui_state_t;

// Initialize all UI styles
void ui_styles_init(void);

// Get a specific style based on type and state
lv_style_t* ui_get_style(ui_style_type_t type, ui_state_t state);

// Create a screen with appropriate background based on state
lv_obj_t* ui_create_screen(ui_state_t state);

// Create a centered container for content
lv_obj_t* ui_create_content_container(lv_obj_t* parent);

// Create a standard title label
lv_obj_t* ui_create_title(lv_obj_t* parent, const char* text, ui_state_t state);

// Create a standard icon
lv_obj_t* ui_create_icon(lv_obj_t* parent, const char* symbol, ui_state_t state);

// Start a pulsing animation on an object
void ui_start_pulse_animation(lv_obj_t* obj);

// Start a spinning animation on an object
void ui_start_spin_animation(lv_obj_t* obj);

// Create a standard progress indicator
lv_obj_t* ui_create_progress_indicator(lv_obj_t* parent, ui_state_t state);

// Create a standard button
lv_obj_t* ui_create_button(lv_obj_t* parent, const char* text, ui_state_t state);

// Set up screen transition animation
void ui_set_screen_transition(lv_obj_t* new_screen, bool slide_left);

#ifdef __cplusplus
}
#endif

#endif // UI_STYLES_H 