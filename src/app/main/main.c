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

        // if (current_state == STATE_WIFI_INIT)
        // {
        //     vTaskDelay(200 / portTICK_PERIOD_MS);
        // }
        // else
        // {
        //     vTaskDelay(1200 / portTICK_PERIOD_MS);
        // }

        /* If the addressable LED is enabled */
        if (s_led_state && current_state == STATE_WIFI_INIT)
        {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            // if (current_state == STATE_WIFI_INIT)
            // {
            for (int i = 0; i < NUM_LEDS; i++)
            {
                led_strip_set_pixel(led_strip, i, 100, 0, 0);
            }
            // }
            // else
            // {
            //     for (int i = 0; i < NUM_LEDS; i++)
            //     {
            //         led_strip_set_pixel(led_strip, i, 0, 0, 50);
            //     }
            // }
            /* Refresh the strip to send data */
            led_strip_refresh(led_strip);
        }
        else
        {
            /* Set all LED off to clear all pixels */
            led_strip_clear(led_strip);
        }
        if (current_state == STATE_USER_DETECTED)
        {
            led_strip_set_pixel(led_strip, 0, 0, 0, 100);
        }
        else if (current_state == STATE_DATABASE_VALIDATION)
        {
            led_strip_set_pixel(led_strip, 0, 0, 0, 100);
            led_strip_set_pixel(led_strip, 1, 100, 100, 0);
        }
        else if (current_state == STATE_CHECK_IN || current_state == STATE_CHECK_OUT)
        {
            led_strip_set_pixel(led_strip, 0, 0, 0, 100);
            led_strip_set_pixel(led_strip, 1, 100, 100, 0);
            led_strip_set_pixel(led_strip, 2, 0, 100, 0);
        }
        else if (current_state == STATE_VALIDATION_FAILURE)
        {
            led_strip_set_pixel(led_strip, 0, 0, 0, 100);
            led_strip_set_pixel(led_strip, 1, 100, 100, 0);
            led_strip_set_pixel(led_strip, 2, 100, 0, 0);
        }
        else if (current_state == STATE_IDLE)
        {
            led_strip_clear(led_strip);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 3, // at least one LED on board
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

static void check_task_creation()
{
    if (state_control_task_handle == NULL)
    {
        ESP_LOGE(TAG, "State Control Task creation failed!");
    }
    else
    {
        ESP_LOGI(TAG, "State Control Task created successfully!");
    }
    if (keypad_task_handle == NULL)
    {
        ESP_LOGE(TAG, "Keypad Task creation failed!");
    }
    else
    {
        ESP_LOGI(TAG, "Keypad Task created successfully!");
    }
    if (lvgl_port_task_handle == NULL)
    {
        ESP_LOGE(TAG, "LVGL Port Task creation failed!");
    }
    else
    {
        ESP_LOGI(TAG, "LVGL Port Task created successfully!");
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
#ifdef MAIN_DEBUG
            ESP_LOGI(TAG, "Wi-Fi Initialized. Ready!");
#endif
            // if (blink_led_task_handle != NULL)
            // {
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
        if (gpio_get_level(PIR_GPIO))
        {
#ifdef MAIN_DEBUG
            ESP_LOGI(TAG, "Proximity Detected");
#endif
            clear_buffer();
            xTaskNotify(state_control_task_handle, 0, eSetValueWithOverwrite);
            current_state = STATE_USER_DETECTED;
        }
        break;
        case STATE_USER_DETECTED: // Wait until NFC data is read or keypad press is entered
            char nfcUserID[ID_LEN] = {'\0'};

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
            display_screen(current_state);
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

    // Initialize peripherals
    i2c_master_init();
    configure_led();
    gc9a01_init();
    nfc_init();
    configure_led();
    // Create semaphore for signaling Wi-Fi init completion
    wifi_init_semaphore = xSemaphoreCreateBinary();

    create_screens();

    xTaskCreate(state_control_task, "state_control_task", 4096 * 2, NULL, 5, &state_control_task_handle);
    xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 3, &keypad_task_handle);
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);

#ifdef MAIN_DEBUG
    check_task_creation();
#endif

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}