#include "display_utils.h"

lv_obj_t *screen_objects[STATE_ERROR + 1] = {NULL};
lv_obj_t *admin_screen_objects[ADMIN_STATE_ERROR + 1] = {NULL};
static const char *TAG = "DISPLAY";
void create_screens()
{
    screen_objects[STATE_HARDWARE_INIT] = ui_screen_hardware_init();
    screen_objects[STATE_WIFI_CONNECTING] = ui_screen_wifi_connecting();
    screen_objects[STATE_SOFTWARE_INIT] = ui_screen_software_init();
    screen_objects[STATE_SYSTEM_READY] = ui_screen_system_ready();
    screen_objects[STATE_IDLE] = ui_screen_idle();
    screen_objects[STATE_USER_DETECTED] = ui_screen_user_detected();
    screen_objects[STATE_DATABASE_VALIDATION] = ui_screen_database_validation();
    screen_objects[STATE_KEYPAD_ENTRY_ERROR] = ui_screen_keypad_entry_error();
    screen_objects[STATE_CHECK_IN] = ui_screen_check_in_success();
    screen_objects[STATE_CHECK_OUT] = ui_screen_check_out_success();
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

void display_screen(state_t display_state, admin_state_t display_admin_state)
{
    ESP_LOGI(TAG, "DISPLAY: Current state=%d, Current admin state=%d", display_state, display_admin_state);
    ESP_LOGI(TAG, "Beginning of display_screen. Free heap: %lu", esp_get_free_heap_size());
    if (display_state == STATE_ADMIN_MODE)
    {
        if (admin_screen_objects[display_admin_state] != NULL)
        {
            #ifdef DISPLAY_UTILS_DEBUG
            ESP_LOGI(TAG, "Displaying admin screen for state %d", display_admin_state);
            #endif
            if (user_info != NULL)
            {
                char full_name[64];
                snprintf(full_name, sizeof(full_name), "%s %s", user_info->first_name, user_info->last_name);
                
                // For TAP_CARD and CARD_WRITE_SUCCESS states, use user_id_to_write
                // For other states, use the current user_id
                const char *id_to_display;
                if (display_admin_state == ADMIN_STATE_VALIDATE_ID || 
                    display_admin_state == ADMIN_STATE_TAP_CARD || 
                    display_admin_state == ADMIN_STATE_CARD_WRITE_SUCCESS) {
                    id_to_display = user_id_to_write;
                    #ifdef DISPLAY_UTILS_DEBUG
                    ESP_LOGI(TAG, "Using user_id_to_write: %s", user_id_to_write);
                    #endif
                } else if (display_admin_state == ADMIN_STATE_ENTER_ID_ERROR) {
                    id_to_display = keypad_buffer.elements;
                    #ifdef DISPLAY_UTILS_DEBUG
                    ESP_LOGI(TAG, "Using keypad_buffer: %s", keypad_buffer.elements);
                    #endif
                } else {
                    id_to_display = user_id;
                    #ifdef DISPLAY_UTILS_DEBUG
                    ESP_LOGI(TAG, "Using user_id: %s", user_id);
                    #endif
                }
                #ifdef DISPLAY_UTILS_DEBUG
                ESP_LOGI(TAG, "Updating UI with name: %s, ID: %s", full_name, id_to_display);
                #endif
                ui_update_user_info(full_name, id_to_display);
            } else {
                ESP_LOGW(TAG, "user_info is NULL, cannot update UI");
            }

            _lock_acquire(&lvgl_api_lock);
            scan_ui_set_screen_transition(admin_screen_objects[display_admin_state]);
            _lock_release(&lvgl_api_lock);
        }
    }
    else
    {
        if (screen_objects[display_state] != NULL)
        {
            #ifdef DISPLAY_UTILS_DEBUG
            ESP_LOGI(TAG, "Displaying screen for state %d", display_state);
            #endif
                if (user_info != NULL)
                {
                    char full_name[64];
                    snprintf(full_name, sizeof(full_name), "%s %s", user_info->first_name, user_info->last_name);
                    ui_update_user_info(full_name, display_state == STATE_KEYPAD_ENTRY_ERROR ? keypad_buffer.elements : user_id);
                }

            _lock_acquire(&lvgl_api_lock);
            scan_ui_set_screen_transition(screen_objects[display_state]);
            _lock_release(&lvgl_api_lock);
        }
        else
        {
            #ifdef DISPLAY_UTILS_DEBUG
            ESP_LOGI(TAG, "Screen object not found for state %d", display_state);
            #endif
        }
    }
}

// void display_test_task(void *pvParameter)
// {
//     while (1)
//     {
//         // First cycle through main states
//         for (state_t test_state = STATE_HARDWARE_INIT; test_state <= STATE_ERROR; test_state++)
//         {
//             current_state = test_state;
//             if (screen_objects[current_state] != NULL)
//             {
//                 ESP_LOGI(TAG, "Testing state: %d", test_state);
//                 ui_update_user_info("Cory Brynds", "1234567890");
//                 _lock_acquire(&lvgl_api_lock);
//                 scan_ui_set_screen_transition(screen_objects[current_state]);
//                 _lock_release(&lvgl_api_lock);
//             }
//             else
//             {
//                 ESP_LOGI(TAG, "Screen does not exist for state %d", test_state);
//             }
//             vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
//         }

//         // Then cycle through admin states
//         for (admin_state_t test_admin_state = ADMIN_STATE_BEGIN; test_admin_state <= ADMIN_STATE_ERROR; test_admin_state++)
//         {
//             current_admin_state = test_admin_state;
//             if (admin_screen_objects[current_admin_state] != NULL)
//             {
//                 ESP_LOGI(TAG, "Testing admin state: %d", test_admin_state);
//                 ui_update_user_info("Dr. Mike", "0987654321");
//                 _lock_acquire(&lvgl_api_lock);
//                 scan_ui_set_screen_transition(admin_screen_objects[current_admin_state]);
//                 _lock_release(&lvgl_api_lock);
//             }
//             else
//             {
//                 ESP_LOGI(TAG, "Screen does not exist for admin state %d", current_admin_state);
//             }
//             vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
//         }

//         // Return to idle state
//         current_state = STATE_IDLE;
//         current_admin_state = ADMIN_STATE_BEGIN;
//     }
// }