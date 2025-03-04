
#include "main.h"

// TASK HANDLES
static TaskHandle_t blink_led_task_handle = NULL;
static TaskHandle_t wifi_init_task_handle = NULL;

static led_strip_handle_t led_strip;
static uint8_t s_led_state = 0;

static const char *MAIN_TAG = "MAIN";

static void blink_led_task(void *pvParameter)
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
#ifdef MAIN_DEBUG
    ESP_LOGI(MAIN_TAG, "Configured to blink addressable LED!");
#endif
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

static void heap_monitor_task(void *pvParameters)
{
    while (1)
    {
        long unsigned int free_heap = (long unsigned int)esp_get_free_heap_size();
        // ESP_LOGI(MAIN_TAG, "Free heap size: %lu bytes", free_heap);

        if (free_heap < HEAP_WARNING_THRESHOLD)
        {
            ESP_LOGE(MAIN_TAG, "CRITICAL: Heap size below safe limit! Free heap: %lu bytes", free_heap);
            // Optional: Restart the system if necessary
            // esp_restart();
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
    }
}

// Function to control state transitions and task management
static void state_control_task(void *pvParameter)
{
    while (1)
    {
#ifdef MAIN_HEAP_DEBUG
        ESP_LOGI("Memory", "Free heap size: %lu bytes", (unsigned long)esp_get_free_heap_size());
#endif
        switch (current_state)
        {
        case STATE_WIFI_INIT:
            // Start Wi-Fi init task if not already started
            if (wifi_init_task_handle == NULL)
            {
#ifdef MAIN_DEBUG
                ESP_LOGI(MAIN_TAG, "Starting Wi-Fi Init Task");
#endif
                xTaskCreate(wifi_init_task, "wifi_init_task", 4096, NULL, 4, &wifi_init_task_handle);
            }

            // Start LED blinking task if not already running
            if (blink_led_task_handle == NULL)
            {
#ifdef MAIN_DEBUG
                ESP_LOGI(MAIN_TAG, "Starting Blink LED Task");
#endif
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
            ESP_LOGI(MAIN_TAG, "Wi-Fi Initialized. Ready!");
#endif

            if (blink_led_task_handle != NULL)
            {
                vTaskDelete(blink_led_task_handle);
                blink_led_task_handle = NULL;
            }

            xEventGroupSetBits(event_group, IDLE_BIT);
            current_state = STATE_IDLE;
            break;

        case STATE_IDLE: // Wait until proximity is detected
            xEventGroupWaitBits(event_group, ENTERING_ID_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            current_state = STATE_USER_DETECTED;
            break;

        case STATE_USER_DETECTED: // Wait until NFC data is read or keypad press is entered
            xEventGroupWaitBits(event_group, ID_ENTERED_SUCCESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            current_state = STATE_DATABASE_VALIDATION;
            break;

        case STATE_DATABASE_VALIDATION: // Wait until validation is complete
            xEventGroupWaitBits(event_group, ID_AUTHENTICATED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            current_state = (!idIsValid) ? STATE_VALIDATION_FAILURE : (isAdministrator) ? STATE_ADMIN_MODE
                                                                  : (isCheckIn)         ? STATE_CHECK_IN
                                                                                        : STATE_CHECK_OUT;
            // current_state = check_in_successful ? STATE_VALIDATION_SUCCESS : STATE_VALIDATION_FAILURE;
            break;
        case STATE_CHECK_IN:
#ifdef MAIN_DEBUG
            ESP_LOGI(MAIN_TAG, "ID %s found in database. Checking in.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            xEventGroupSetBits(event_group, IDLE_BIT | CLEAN_UP_BIT);
            current_state = STATE_IDLE;
            break;
        case STATE_CHECK_OUT:
#ifdef MAIN_DEBUG
            ESP_LOGI(MAIN_TAG, "ID %s found in database. Checking out.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            xEventGroupSetBits(event_group, IDLE_BIT | CLEAN_UP_BIT);
            current_state = STATE_IDLE;
            break;
        case STATE_ADMIN_MODE:
#ifdef MAIN_DEBUG
            ESP_LOGI(MAIN_TAG, "ID %s found in database. Entering admin mode.", user_id);
#endif
            init_admin_mode();
            xEventGroupWaitBits(event_group, EXIT_ADMIN_MODE_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
            teardown_admin_mode();
            current_state = STATE_IDLE;
            break;
        case STATE_VALIDATION_FAILURE:
#ifdef MAIN_DEBUG
            ESP_LOGI(MAIN_TAG, "ID %s not found in database. Try again.", user_id);
#endif
            vTaskDelay(pdMS_TO_TICKS(2000)); // Display result for 5 seconds
            xEventGroupSetBits(event_group, ENTERING_ID_BIT);
            xEventGroupSetBits(event_group, CLEAN_UP_BIT);
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
            ESP_LOGE(MAIN_TAG, "Error state reached!");
            break;

        default:
            ESP_LOGW(MAIN_TAG, "Unknown state encountered: %d", current_state);
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#ifdef MAIN_DEBUG
    ESP_LOGI(MAIN_TAG, "State control task finished"); // Should not reach here unless task is deleted
#endif
}

// Main State Machine
void app_main(void)
{
    ESP_LOGI(MAIN_TAG, "Starting Kiosk Application");

    // Initialize event group
    event_group = xEventGroupCreate();

    // Initialize hardware and network
    i2c_master_init();
    nfc_init();
    gc9a01_init();
    sensor_init();
    configure_led();
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

    // ESP_ERROR_CHECK(heap_trace_init_standalone(trace_record, NUM_RECORDS));

#ifdef MAIN_HEAP_DEBUG
    xTaskCreate(heap_monitor_task, "HeapMonitor", MONITOR_TASK_STACK_SIZE, NULL, MONITOR_TASK_PRIORITY, NULL);
#endif

    // Create semaphore for signaling Wi-Fi init completion
    wifi_init_semaphore = xSemaphoreCreateBinary();

    init_kiosk_tasks();

#ifdef MAIN_HEAP_DEBUG
    ESP_LOGI("Memory", "STARTING FREE HEAP SIZE: %lu bytes", (long unsigned int)esp_get_free_heap_size());
#endif

    xTaskCreate(state_control_task, "state_control_task", 4096 * 2, NULL, 3, NULL);

    while (1)
    {
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }

    teardown_kiosk_tasks();
    ESP_LOGI(MAIN_TAG, "Kiosk Application Finished");
}