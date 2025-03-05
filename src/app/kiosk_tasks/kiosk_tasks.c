#include "kiosk_tasks.h"

bool nfcReadFlag = false;     // Flag set by nfc task
bool idIsValid = true;        // Flag set by database query results
bool isAdministrator = false; // Flag set by database query results
bool isCheckIn = true;        // Flag set by database query results
char user_id[ID_LEN];
char nfcUserID[MAX_ID_LEN];

state_t prev_state = STATE_WIFI_INIT;
static const char *TASK_TAG = "kiosk_tasks";

static TaskHandle_t proximity_task_handle = NULL;
static TaskHandle_t nfc_scan_id_task_handle = NULL;
static TaskHandle_t keypad_enter_id_task_handle = NULL;
static TaskHandle_t database_query_task_handle = NULL;
static TaskHandle_t display_task_handle = NULL;
static TaskHandle_t lvgl_port_task_handle = NULL;
static TaskHandle_t keypad_task_handle = NULL;
static TaskHandle_t cleanup_task_handle = NULL;

// Task to handle proximity sensor
static void proximity_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, IDLE_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (gpio_get_level(PIR_GPIO))
        {
            xEventGroupSetBits(event_group, ENTERING_ID_BIT);
            xEventGroupClearBits(event_group, IDLE_BIT);
#ifdef TASK_DEBUG
            ESP_LOGI(TASK_TAG, "Proximity Detected");
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Check every 500 ms
    }
}

// Task to handle NFC reading
static void nfc_scan_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ENTERING_ID_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

        nfcReadFlag = read_user_id(nfcUserID);

        if (nfcReadFlag)
        {
            // xEventGroupClearBits(event_group, ENTERING_ID_BIT);
            xEventGroupSetBits(event_group, ID_ENTERED_BIT);
#ifdef TASK_DEBUG
            ESP_LOGI(TASK_TAG, "NFC Read Success. ID is: %s", nfcUserID);
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void keypad_enter_id_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, ENTERING_ID_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (keypad_task_handle == NULL)
        {
#ifdef TASK_DEBUG
            ESP_LOGI(TASK_TAG, "Starting Keypad Task");
#endif
            xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 1, &keypad_task_handle);
        }

        xEventGroupWaitBits(event_group, ID_ENTERED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if (!nfcReadFlag)
        {
#ifdef TASK_DEBUG
            ESP_LOGI(TASK_TAG, "Keypad Read Success. ID is: %s", keypad_buffer.elements);
#endif
            // xEventGroupClearBits(event_group, ENTERING_ID_BIT);
        }
        if (keypad_task_handle != NULL)
        {
#ifdef TASK_DEBUG
            ESP_LOGI(TASK_TAG, "Deleting Keypad Task");
#endif
            vTaskDelete(keypad_task_handle);
            keypad_task_handle = NULL;
        }
        xEventGroupSetBits(event_group, DATABASE_QUERY_BIT);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void database_query_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, DATABASE_QUERY_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        memcpy(user_id, nfcReadFlag ? nfcUserID : keypad_buffer.elements, ID_LEN);
#ifdef TASK_DEBUG
        ESP_LOGI(TASK_TAG, "Querying database for ID: %s", user_id);
#endif

#ifdef DATABASE_ENABLED
        // Database will return: valid ID, administrator state, and check-in/out state
        // UserInfo info = database_query(nfcUserID);
        // idIsValid = info.idIsValid;
        // isAdministrator = info.isAdministrator;
        // isCheckIn = info.isCheckIn;
#else
        // Hard code valuse for testing
        idIsValid = true;
        isAdministrator = false;
        isCheckIn = true;
#endif
        xEventGroupSetBits(event_group, ID_AUTHENTICATED_BIT);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

static void cleanup_task(void *param)
{
    while (1)
    {
        xEventGroupWaitBits(event_group, CLEAN_UP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        keypadEntered = false;
        nfcReadFlag = false;
        memcpy(nfcUserID, "0000000", ID_LEN);
        memcpy(user_id, "0000000", ID_LEN);
        clear_buffer();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
// Task to update the display
static void display_task(void *param)
{
    while (1)
    {
        if (current_state != prev_state)
        {
            if (disp_obj != NULL)
                lv_obj_delete(disp_obj);
            switch (current_state)
            {
            case STATE_IDLE:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying idle state screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_USER_DETECTED:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying user detected screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_DATABASE_VALIDATION:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying database validation screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_transmitting(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_CHECK_IN:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying check-in success screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_in_success(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_CHECK_OUT:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying check-out success screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_out_success(display);
                _lock_release(&lvgl_api_lock);
                break;
            case STATE_VALIDATION_FAILURE:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying validation failure screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_check_in_failed(display);
                _lock_release(&lvgl_api_lock);
                break;
            default:
#ifdef TASK_DEBUG
                ESP_LOGI(TASK_TAG, "Displaying default screen");
#endif
                _lock_acquire(&lvgl_api_lock);
                disp_obj = display_idle(display);
                _lock_release(&lvgl_api_lock);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        prev_state = current_state;
    }
}

// Initialize all kiosk tasks
void init_kiosk_tasks()
{
    xTaskCreate(proximity_task, "Proximity Task", 2048, NULL, 1, &proximity_task_handle);
    xTaskCreate(nfc_scan_id_task, "nfc_scan_id_task", 4096, NULL, 1, &nfc_scan_id_task_handle);
    xTaskCreate(keypad_enter_id_task, "keypad_enter_id_task", 4096, NULL, 1, &keypad_enter_id_task_handle);
    xTaskCreate(database_query_task, "Database validation task", 4096, NULL, 1, &database_query_task_handle);
    xTaskCreate(display_task, "Display Task", 2048, NULL, 1, &display_task_handle);
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);
    xTaskCreate(cleanup_task, "Cleanup Task", 2048, NULL, 1, &cleanup_task_handle);
}

// Clean up tasks
void teardown_kiosk_tasks()
{
    if (proximity_task_handle != NULL)
    {
        vTaskDelete(proximity_task_handle);
        proximity_task_handle = NULL;
    }
    if (nfc_scan_id_task_handle != NULL)
    {
        vTaskDelete(nfc_scan_id_task_handle);
        nfc_scan_id_task_handle = NULL;
    }
    if (keypad_enter_id_task_handle != NULL)
    {
        vTaskDelete(keypad_enter_id_task_handle);
        keypad_enter_id_task_handle = NULL;
    }
    if (database_query_task_handle != NULL)
    {
        vTaskDelete(database_query_task_handle);
        database_query_task_handle = NULL;
    }
    if (display_task_handle != NULL)
    {
        vTaskDelete(display_task_handle);
        display_task_handle = NULL;
    }
    if (lvgl_port_task_handle != NULL)
    {
        vTaskDelete(lvgl_port_task_handle);
        lvgl_port_task_handle = NULL;
    }
    if (cleanup_task_handle != NULL)
    {
        vTaskDelete(cleanup_task_handle);
        cleanup_task_handle = NULL;
    }
}