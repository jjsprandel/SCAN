#include "kiosk_tasks.h"

bool idIsValid = true;        // Flag set by database query results
bool isAdministrator = false; // Flag set by database query results
bool keypadEnteredFlag = false;
bool nfcReadFlag = false;
state_t prev_state = STATE_WIFI_INIT;
static TaskHandle_t database_task_handle = NULL;
static const char *TASK_TAG = "kiosk_tasks";

// Task to handle proximity sensor
void proximity_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, IDLE_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (gpio_get_level(PIR_GPIO))
        {
            xEventGroupSetBits(event_group, ENTERING_ID_BIT);
            xEventGroupClearBits(event_group, IDLE_BIT);
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "Proximity Detected");
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Check every 500 ms
    }
}

// // Task to handle NFC reading
void nfc_scan_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ENTERING_ID_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        bool readIdSuccess = read_user_id(nfcUserID);

        if (readIdSuccess)
        {
            xEventGroupClearBits(event_group, ENTERING_ID_BIT);
            xEventGroupSetBits(event_group, ID_ENTERED_SUCCESS_BIT);
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "NFC Read Success. ID is: %s", nfcUserID);
#endif
            nfcReadFlag = true;
        }
    }
}

void keypad_enter_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ENTERING_ID_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (keypad_task_handle == NULL)
        {
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "Starting Keypad Task");
#endif
            xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 1, &keypad_task_handle);
        }

        xEventGroupClearBits(event_group, ENTERING_ID_BIT);
        xEventGroupWaitBits(event_group, ID_ENTERED_SUCCESS_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

#ifdef MAIN_DEBUG
        if (!nfcReadFlag)
            ESP_LOGI(TASK_TAG, "Keypad Read Success. ID is: %s", keypad_buffer.elements);
#endif

        if (keypad_task_handle != NULL)
        {
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "Deleting Keypad Task");
#endif
            vTaskDelete(keypad_task_handle);
            keypad_task_handle = NULL;
        }
    }
}

// Task to validate NFC data online
void validation_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ID_ENTERED_SUCCESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        // Database will return: valid ID, check In status, and administrator state
        // idIsValid = check_in_user(nfcUserID);
        idIsValid = true;
        isAdministrator = false;

        if (database_task_handle == NULL)
        {
            ESP_LOGI(TASK_TAG, "Creating check-in task for ID: %s", nfcUserID);
            xTaskCreate(check_in_user_task, "CHECK IN TASK", 1024 * 12, (void *)nfcUserID, 8, &database_task_handle);
            database_task_handle = NULL;
        }

        //         if (idIsValid)
        //         {
        //             if (isAdministrator)
        //             {
        //                 xEventGroupSetBits(event_group, ADMIN_MODE_BIT);
        //                 xEventGroupWaitBits(event_group, NEW_ID_ENTERED_SUCCESS_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        //             }
        //             else
        //             {
        //                 // xEventGroupSetBits(event_group, ID_AUTHENTICATED_BIT);
        // #ifdef MAIN_DEBUG
        //                 ESP_LOGI(TASK_TAG, "ID %s found in database. ID accepted.", !nfcReadFlag ? keypad_buffer.elements : nfcUserID);
        // #endif
        //             }
        //             nfcReadFlag = false;
        //         }
        //         else
        //         {
        //             keypadEnteredFlag = false;
        // #ifdef MAIN_DEBUG
        //             ESP_LOGI(TASK_TAG, "ID %s not found in database. ID denied.", !nfcReadFlag ? keypad_buffer.elements : nfcUserID);
        // #endif
        //         }

        //         clear_buffer();
    }
}

void keypad_enter_new_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ADMIN_MODE_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (keypad_task_handle == NULL)
        {
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "Starting Keypad Task");
#endif
            xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 1, &keypad_task_handle);
        }

        xEventGroupClearBits(event_group, ADMIN_MODE_BIT);
        xEventGroupWaitBits(event_group, NEW_ID_ENTERED_SUCCESS_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

        if (sizeof(keypad_buffer.elements) / sizeof(char) == ID_LEN)
        {
            // idIsValid = databaseQuery(); // Transmit to database
            if (idIsValid)
            {
            }
        }
#ifdef MAIN_DEBUG
        ESP_LOGI(TASK_TAG, "Keypad Read Success. New ID is: %s", keypad_buffer.elements);
#endif

        if (keypad_task_handle != NULL)
        {
#ifdef MAIN_DEBUG
            ESP_LOGI(TASK_TAG, "Deleting Keypad Task");
#endif
            vTaskDelete(keypad_task_handle);
            keypad_task_handle = NULL;
        }
    }
}

// Task to update the display
void display_task(void *param)
{
    // lv_obj_t *new_scr = NULL;
    while (1)
    {
        if (current_state != prev_state)
        {
            if (disp_obj != NULL)
                lv_obj_delete(disp_obj);
            switch (current_state)
            {
            case STATE_IDLE:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_USER_DETECTED:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_VALIDATING:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_transmitting(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_VALIDATION_SUCCESS:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_in_success(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_VALIDATION_FAILURE:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_in_failed(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_ADMIN:
#ifdef MAIN_DEBUG
                ESP_LOGI(TASK_TAG, "Entering admin mode");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_in_success(display);
                _lock_release(&lvgl_api_lock);
                break;
            default:
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            }
        }
        prev_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}