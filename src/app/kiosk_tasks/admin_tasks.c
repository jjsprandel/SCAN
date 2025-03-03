#include "admin_tasks.h"

static admin_state_t admin_state = ADMIN_STATE_BEGIN, prev_admin_state;
static EventGroupHandle_t admin_event_group;
static const char *ADMIN_TAG = "ADMIN_TASKS";
static TaskHandle_t admin_keypad_handler_task_handle = NULL;
static bool adminWriteSuccess = false;
static bool adminValidationSuccess = false;

static TaskHandle_t admin_mode_control_task_handle = NULL;
static TaskHandle_t admin_keypad_task_handle = NULL;
static TaskHandle_t admin_validate_id_task_handle = NULL;
static TaskHandle_t admin_nfc_task_handle = NULL;
static TaskHandle_t admin_display_task_handle = NULL;

static void admin_mode_control_task(void *param)
{
    while (1)
    {
        switch (admin_state)
        {
        case ADMIN_STATE_BEGIN:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Enter a valid ID # to write to the card");
#endif
            xEventGroupSetBits(admin_event_group, ADMIN_READY_BIT);
            admin_state = ADMIN_STATE_ENTER_ID;
            break;

        case ADMIN_STATE_ENTER_ID:
            xEventGroupWaitBits(admin_event_group, ADMIN_ID_ENTERED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
            admin_state = ADMIN_STATE_VALIDATE_ID;
            break;

        case ADMIN_STATE_VALIDATE_ID:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Sending ID to database for validation");
#endif
            xEventGroupWaitBits(admin_event_group, ADMIN_VALIDATION_RESPONSE_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            admin_state = adminValidationSuccess ? ADMIN_STATE_TAP_CARD : ADMIN_STATE_ENTER_ID_ERROR;
            break;

        case ADMIN_STATE_TAP_CARD:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Tap a blank NTAG213 card for writing");
#endif
            xEventGroupSetBits(admin_event_group, ADMIN_TAP_CARD_BIT);
            xEventGroupWaitBits(admin_event_group, ADMIN_CARD_WRITTEN_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            admin_state = adminWriteSuccess ? ADMIN_STATE_CARD_WRITE_SUCCESS : ADMIN_STATE_CARD_WRITE_ERROR; // May just need to be straight to ADMIN_STATE_CARD_WRITE_SUCCESS
            break;

        case ADMIN_STATE_CARD_WRITE_SUCCESS:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "ID Successfully Written to Card");
#endif
            vTaskDelay(pdMS_TO_TICKS(2000));
            xEventGroupSetBits(event_group, EXIT_ADMIN_MODE_BIT);
            vTaskDelete(NULL);
            break;
        case ADMIN_STATE_CARD_WRITE_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error writing ID to card. Try again.");
#endif
            vTaskDelay(pdMS_TO_TICKS(2000));
            admin_state = ADMIN_STATE_TAP_CARD;
            break;
        case ADMIN_STATE_ENTER_ID_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error validating ID in database. Try again.");
#endif
            vTaskDelay(pdMS_TO_TICKS(2000));
            admin_state = ADMIN_STATE_ENTER_ID;
            break;
        case ADMIN_STATE_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error encountered in Admin Mode. Returning to STATE_ADMIN_BEGIN");
#endif
            vTaskDelay(pdMS_TO_TICKS(2000));
            admin_state = ADMIN_STATE_BEGIN;
            break;

        default:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Unknown state encountered: %d. Exiting admin mode.", admin_state);
#endif
            vTaskDelay(pdMS_TO_TICKS(5000));
            vTaskDelete(NULL);
            break;
        }
    }
}

static void admin_keypad_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(admin_event_group, ADMIN_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (admin_keypad_handler_task_handle == NULL)
        {
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Starting Admin Keypad Task");
#endif
            xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 1, &admin_keypad_handler_task_handle);
        }

        xEventGroupWaitBits(event_group, ID_ENTERED_SUCCESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        xEventGroupSetBits(admin_event_group, ADMIN_ID_ENTERED_BIT);

#ifdef ADMIN_DEBUG
        ESP_LOGI(ADMIN_TAG, "Keypad Read Success. ID is: %s", keypad_buffer.elements);
#endif

        if (admin_keypad_task_handle != NULL)
        {
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Deleting Keypad Task");
#endif
            vTaskDelete(admin_keypad_handler_task_handle);
            admin_keypad_handler_task_handle = NULL;
        }
    }
}

static void admin_validate_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(admin_event_group, ADMIN_ID_ENTERED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
#ifdef DATABASE_ENABLED
        adminValidationSuccess = check_in_user(keypad_buffer.elements);
#else
        adminValidationSuccess = true;
#endif
        xEventGroupSetBits(admin_event_group, ADMIN_VALIDATION_RESPONSE_BIT);
    }
}

static void admin_nfc_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(admin_event_group, ADMIN_TAP_CARD_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        adminWriteSuccess = write_user_id(nfcUserID);

        if (adminWriteSuccess)
        {
            xEventGroupSetBits(admin_event_group, ADMIN_CARD_WRITTEN_BIT);
            xEventGroupClearBits(admin_event_group, ADMIN_TAP_CARD_BIT);
        }
    }
}

static void admin_display_task(void *param)
{
    while (1)
    {
        if (admin_state != prev_admin_state)
        {
            if (disp_obj != NULL)
                lv_obj_delete(disp_obj);
            switch (admin_state)
            {
            case ADMIN_STATE_BEGIN:
            case ADMIN_STATE_ENTER_ID:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state enter ID screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_admin_enter_id(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_VALIDATE_ID:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state validating ID screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_admin_id_validating(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_TAP_CARD:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state tap card screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_admin_tap_card(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_CARD_WRITE_SUCCESS:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state card write success screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_card_write_success(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_ENTER_ID_ERROR:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state enter ID error screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_id_enter_error(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_CARD_WRITE_ERROR:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state card write error screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_card_write_error(display);
                _lock_release(&lvgl_api_lock);
                break;
            case ADMIN_STATE_ERROR:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying admin state general error screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_admin_error(display);
                _lock_release(&lvgl_api_lock);
                break;
            }
            prev_admin_state = admin_state;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void init_admin_mode()
{
    admin_event_group = xEventGroupCreate();
    xTaskCreate(admin_mode_control_task, "admin_mode_control_task", 4096, NULL, 1, &admin_mode_control_task_handle);
    xTaskCreate(admin_keypad_task, "admin_keypad_task", 4096, NULL, 1, &admin_keypad_task_handle);
    xTaskCreate(admin_validate_id_task, "admin_validate_id_task", 4096, NULL, 1, &admin_validate_id_task_handle);
    xTaskCreate(admin_nfc_task, "admin_nfc_task", 4096, NULL, 1, &admin_nfc_task_handle);
    xTaskCreate(admin_display_task, "admin_display_task", 4096, NULL, 1, &admin_display_task_handle);
}

void teardown_admin_mode()
{
    if (admin_mode_control_task_handle != NULL)
    {
        vTaskDelete(admin_mode_control_task_handle);
        admin_mode_control_task_handle = NULL;
    }
    if (admin_keypad_task_handle != NULL)
    {
        vTaskDelete(admin_keypad_task_handle);
        admin_keypad_task_handle = NULL;
    }
    if (admin_keypad_handler_task_handle != NULL)
    {
        vTaskDelete(admin_keypad_handler_task_handle);
        admin_keypad_handler_task_handle = NULL;
    }
    if (admin_validate_id_task_handle != NULL)
    {
        vTaskDelete(admin_validate_id_task_handle);
        admin_validate_id_task_handle = NULL;
    }
    if (admin_nfc_task_handle != NULL)
    {
        vTaskDelete(admin_nfc_task_handle);
        admin_nfc_task_handle = NULL;
    }
    if (admin_display_task_handle != NULL)
    {
        vTaskDelete(admin_display_task_handle);
        admin_display_task_handle = NULL;
    }
    if (admin_event_group != NULL)
    {
        vEventGroupDelete(admin_event_group);
        admin_event_group = NULL;
    }
}