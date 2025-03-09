#include "main.h"

static state_t current_state = STATE_WIFI_INIT, prev_state = STATE_ERROR;

// Task Handles
static TaskHandle_t blink_led_task_handle = NULL;
static TaskHandle_t wifi_init_task_handle = NULL;
static TaskHandle_t ota_update_task_handle = NULL;
static TaskHandle_t database_task_handle = NULL;
static TaskHandle_t keypad_task_handle = NULL;
static TaskHandle_t lvgl_port_task_handle = NULL;
TaskHandle_t state_control_task_handle = NULL;

// not static because it is being used in wifi_init.c as extern variable
SemaphoreHandle_t wifi_init_semaphore = NULL; // Semaphore to signal Wi-Fi init completion

static const char *TAG = "MAIN";

static uint8_t s_led_state = 0;

static led_strip_handle_t led_strip;

void blink_led_task(void *pvParameter)
{
    while (1)
    {
        s_led_state = !s_led_state;

        if (current_state == STATE_WIFI_INIT)
        {
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay(1200 / portTICK_PERIOD_MS);
        }

        /* If the addressable LED is enabled */
        if (s_led_state)
        {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            if (current_state == STATE_WIFI_INIT)
            {
                led_strip_set_pixel(led_strip, 0, 100, 0, 0);
            }
            else
            {
                led_strip_set_pixel(led_strip, 0, 0, 0, 50);
            }
            /* Refresh the strip to send data */
            led_strip_refresh(led_strip);
        }
        else
        {
            /* Set all LED off to clear all pixels */
            led_strip_clear(led_strip);
        }
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

static void create_screens()
{
    screen_objects[STATE_IDLE] = display_idle(display);
    screen_objects[STATE_USER_DETECTED] = display_user_detected(display);
    screen_objects[STATE_CHECK_IN] = display_check_in_success(display);
    screen_objects[STATE_CHECK_OUT] = display_check_out_success(display);
    screen_objects[STATE_DATABASE_VALIDATION] = display_database_validation(display);
    screen_objects[STATE_VALIDATION_FAILURE] = display_check_in_failed(display);
}

static void display_screen(state_t display_state)
{
    if (screen_objects[current_state] != NULL)
    {
        ESP_LOGI(TAG, "Displaying screen for state %d", current_state);
        _lock_acquire(&lvgl_api_lock);
        lv_scr_load(screen_objects[current_state]);
        _lock_release(&lvgl_api_lock);
    }
    else
    {
        ESP_LOGE(TAG, "Screen object not found for state %d", current_state);
    }
}

// Function to control state transitions and task management
void state_control_task(void *pvParameter)
{
    char user_id[ID_LEN];
    while (1)
    {
        // ESP_LOGI("Memory", "Free heap size: %lu bytes", (unsigned long)esp_get_free_heap_size());

        switch (current_state)
        {
        case STATE_WIFI_INIT:
            // Start Wi-Fi init task if not already started
            if (wifi_init_task_handle == NULL)
            {
                ESP_LOGI(TAG, "Starting Wi-Fi Init Task");
                xTaskCreate(wifi_init_task, "wifi_init_task", 4096, NULL, 4, &wifi_init_task_handle);
            }

            // Start LED blinking task if not already running
            if (blink_led_task_handle == NULL)
            {
                ESP_LOGI(TAG, "Starting Blink LED Task");
                xTaskCreate(blink_led_task, "blink_led_task", 1024, NULL, 2, &blink_led_task_handle);
            }

            // Check if Wi-Fi init is completed (signaled by semaphore)
            if (xSemaphoreTake(wifi_init_semaphore, portMAX_DELAY) == pdTRUE)
            {
                current_state = STATE_WIFI_READY; // Transition state outside of the task
            }

            break;

        case STATE_WIFI_READY:
            // Stop Wi-Fi task when ready
            if (wifi_init_task_handle != NULL)
            {
                vTaskDelete(wifi_init_task_handle);
                wifi_init_task_handle = NULL;
            }
            // ESP_LOGI(TAG, "Wi-Fi Initialized. Ready!");

            // if (blink_led_task_handle != NULL) {
            //     vTaskDelete(blink_led_task_handle);
            //     blink_led_task_handle = NULL;
            // }
            /*
                            if (ota_update_task_handle == NULL) {
                                ESP_LOGI(TAG, "Creating OTA update task");
                                xTaskCreate(ota_update_fw_task, "OTA UPDATE TASK", 1024 * 4, NULL, 8, &ota_update_task_handle);
                            }
            */

            current_state = STATE_IDLE;
            break;
        case STATE_IDLE: // Wait until proximity is detected
            // Insert proximity sensor logic here
            current_state = STATE_USER_DETECTED;
            break;

        case STATE_USER_DETECTED: // Wait until NFC data is read or keypad press is entered
            char nfcUserID[ID_LEN];

            bool nfcReadFlag = false;

            BaseType_t keypadNotify = ulTaskNotifyTake(pdTRUE, 0);

            nfcReadFlag = read_user_id(nfcUserID, 50);

            if ((nfcReadFlag) || (keypadNotify > 0))
            {
#ifdef MAIN_DEBUG
                ESP_LOGI(TAG, "User ID entered by %s", nfcReadFlag ? "NFC Transceiver" : "Keypad");
#endif
                memcpy(user_id, nfcReadFlag ? nfcUserID : keypad_buffer.elements, ID_LEN);
                current_state = STATE_DATABASE_VALIDATION;
            }
            break;

        case STATE_DATABASE_VALIDATION: // Wait until validation is complete
            // If invalid user, set state to STATE_VALIDATION_FAILURE
            if (!get_user_info(user_id))
            {
                ESP_LOGE(TAG, "Invalid user detected");
                current_state = STATE_VALIDATION_FAILURE;
                break;
            }
            // If admin, set state to STATE_ADMIN_MODE
            else if (strcmp(user_info->role, "admin") == 0)
            {
                ESP_LOGI(TAG, "Entering Admin Mode");
                current_state = STATE_ADMIN_MODE;
                break;
            }
            // If student, check-in/out
            else if (strcmp(user_info->role, "student") == 0)
            {
                if (strcmp(user_info->check_in_status, "Checked In") == 0)
                    current_state = check_out_user(user_id) ? STATE_CHECK_OUT : STATE_VALIDATION_FAILURE;
                else
                    current_state = check_in_user(user_id) ? STATE_CHECK_IN : STATE_VALIDATION_FAILURE;
            }
            break;
        case STATE_CHECK_IN:
#ifdef MAIN_DEBUG
            ESP_LOGI(TAG, "ID %s found in database. Checking in.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;
        case STATE_CHECK_OUT:
#ifdef MAIN_DEBUG
            ESP_LOGI(TAG, "ID %s found in database. Checking out.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;
        case STATE_ADMIN_MODE:
            // Insert admin mode logic here
            current_state = STATE_IDLE;
            break;
        case STATE_VALIDATION_FAILURE:
#ifdef MAIN_DEBUG
            ESP_LOGI(TAG, "ID %s not found in database. Try again.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            current_state = STATE_USER_DETECTED;
            break;
        case STATE_ERROR:
            // Handle error state - for now just stopping all tasks
            if (wifi_init_task_handle != NULL)
            {
                vTaskDelete(wifi_init_task_handle);
                wifi_init_task_handle = NULL;
            }
            if (blink_led_task_handle != NULL)
            {
                vTaskDelete(blink_led_task_handle);
                blink_led_task_handle = NULL;
            }
            ESP_LOGE(TAG, "Error state reached!");
            break;

        default:
            ESP_LOGW(TAG, "Unknown state encountered: %d", current_state);
            break;
        }
        if (current_state != prev_state)
        {
            // display_screen(current_state);
            prev_state = current_state;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "State control task finished"); // Should not reach here unless task is deleted
}

void app_main(void)
{
    ESP_LOGI(TAG, "App Main Start");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ESP_ERROR_CHECK( heap_trace_init_standalone( trace_record, NUM_RECORDS ) );
    // ESP_LOGI("Memory", "STARTING FREE HEAP SIZE: %lu bytes", (long unsigned int)esp_get_free_heap_size());
    /* Configure the peripheral according to the LED type */
    i2c_master_init();
    configure_led();
    // gc9a01_init();
    nfc_init();

    // Create semaphore for signaling Wi-Fi init completion
    wifi_init_semaphore = xSemaphoreCreateBinary();
    // create_screens();
    xTaskCreate(state_control_task, "state_control_task", 4096 * 2, NULL, 5, &state_control_task_handle);

    xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 3, &keypad_task_handle);
    // xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);

#ifdef MAIN_DEBUG
    if (keypad_task_handle != NULL)
        ESP_LOGI(TAG, "Creating keypad task");
    else
        ESP_LOGE(TAG, "Failed to create keypad task");
#endif

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*

#define PROXIMITY_DETECTED BIT0
#define NFC_READ_SUCCESS BIT1
#define NFC_VALIDATED BIT2

static EventGroupHandle_t event_group;

typedef enum {
    STATE_IDLE,
    STATE_USER_DETECTED,
    STATE_READING_NFC,
    STATE_VALIDATING,
    STATE_DISPLAY_RESULT
} state_t;

static state_t current_state = STATE_IDLE;

// Task to handle proximity sensor
void proximity_task(void *param) {
    while (1) {
        if (proximity_sensor_detected()) {
            xEventGroupSetBits(event_group, PROXIMITY_DETECTED);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Check every 500 ms
    }
}

// Task to handle NFC reading
void nfc_task(void *param) {
    while (1) {
        xEventGroupWaitBits(event_group, PROXIMITY_DETECTED, pdTRUE, pdFALSE, portMAX_DELAY);
        if (nfc_read_data()) {
            xEventGroupSetBits(event_group, NFC_READ_SUCCESS);
        }
    }
}

// Task to validate NFC data online
void validation_task(void *param) {
    while (1) {
        xEventGroupWaitBits(event_group, NFC_READ_SUCCESS, pdTRUE, pdFALSE, portMAX_DELAY);
        bool valid = database_validate_nfc();
        if (valid) {
            xEventGroupSetBits(event_group, NFC_VALIDATED);
        }
        current_state = STATE_DISPLAY_RESULT;
    }
}

// Task to update the display based on validation result
void display_task(void *param) {
    while (1) {
        xEventGroupWaitBits(event_group, NFC_VALIDATED, pdTRUE, pdFALSE, portMAX_DELAY);
        display_show_result(true);  // Show success
        vTaskDelay(pdMS_TO_TICKS(5000));  // Display result for 5 seconds
        current_state = STATE_IDLE;
    }
}

// Main State Machine
void app_main(void) {
    // Initialize event group
    event_group = xEventGroupCreate();

    // Initialize hardware and network
    nfc_reader_init();
    display_init();
    wifi_init();
    sensor_init();

    // Create tasks
    xTaskCreate(proximity_task, "Proximity Task", 2048, NULL, 1, NULL);
    xTaskCreate(nfc_task, "NFC Task", 4096, NULL, 1, NULL);
    xTaskCreate(validation_task, "Validation Task", 4096, NULL, 1, NULL);
    xTaskCreate(display_task, "Display Task", 2048, NULL, 1, NULL);

    while (1) {
        switch (current_state) {
            case STATE_IDLE:
                // Wait until proximity is detected
                xEventGroupWaitBits(event_group, PROXIMITY_DETECTED, pdTRUE, pdFALSE, portMAX_DELAY);
                current_state = STATE_USER_DETECTED;
                break;

            case STATE_USER_DETECTED:
                // Wait until NFC data is read
                xEventGroupWaitBits(event_group, NFC_READ_SUCCESS, pdTRUE, pdFALSE, portMAX_DELAY);
                current_state = STATE_READING_NFC;
                break;

            case STATE_READING_NFC:
                // Move to validation state after NFC read
                current_state = STATE_VALIDATING;
                break;

            case STATE_VALIDATING:
                // Wait until NFC validation completes
                xEventGroupWaitBits(event_group, NFC_VALIDATED, pdTRUE, pdFALSE, portMAX_DELAY);
                current_state = STATE_DISPLAY_RESULT;
                break;

            case STATE_DISPLAY_RESULT:
                // Result is displayed; transition back to idle state after showing result
                current_state = STATE_IDLE;
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to prevent rapid state change
    }
}
*/