#include "main.h"
#include "global.h"


static state_t current_state = STATE_HARDWARE_INIT, prev_state = STATE_ERROR;

// Task Handles
static TaskHandle_t wifi_init_task_handle = NULL;
TaskHandle_t keypad_task_handle = NULL;
TaskHandle_t ota_update_task_handle = NULL;
extern TaskHandle_t cypd3177_task_handle = NULL;
static TaskHandle_t lvgl_port_task_handle = NULL;
static TaskHandle_t blink_led_task_handle = NULL;
TaskHandle_t admin_mode_control_task_handle = NULL;
TaskHandle_t state_control_task_handle = NULL;

// Semaphore Handles
SemaphoreHandle_t wifi_init_semaphore = NULL; // Semaphore to signal Wi-Fi init completion

static const char *TAG = "MAIN";

static uint8_t s_led_state = 0;
static int64_t start_time, end_time;

static led_strip_handle_t led_strip;

extern int usb_connected = 1;

void blink_led_task(void *pvParameter)
{
    while (1)
    {
        s_led_state = !s_led_state;

        /* If the addressable LED is enabled */
        if (s_led_state && current_state == STATE_WIFI_CONNECTING)
        {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            for (int i = 0; i < NUM_LEDS; i++)
            {
                led_strip_set_pixel(led_strip, i, 100, 0, 0);
            }
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

        if (usb_connected == 0)
        {
            led_strip_set_pixel(led_strip, 0, 50, 75, 60);
            led_strip_set_pixel(led_strip, 1, 50, 75, 60);
            led_strip_set_pixel(led_strip, 2, 50, 75, 60);
        }

        led_strip_refresh(led_strip);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

static void configure_led(void)
{
    MAIN_DEBUG_LOG("Configured to blink addressable LED!");
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
    screen_objects[STATE_IDLE] = display_idle();
    screen_objects[STATE_USER_DETECTED] = display_user_detected();
    screen_objects[STATE_CHECK_IN] = display_check_in_success();
    screen_objects[STATE_CHECK_OUT] = display_check_out_success();
    screen_objects[STATE_DATABASE_VALIDATION] = display_database_validation();
    screen_objects[STATE_VALIDATION_FAILURE] = display_check_in_failed();
    screen_objects[STATE_ERROR] = display_error();

    admin_screen_objects[ADMIN_STATE_ENTER_ID] = display_admin_enter_id();
    admin_screen_objects[ADMIN_STATE_VALIDATE_ID] = display_admin_id_validating();
    admin_screen_objects[ADMIN_STATE_TAP_CARD] = display_admin_tap_card();
    admin_screen_objects[ADMIN_STATE_CARD_WRITE_SUCCESS] = display_card_write_success();
    admin_screen_objects[ADMIN_STATE_ENTER_ID_ERROR] = display_id_enter_error();
    admin_screen_objects[ADMIN_STATE_CARD_WRITE_ERROR] = display_card_write_error();
    admin_screen_objects[ADMIN_STATE_ERROR] = display_admin_error();
}

// Load screen onto display as a function of current state
static void display_screen(state_t display_state)
{
    // Skip screen display for initialization states that don't have screens
    if (current_state == STATE_HARDWARE_INIT || 
        current_state == STATE_WIFI_CONNECTING || 
        current_state == STATE_SOFTWARE_INIT || 
        current_state == STATE_SYSTEM_READY)
    {
        return;
    }

    if (current_state != STATE_ADMIN_MODE)
    {
        if (screen_objects[current_state] != NULL)
        {
            MAIN_DEBUG_LOG("Displaying screen for state %d", current_state);
            _lock_acquire(&lvgl_api_lock);
            lv_scr_load(screen_objects[current_state]);
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
            // MAIN_DEBUG_LOG("Displaying admin screen for state %d", current_admin_state);
            _lock_acquire(&lvgl_api_lock);
            lv_scr_load(admin_screen_objects[current_admin_state]);
            _lock_release(&lvgl_api_lock);
        }
        // else
        // {
        //     MAIN_ERROR_LOG("Admin screen object not found for state %d", current_admin_state);
        // }
    }
}

// Function to control state transitions and task management
void state_control_task(void *pvParameter)
{
    char user_id[ID_LEN];
    while (1)
    {
        switch (current_state)
        {
        case STATE_HARDWARE_INIT:
            // Hardware is already initialized in app_main
            // Just start LED blinking task if not already running
            if (blink_led_task_handle == NULL)
            {
                MAIN_DEBUG_LOG("Starting Blink LED Task");
                xTaskCreate(blink_led_task, "blink_led_task", 1024, NULL, 2, &blink_led_task_handle);
            }

            // Move to WiFi connecting state
            current_state = STATE_WIFI_CONNECTING;
            break;

        case STATE_WIFI_CONNECTING:
            // Start Wi-Fi init task if not already started
            if (wifi_init_task_handle == NULL)
            {
                ESP_LOGI(TAG, "Starting Wi-Fi Init Task. Free heap: %lu bytes", esp_get_free_heap_size());
                BaseType_t xReturned = xTaskCreate(wifi_init_task, "wifi_init_task", 8192, NULL, 4, &wifi_init_task_handle);
                if (xReturned != pdPASS)
                {
                    ESP_LOGE(TAG, "Failed to create WiFi task!");
                    current_state = STATE_ERROR;
                    break;
                }
                ESP_LOGI(TAG, "WiFi task created successfully");
            }

            // Check if Wi-Fi init is completed (signaled by semaphore)
            if (xSemaphoreTake(wifi_init_semaphore, portMAX_DELAY) == pdTRUE)
            {
                ESP_LOGI(TAG, "WiFi initialization completed");
                current_state = STATE_SOFTWARE_INIT;
            }
            break;

        case STATE_SOFTWARE_INIT:
            // Initialize software services that need WiFi
            ESP_LOGI(TAG, "Starting software initialization. Free heap: %lu bytes", esp_get_free_heap_size());
            
            // Initialize MQTT first
            ESP_LOGI(TAG, "Initializing MQTT...");
            mqtt_init();
            ESP_LOGI(TAG, "MQTT initialized. Free heap: %lu bytes", esp_get_free_heap_size());
            
            // Publish connected status
            mqtt_publish_status("connected");
            ESP_LOGI(TAG, "Published connected status");
            
            // Start MQTT ping task
            ESP_LOGI(TAG, "Starting MQTT ping task...");
            mqtt_start_ping_task();
            ESP_LOGI(TAG, "MQTT ping task started. Free heap: %lu bytes", esp_get_free_heap_size());

            /*
            if (ota_update_task_handle == NULL) {
                MAIN_DEBUG_LOG("Creating OTA update task");
                xTaskCreate(ota_update_fw_task, "OTA UPDATE TASK", 1024 * 4, NULL, 8, &ota_update_task_handle);
            }
            */

            MAIN_DEBUG_LOG("Software services initialized");
            current_state = STATE_SYSTEM_READY;
            break;

        case STATE_SYSTEM_READY:
            MAIN_DEBUG_LOG("System fully initialized");
            current_state = STATE_IDLE;
            break;

        case STATE_IDLE: // Wait until proximity is detected
            if (gpio_get_level(PIR_GPIO))
            {
                MAIN_DEBUG_LOG("Proximity Detected");
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
                MAIN_DEBUG_LOG("User ID entered by %s", nfcReadFlag ? "NFC Transceiver" : "Keypad");

                if (nfcReadFlag)
                {
                    bool valid_id = true;
                    for (int i = 0; i < ID_LEN && nfcUserID[i] != '\0'; i++)
                    {
                        if (!isprint((unsigned char)nfcUserID[i]))
                        {
                            MAIN_ERROR_LOG("Invalid character in NFC ID at position %d", i);
                            valid_id = false;
                            break;
                        }
                    }

                    if (!valid_id)
                    {
                        MAIN_ERROR_LOG("Rejecting invalid NFC ID");
                        current_state = STATE_ERROR;
                        break;
                    }
                }
                memcpy(user_id, nfcReadFlag ? nfcUserID : keypad_buffer.elements, ID_LEN);

                MAIN_DEBUG_LOG("Processing user ID: %s", user_id);

                if (!is_numeric_string(user_id, ID_LEN))
                {
                    MAIN_ERROR_LOG("Invalid user ID: '%s'. Contains non-numeric characters", user_id);
                    current_state = STATE_VALIDATION_FAILURE;
                    break;
                }

                current_state = STATE_DATABASE_VALIDATION;
            }
            break;

        case STATE_DATABASE_VALIDATION: // Wait until validation is complete
#ifdef DATABASE_QUERY_ENABLED
            if (!get_user_info(user_id))
            {
                MAIN_ERROR_LOG("Invalid user detected");
                current_state = STATE_VALIDATION_FAILURE;
                break;
            }
            // If admin, set state to STATE_ADMIN_MODE
            else if (strcmp(user_info->role, "admin") == 0)
            {
                MAIN_DEBUG_LOG("Entering Admin Mode");
                xTaskNotifyGive(keypad_task_handle);
                xTaskNotifyGive(admin_mode_control_task_handle);
                current_state = STATE_ADMIN_MODE;
                break;
            }
            // If student, check-in/out
            else if (strcmp(user_info->role, "student") == 0)
            {
                start_time = esp_timer_get_time();
                if (strcmp(user_info->check_in_status, "Checked In") == 0)
                    current_state = check_out_user(user_id) ? STATE_CHECK_OUT : STATE_VALIDATION_FAILURE;
                else
                    current_state = check_in_user(user_id) ? STATE_CHECK_IN : STATE_VALIDATION_FAILURE;
            }
#else
            MAIN_DEBUG_LOG("Entering Admin Mode");
            xTaskNotifyGive(keypad_task_handle);
            xTaskNotifyGive(admin_mode_control_task_handle);
            current_state = STATE_ADMIN_MODE;
#endif
            break;

        case STATE_CHECK_IN:
            log_elapsed_time("check in authentication", start_time);
            MAIN_DEBUG_LOG("ID %s found in database. Checking in.", user_id);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;

        case STATE_CHECK_OUT:
            log_elapsed_time("check out authentication", start_time);
            MAIN_DEBUG_LOG("ID %s found in database. Checking out.", user_id);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;

        case STATE_ADMIN_MODE:
            BaseType_t adminNotify = ulTaskNotifyTake(pdTRUE, 0);

            if (adminNotify > 0)
            {
                MAIN_DEBUG_LOG("Exiting admin mode");
                current_state = STATE_IDLE;
            }
            break;

        case STATE_VALIDATION_FAILURE:
            MAIN_DEBUG_LOG("ID %s not found in database. Try again.", user_id);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Display result for 5 seconds
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
            MAIN_ERROR_LOG("Error state reached!");
            break;

        default:
            ESP_LOGW(TAG, "Unknown state encountered: %d", current_state);
            break;
        }
        if ((current_state != prev_state) || current_state == STATE_ADMIN_MODE)
        {
            play_kiosk_buzzer(current_state);
            display_screen(current_state);
            prev_state = current_state;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    MAIN_DEBUG_LOG("State control task finished"); // Should not reach here unless task is deleted
}
void app_main(void)
{
    MAIN_DEBUG_LOG("App Main Start");
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Initialize device info
    init_device_info();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Log initial heap size
    ESP_LOGI(TAG, "Initial free heap: %lu bytes", esp_get_free_heap_size());

    // GPIO config
    // gpio_config(&cypd3177_intr_config);

    // Install the ISR service and attach handlers
    gpio_install_isr_service(0);
    // Initialize I2C bus
    i2c_master_init(&master_handle);
    // i2c_master_add_device(&master_handle, &cypd3177_i2c_handle, &cypd3177_i2c_config);
    i2c_master_add_device(&master_handle, &pcf8574n_i2c_handle, &pcf8574n_i2c_config);
    i2c_master_add_device(&master_handle, &bq25798_i2c_handle, &bq25798_i2c_config);
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Initialize peripherals
    // Create semaphore for signaling Wi-Fi init completion
    wifi_init_semaphore = xSemaphoreCreateBinary();

    // Initialize hardware components that don't need WiFi
    MAIN_DEBUG_LOG("Starting hardware initialization. Free heap: %lu bytes", esp_get_free_heap_size());
  
    configure_led();
    MAIN_DEBUG_LOG("LED configured. Free heap: %lu bytes", esp_get_free_heap_size());
    
    gc9a01_init();
    MAIN_DEBUG_LOG("Display initialized. Free heap: %lu bytes", esp_get_free_heap_size());
    
    nfc_init();
    MAIN_DEBUG_LOG("NFC initialized. Free heap: %lu bytes", esp_get_free_heap_size());
    
    buzzer_init();
    MAIN_DEBUG_LOG("Buzzer initialized. Free heap: %lu bytes", esp_get_free_heap_size());
  
    sensor_init();
    // Log heap size before screen creation
    MAIN_DEBUG_LOG("Free heap before screen creation: %lu bytes", esp_get_free_heap_size());

    // Create screens with memory monitoring
    MAIN_DEBUG_LOG("Creating display screens...");
    create_screens();
    MAIN_DEBUG_LOG("Free heap after screen creation: %lu bytes", esp_get_free_heap_size());

#ifdef MAIN_HEAP_DEBUG
    xTaskCreate(heap_monitor_task, "HeapMonitor", MONITOR_TASK_STACK_SIZE, NULL, 1, NULL);
#endif

    // Create tasks with increased stack sizes and priorities
    MAIN_DEBUG_LOG("Creating tasks...");
    xTaskCreate(state_control_task, "state_control_task", 4096 * 2, NULL, 5, &state_control_task_handle);
    xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 3, &keypad_task_handle);
    // xTaskCreate(power_check, "cypd3177_task", 4096, NULL, 3, &cypd3177_task_handle);
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);
    xTaskCreate(admin_mode_control_task, "admin_mode_control_task", 4096 * 2, NULL, 4, &admin_mode_control_task_handle);
    xTaskCreate(bq25798_monitor_task, "bq25798_monitor", 4096, NULL, 2, NULL);
    ESP_LOGI(TAG, "BQ25798 monitoring task started");

    MAIN_DEBUG_LOG("Free heap after task creation: %lu bytes", esp_get_free_heap_size());

#ifdef MAIN_DEBUG
    check_task_creation("State control", state_control_task_handle);
    check_task_creation("Keypad", keypad_task_handle);
    check_task_creation("LVGL", lvgl_port_task_handle);
    check_task_creation("Admin mode control", admin_mode_control_task_handle);
#endif

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    teardown_task(&state_control_task_handle);
    teardown_task(&keypad_task_handle);
    teardown_task(&lvgl_port_task_handle);
    teardown_task(&admin_mode_control_task_handle);
    teardown_task(&blink_led_task_handle);
    teardown_task(&wifi_init_task_handle);

    MAIN_DEBUG_LOG("App Main End");
}
