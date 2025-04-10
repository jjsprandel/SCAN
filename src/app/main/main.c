#include "global.h"
#include "main.h"
#include "power_mgmt.h"
#include "../database/include/kiosk_firebase.h"

// State variables
static state_t current_state = STATE_HARDWARE_INIT, prev_state = STATE_ERROR;
static admin_state_t prev_admin_state = ADMIN_STATE_ERROR;

// Task Handles
TaskHandle_t state_control_task_handle = NULL;
static TaskHandle_t wifi_init_task_handle = NULL;
TaskHandle_t ota_update_task_handle = NULL;
TaskHandle_t keypad_task_handle = NULL;
TaskHandle_t cypd3177_task_handle = NULL;
static TaskHandle_t lvgl_port_task_handle = NULL;
static TaskHandle_t blink_led_task_handle = NULL;
TaskHandle_t admin_mode_control_task_handle = NULL;

// Semaphore Handles
SemaphoreHandle_t wifi_init_semaphore = NULL; // Semaphore to signal Wi-Fi init completion

static const char *TAG = "MAIN";

static uint8_t s_led_state = 0;
static int64_t start_time, end_time;

static led_strip_handle_t led_strip;

int usb_connected = 1;
char user_id[ID_LEN + 1];

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

// Function to control state transitions and task management
void state_control_task(void *pvParameter)
{
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
            mqtt_publish_status("Kiosk Connected");
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
            vTaskDelay(pdMS_TO_TICKS(5000));
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
                if (nfcReadFlag)
                {
                    MAIN_DEBUG_LOG("User ID entered by NFC Transceiver");
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
                else{
                    MAIN_DEBUG_LOG("User ID entered by Keypad");
                    if (keypad_buffer.occupied == ID_LEN){
                        MAIN_DEBUG_LOG("ID of valid length is entered");
                    }
                    else{
                        MAIN_ERROR_LOG("ID of invalid length is entered");
                        current_state = STATE_KEYPAD_ENTRY_ERROR;
                        break;
                    }
                }
                memcpy(user_id, nfcReadFlag ? nfcUserID : keypad_buffer.elements, ID_LEN);
                user_id[ID_LEN] = '\0';
                MAIN_DEBUG_LOG("Processing user ID: %s", user_id);

                if (!is_numeric_string(user_id, ID_LEN))
                {
                    MAIN_ERROR_LOG("Invalid user ID: '%s'. Contains non-numeric characters", user_id);
                    current_state = STATE_VALIDATION_FAILURE;
                    break;
                }
                current_state = STATE_DATABASE_VALIDATION;
                
                // Format user ID for MQTT message
                char formatted_id[ID_LEN + 1];
                snprintf(formatted_id, sizeof(formatted_id), "%s", user_id);
                char mqtt_message[64];
                snprintf(mqtt_message, sizeof(mqtt_message), "Validating %s", formatted_id);
                mqtt_publish_status(mqtt_message);
            }
            break;
            
        case STATE_KEYPAD_ENTRY_ERROR:
            MAIN_ERROR_LOG("ID %s of length %d entered. ID must be of length %d. Try again.", keypad_buffer.elements, keypad_buffer.occupied, ID_LEN);
            clear_buffer();
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_state = STATE_USER_DETECTED;
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
            else if (strcmp(user_info->role, "Admin") == 0)
            {
                MAIN_DEBUG_LOG("Entering Admin Mode");
                mqtt_publish_status("Entering Admin Mode");
                xTaskNotifyGive(keypad_task_handle);
                xTaskNotifyGive(admin_mode_control_task_handle);
                current_state = STATE_ADMIN_MODE;
                break;
            }
            // If student, check-in/out
            else if (strcmp(user_info->role, "Student") == 0)
            {
                start_time = esp_timer_get_time();
                if (strcmp(user_info->check_in_status, "Checked In") == 0)
                {
                    current_state = check_out_user(user_id) ? STATE_CHECK_OUT : STATE_VALIDATION_FAILURE;
                }
                else
                {
                    current_state = check_in_user(user_id) ? STATE_CHECK_IN : STATE_VALIDATION_FAILURE;
                }
            }
            else
            {
                MAIN_ERROR_LOG("Invalid user role: %s", user_info->role);
                current_state = STATE_VALIDATION_FAILURE;
                break;
            }
#else
            MAIN_DEBUG_LOG("Entering Admin Mode");
            mqtt_publish_status("Entering Admin Mode");
            xTaskNotifyGive(keypad_task_handle);
            xTaskNotifyGive(admin_mode_control_task_handle);
            current_state = STATE_ADMIN_MODE;
#endif
            break;
        case STATE_CHECK_IN:
            log_elapsed_time("check in authentication", start_time);

            // Debug logs for user info
            if (user_info != NULL)
            {
                MAIN_DEBUG_LOG("CHECK-IN: User info available - Name: %s %s, ID: %s",
                               user_info->first_name, user_info->last_name, user_id);
                
                // Format user info for MQTT message
                char mqtt_message[128];
                snprintf(mqtt_message, sizeof(mqtt_message), "User Checked In: %s %s", 
                         user_info->first_name, user_info->last_name);
                mqtt_publish_status(mqtt_message);
            }
            else
            {
                MAIN_DEBUG_LOG("CHECK-IN: No user info available. ID: %s", user_id);
                char mqtt_message[64];
                snprintf(mqtt_message, sizeof(mqtt_message), "User Checked In: %s", user_id);
                mqtt_publish_status(mqtt_message);
            }

            MAIN_DEBUG_LOG("ID %s found in database. Checking in.", user_id);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;
        case STATE_CHECK_OUT:
            log_elapsed_time("check out authentication", start_time);
            
            // Format user info for MQTT message
            if (user_info != NULL)
            {
                char mqtt_message[128];
                snprintf(mqtt_message, sizeof(mqtt_message), "User Checked Out: %s %s", 
                         user_info->first_name, user_info->last_name);
                mqtt_publish_status(mqtt_message);
            }
            else
            {
                char mqtt_message[64];
                snprintf(mqtt_message, sizeof(mqtt_message), "User Checked Out: %s", user_id);
                mqtt_publish_status(mqtt_message);
            }
            
            MAIN_DEBUG_LOG("ID %s found in database. Checking out.", user_id);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;
        case STATE_ADMIN_MODE:
            BaseType_t adminNotify = ulTaskNotifyTake(pdTRUE, 0);

            if (adminNotify > 0)
            {
                MAIN_DEBUG_LOG("Exiting admin mode");
                mqtt_publish_status("Exiting Admin Mode");
                current_state = STATE_IDLE;
            }
            break;
        case STATE_VALIDATION_FAILURE:
#ifdef MAIN_DEBUG
            MAIN_DEBUG_LOG("ID %s not found in database. Try again.", user_id);
#endif
            // Add MQTT status message for validation failure
            char validation_failure_msg[64];
            snprintf(validation_failure_msg, sizeof(validation_failure_msg), "Validation Failed: %s", user_id);
            mqtt_publish_status(validation_failure_msg);
            
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
        if ((current_state != prev_state) || ((current_state == STATE_ADMIN_MODE) && (current_admin_state != prev_admin_state)))
        {
            play_kiosk_buzzer(current_state, current_admin_state);
            display_screen(current_state, current_admin_state);
            prev_state = current_state;

            if (current_state == STATE_ADMIN_MODE)
                prev_admin_state = current_admin_state;
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
    gpio_config(&cypd3177_intr_config);

    // Install the ISR service and attach handlers
    gpio_install_isr_service(0);

    // Initialize I2C bus
    i2c_master_init(&master_handle);
    i2c_master_add_device(&master_handle, &cypd3177_i2c_handle, &cypd3177_i2c_config);
    i2c_master_add_device(&master_handle, &pcf8574n_i2c_handle, &pcf8574n_i2c_config);
    i2c_master_add_device(&master_handle, &bq25798_i2c_handle, &bq25798_i2c_config);
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Initialize BQ25798 charger
    ESP_LOGI(TAG, "Initializing BQ25798 charger...");
    ret = bq25798_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BQ25798 charger");
        return;
    }
    ESP_LOGI(TAG, "BQ25798 charger initialized successfully");
    
    // Initialize power management
    ESP_LOGI(TAG, "Initializing power management...");
    ret = power_mgmt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize power management");
        return;
    }
    ESP_LOGI(TAG, "Power management initialized successfully");
    
    // Initialize kiosk Firebase client
    ESP_LOGI(TAG, "Initializing kiosk Firebase client...");
    if (!kiosk_firebase_init()) {
        ESP_LOGE(TAG, "Failed to initialize kiosk Firebase client");
        return;
    }
    ESP_LOGI(TAG, "Kiosk Firebase client initialized successfully");
    
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
    ESP_LOGI(TAG, "Creating tasks...");
    xTaskCreate(state_control_task, "state_control_task", 4096 * 2, NULL, 5, &state_control_task_handle);
    xTaskCreate(keypad_handler, "keypad_task", 4096, NULL, 3, &keypad_task_handle);
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);
    xTaskCreate(admin_mode_control_task, "admin_mode_control_task", 4096 * 2, NULL, 4, &admin_mode_control_task_handle);

    ESP_LOGI(TAG, "Free heap after task creation: %lu bytes", esp_get_free_heap_size());

#ifdef MAIN_DEBUG
    check_task_creation("State control", state_control_task_handle);
    check_task_creation("Keypad", keypad_task_handle);
    check_task_creation("LVGL", lvgl_port_task_handle);
    check_task_creation("Admin mode control", admin_mode_control_task_handle);
#endif

    // Create display test task
    // xTaskCreate(display_test_task, "display_test", 4096, NULL, 5, NULL);
    
    // Power management tasks are created in power_mgmt_init()
    
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
