# SCAN Kiosk UI Integration Guide

This document explains how to integrate the new UI system into the SCAN kiosk project.

## Overview

The new UI system provides modernized, aesthetically pleasing screens with animations and improved visual feedback. It consists of several components:

1. **UI Styles** - Common styling for all UI elements
2. **UI Screens** - Implementation of each screen in the kiosk
3. **UI Symbols** - Temporary symbols until proper icons/images are available

## Required Assets

The UI design uses default LVGL symbols as placeholders, but for a more polished look, we should create or obtain the following assets:

- `id_card.png` - ID card icon (64x64px)
- `check.png` - Checkmark icon (64x64px)
- `error.png` - Error/warning icon (64x64px)
- `database.png` - Database icon (64x64px)
- `admin.png` - Admin icon (64x64px)
- `keypad.png` - Keypad icon (64x64px)
- `card_write.png` - Card icon (64x64px)
- `wifi.png` - WiFi icon (64x64px)
- `ucf_logo.png` - UCF logo (128x64px)

All icons should be PNG format with transparency. Once available, place them in a new `assets` directory.

## Integration Steps

### 1. Update CMakeLists.txt

The CMakeLists.txt file has already been updated to include the new UI components.

### 2. Initialize the UI System

In `app_main()` (main.c), add the UI initialization before creating screens:

```c
// After other initializations but before creating screens
ui_init();
```

### 3. Replace Screen Creation

Replace the current screen creation in `create_screens()` function:

```c
static void create_screens()
{
    // Replace old display functions with new UI functions
    screen_objects[STATE_IDLE] = ui_screen_idle();
    screen_objects[STATE_USER_DETECTED] = ui_screen_user_detected();
    screen_objects[STATE_CHECK_IN] = ui_screen_check_in_success();
    screen_objects[STATE_CHECK_OUT] = ui_screen_check_out_success();
    screen_objects[STATE_DATABASE_VALIDATION] = ui_screen_database_validation();
    screen_objects[STATE_VALIDATION_FAILURE] = ui_screen_validation_failure();
    screen_objects[STATE_ERROR] = ui_screen_error();

    admin_screen_objects[ADMIN_STATE_ENTER_ID] = ui_screen_admin_enter_id();
    admin_screen_objects[ADMIN_STATE_VALIDATE_ID] = ui_screen_admin_id_validating();
    admin_screen_objects[ADMIN_STATE_TAP_CARD] = ui_screen_admin_tap_card();
    admin_screen_objects[ADMIN_STATE_CARD_WRITE_SUCCESS] = ui_screen_card_write_success();
    admin_screen_objects[ADMIN_STATE_ENTER_ID_ERROR] = ui_screen_id_enter_error();
    admin_screen_objects[ADMIN_STATE_CARD_WRITE_ERROR] = ui_screen_card_write_error();
    admin_screen_objects[ADMIN_STATE_ERROR] = ui_screen_admin_error();
}
```

### 4. Update Headers

Include the new UI header in `main.h`:

```c
#include "ui_screens.h"
```

### 5. Add Animation for Screen Transitions

In `display_screen()` function, add animation for screen transitions:

```c
static void display_screen(state_t display_state)
{
    if (current_state != STATE_ADMIN_MODE)
    {
        if (screen_objects[current_state] != NULL)
        {
            MAIN_DEBUG_LOG("Displaying screen for state %d", current_state);
            _lock_acquire(&lvgl_api_lock);
            // Add animation when changing screens
            ui_set_screen_transition(screen_objects[current_state], current_state > prev_state);
            _lock_release(&lvgl_api_lock);
        }
        else
        {
            MAIN_ERROR_LOG("Screen object not found for state %d", current_state);
        }
    }
    else
    {
        if (admin_screen_objects[current_admin_state] != NULL)
        {
            _lock_acquire(&lvgl_api_lock);
            // Add animation when changing admin screens
            ui_set_screen_transition(admin_screen_objects[current_admin_state], current_admin_state > prev_admin_state);
            _lock_release(&lvgl_api_lock);
        }
    }
}
```

### 6. Adding User Information to Screens

To display user information on the check-in/out screens, add a function to update user info:

```c
// Add this function to ui_screens.h
void ui_update_user_info(const char* name, const char* id);

// Implement it in ui_screens.c to update the displayed user information
```

## Next Steps

1. Complete implementation of all screens in ui_screens.c
2. Create or obtain proper icon assets
3. Integrate with user data from the database
4. Refine animations and transitions
5. Add support for multiple languages if needed

## Troubleshooting

- If screens appear blank, ensure `ui_init()` is called before creating screens
- If elements appear incorrectly positioned, check the screen dimension constants in ui_styles.h
- For animation issues, verify that LVGL tick timer is configured correctly 