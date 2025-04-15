#include "global.h"
#include "main.h"
#include "power_mgmt.h"
#include "../database/include/kiosk_firebase.h"

// State variables
static state_t current_state = STATE_HARDWARE_INIT; 
static state_t prev_state = STATE_ERROR;
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
static int64_t start_time;

static led_strip_handle_t led_strip;

int usb_connected = 1;
char user_id[ID_LEN + 1];

void blink_led_task(void *pvParameter)
{
    while (1)
    {
        s_led_state = !s_led_state;

        if (s_led_state && current_state == STATE_WIFI_CONNECTING)
        {
            for (int i = 0; i < NUM_LEDS; i++)
            {
                led_strip_set_pixel(led_strip, i, 100, 0, 0);
            }
            led_strip_refresh(led_strip);
        }
        else
        {
            led_strip_clear(led_strip);
        }
        
        switch (current_state)
        {
            case STATE_HARDWARE_INIT:
                for (int i = 0; i < NUM_LEDS; i++)
                {
                    led_strip_set_pixel(led_strip, i, 100, 0, 0);
                }
                break;
                
            case STATE_WIFI_CONNECTING:
                break;
                
            case STATE_SOFTWARE_INIT:
                for (int i = 0; i < NUM_LEDS; i++)
                {
                    led_strip_set_pixel(led_strip, i, 100, 100, 0);
                }
                break;
                
            case STATE_SYSTEM_READY:
                for (int i = 0; i < NUM_LEDS; i++)
                {
                    led_strip_set_pixel(led_strip, i, 0, 100, 0);
                }
                break;
                
            case STATE_USER_DETECTED:
                led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                break;
                
            case STATE_DATABASE_VALIDATION:
                led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                break;
                
            case STATE_CHECK_IN:
            case STATE_CHECK_OUT:
                led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                led_strip_set_pixel(led_strip, 0, 0, 100, 0);
                break;
                
            case STATE_VALIDATION_FAILURE:
                led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                led_strip_set_pixel(led_strip, 0, 100, 0, 0);
                break;
                
            case STATE_KEYPAD_ENTRY_ERROR:
                for (int i = 0; i < NUM_LEDS; i++)
                {
                    led_strip_set_pixel(led_strip, i, 100, 0, 0);
                }
                break;
                
            case STATE_IDLE:
                if (ota_update_task_handle != NULL) {
                    // OTA update in progress - all LEDs white
                    for (int i = 0; i < NUM_LEDS; i++) {
                        led_strip_set_pixel(led_strip, i, 100, 100, 100);
                    }
                } else {
                    // Normal idle state - all LEDs off
                    led_strip_clear(led_strip);
                }
                break;
                
            case STATE_ERROR:
                for (int i = 0; i < NUM_LEDS; i++)
                {
                    led_strip_set_pixel(led_strip, i, 100, 0, 0);
                }
                break;
                
            case STATE_ADMIN_MODE:
                switch (current_admin_state)
                {
                    case ADMIN_STATE_BEGIN:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 0, 0, 100);
                        }
                        break;
                        
                    case ADMIN_STATE_ENTER_ID:
                        led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                        led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                        break;
                        
                    case ADMIN_STATE_VALIDATE_ID:
                        led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                        led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                        led_strip_set_pixel(led_strip, 0, 0, 100, 0);
                        break;
                        
                    case ADMIN_STATE_TAP_CARD:
                        led_strip_set_pixel(led_strip, 2, 0, 0, 100);
                        led_strip_set_pixel(led_strip, 1, 100, 100, 0);
                        led_strip_set_pixel(led_strip, 0, 100, 0, 100);
                        break;
                        
                    case ADMIN_STATE_CARD_WRITE_SUCCESS:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 0, 100, 0);
                        }
                        break;
                        
                    case ADMIN_STATE_ENTER_ID_ERROR:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 100, 0, 0);
                        }
                        break;
                        
                    case ADMIN_STATE_CARD_WRITE_ERROR:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 100, 0, 0);
                        }
                        break;
                        
                    case ADMIN_STATE_ERROR:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 100, 0, 0);
                        }
                        break;
                        
                    default:
                        for (int i = 0; i < NUM_LEDS; i++)
                        {
                            led_strip_set_pixel(led_strip, i, 0, 0, 100);
                        }
                        break;
                }
                break;
                
            default:
                led_strip_clear(led_strip);
                break;
        }

        if (usb_connected == 0)
        {
            led_strip_set_pixel(led_strip, 2, 50, 75, 60);
            led_strip_set_pixel(led_strip, 1, 50, 75, 60);
            led_strip_set_pixel(led_strip, 0, 50, 75, 60);
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
    static int64_t user_detected_start_time = 0;
    while (1)
    {
        int64_t state_control_loop_start_time = esp_timer_get_time();
        switch (current_state)
        {
        case STATE_HARDWARE_INIT:
            // Hardware is already initialized in app_main
            // Just start LED blinking task if not already running
            if (blink_led_task_handle == NULL)
            {
                MAIN_DEBUG_LOG("Starting Blink LED Task");
                xTaskCreate(blink_led_task, "blink_led_task", 1024, NULL, 2, &blink_led_task_handle);
                MAIN_DEBUG_LOG("Blink LED Task created. Free heap: %lu bytes", esp_get_free_heap_size());
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
                ESP_LOGI(TAG, "WiFi task created successfully. Free heap: %lu bytes", esp_get_free_heap_size());
            }

            // Check if Wi-Fi init is completed (signaled by semaphore)
            if (xSemaphoreTake(wifi_init_semaphore, portMAX_DELAY) == pdTRUE)
            {
                ESP_LOGI(TAG, "WiFi initialization completed. Free heap: %lu bytes", esp_get_free_heap_size());
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
            ESP_LOGI(TAG, "Published connected status. Free heap: %lu bytes", esp_get_free_heap_size());
            
            // Start MQTT ping task
            ESP_LOGI(TAG, "Starting MQTT ping task...");
            mqtt_start_ping_task();
            ESP_LOGI(TAG, "MQTT ping task started. Free heap: %lu bytes", esp_get_free_heap_size());

            MAIN_DEBUG_LOG("Software services initialized. Free heap: %lu bytes", esp_get_free_heap_size());
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
                user_detected_start_time = esp_timer_get_time();
                MAIN_DEBUG_LOG("User detection started - 10 second timeout");
                current_state = STATE_USER_DETECTED;
            }
            break;
        case STATE_USER_DETECTED: // Wait until NFC data is read or keypad press is entered
            const int64_t USER_DETECTED_TIMEOUT_US = ID_ENTRY_TIMEOUT_SEC * 1000 * 1000; // 10 seconds in microseconds
            static int last_keypad_buffer_length = 0;

            // Check if timeout has occurred
            int64_t current_time = esp_timer_get_time();

            // Reset the timeout if a key on the keypad is pressed
            if (last_keypad_buffer_length != keypad_buffer.occupied) {
                user_detected_start_time = esp_timer_get_time();
                MAIN_DEBUG_LOG("Keypad pressed - 10 second timeout reset");
            }

            if (current_time - user_detected_start_time > USER_DETECTED_TIMEOUT_US) {
                MAIN_DEBUG_LOG("User detection timeout - returning to IDLE state");
                // user_detected_timeout_initialized = false; // Reset for next time
                current_state = STATE_IDLE;
                break;
            }
            
            char nfcUserID[ID_LEN] = {'\0'};
            bool nfcReadFlag = false;
            BaseType_t keypadNotify = ulTaskNotifyTake(pdTRUE, 0);
            
            nfcReadFlag = read_user_id(nfcUserID, 50);

            if ((nfcReadFlag) || (keypadNotify > 0))
            {
                // Reset the timeout flag since we're exiting this state
                // user_detected_timeout_initialized = false;
                
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
            last_keypad_buffer_length = keypad_buffer.occupied;
            break;
            
        case STATE_KEYPAD_ENTRY_ERROR:
            MAIN_ERROR_LOG("ID %s of length %d entered. ID must be of length %d. Try again.", keypad_buffer.elements, keypad_buffer.occupied, ID_LEN);
            clear_buffer();
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_state = STATE_USER_DETECTED;
            break;

        case STATE_DATABASE_VALIDATION: // Wait until validation is complete
#ifdef DATABASE_QUERY_ENABLED
            start_time = esp_timer_get_time();
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
            vTaskDelay(pdMS_TO_TICKS(4000)); // Display result for 5 seconds
            current_state = STATE_IDLE;
            break;
        case STATE_CHECK_OUT:
            log_elapsed_time("check-out authentication", start_time);
            
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
            vTaskDelay(pdMS_TO_TICKS(4000)); // Display result for 5 seconds
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
            log_elapsed_time("validation failure", start_time);
#ifdef MAIN_DEBUG
            MAIN_DEBUG_LOG("ID %s not found in database. Try again.", user_id);
#endif
            // Add MQTT status message for validation failure
            char validation_failure_msg[64];
            snprintf(validation_failure_msg, sizeof(validation_failure_msg), "Validation Failed: %s", user_id);
            mqtt_publish_status(validation_failure_msg);
            
            vTaskDelay(pdMS_TO_TICKS(4000)); // Display result for 5 seconds
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
        // log_elapsed_time("state control loop", state_control_loop_start_time);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    // ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
    MAIN_DEBUG_LOG("State control task finished"); // Should not reach here unless task is deleted
}

void app_main(void)
{
    MAIN_DEBUG_LOG("App Main Start");
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "App main starting free heap size: %lu", esp_get_free_heap_size());

    // Initialize device info
    init_device_info();
    ESP_LOGI(TAG, "Device info initialized. Free heap: %lu bytes", esp_get_free_heap_size());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized. Free heap: %lu bytes", esp_get_free_heap_size());

    // Log initial heap size
    ESP_LOGI(TAG, "Initial free heap: %lu bytes", esp_get_free_heap_size());

    // GPIO config
    //gpio_config(&cypd3177_intr_config);
    //ESP_LOGI(TAG, "GPIO configured. Free heap: %lu bytes", esp_get_free_heap_size());

    // Install the ISR service and attach handlers
    gpio_install_isr_service(0);
    ESP_LOGI(TAG, "ISR service installed. Free heap: %lu bytes", esp_get_free_heap_size());

    // Initialize I2C bus
    i2c_master_init(&master_handle);
    i2c_master_add_device(&master_handle, &cypd3177_i2c_handle, &cypd3177_i2c_config);
    i2c_master_add_device(&master_handle, &pcf8574n_i2c_handle, &pcf8574n_i2c_config);
    i2c_master_add_device(&master_handle, &bq25798_i2c_handle, &bq25798_i2c_config);
    ESP_LOGI(TAG, "I2C initialized successfully. Free heap: %lu bytes", esp_get_free_heap_size());

    // Initialize BQ25798 charger
    ESP_LOGI(TAG, "Initializing BQ25798 charger...");
    ret = bq25798_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BQ25798 charger");
        return;
    }
    ESP_LOGI(TAG, "BQ25798 charger initialized successfully. Free heap: %lu bytes", esp_get_free_heap_size());
    
    // Initialize power management
    ESP_LOGI(TAG, "Initializing power management...");
    ret = power_mgmt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize power management");
        return;
    }
    ESP_LOGI(TAG, "Power management initialized successfully. Free heap: %lu bytes", esp_get_free_heap_size());
    
    // Initialize kiosk Firebase client
    ESP_LOGI(TAG, "Initializing kiosk Firebase client...");
    if (!kiosk_firebase_init()) {
        ESP_LOGE(TAG, "Failed to initialize kiosk Firebase client");
        return;
    }
    ESP_LOGI(TAG, "Kiosk Firebase client initialized successfully. Free heap: %lu bytes", esp_get_free_heap_size());
    
    // Initialize peripherals
    // Create semaphore for signaling Wi-Fi init completion
    wifi_init_semaphore = xSemaphoreCreateBinary();
    ESP_LOGI(TAG, "WiFi init semaphore created. Free heap: %lu bytes", esp_get_free_heap_size());

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
    MAIN_DEBUG_LOG("Sensor initialized. Free heap: %lu bytes", esp_get_free_heap_size());
    
    // Log heap size before screen creation
    MAIN_DEBUG_LOG("Free heap before screen creation: %lu bytes", esp_get_free_heap_size());

    // Create screens with memory monitoring
    MAIN_DEBUG_LOG("Creating display screens...");
    create_screens();
    MAIN_DEBUG_LOG("Free heap after screen creation: %lu bytes", esp_get_free_heap_size());

#ifdef MAIN_HEAP_DEBUG
    xTaskCreate(heap_monitor_task, "HeapMonitor", MONITOR_TASK_STACK_SIZE, NULL, 1, NULL);
    ESP_LOGI(TAG, "Heap monitor task created. Free heap: %lu bytes", esp_get_free_heap_size());
#endif

    // Create tasks with increased stack sizes and priorities
    ESP_LOGI(TAG, "Creating tasks...");
    xTaskCreate(state_control_task, "state_control_task", 8192, NULL, STATE_CONTROL_TASK_PRIORITY, &state_control_task_handle);
    ESP_LOGI(TAG, "State control task created. Free heap: %lu bytes", esp_get_free_heap_size());
    
    xTaskCreate(keypad_handler, "keypad_task", 1024 * 2, NULL, 3, &keypad_task_handle);
    ESP_LOGI(TAG, "Keypad task created. Free heap: %lu bytes", esp_get_free_heap_size());
    
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_port_task_handle);
    ESP_LOGI(TAG, "LVGL task created. Free heap: %lu bytes", esp_get_free_heap_size());
    
    xTaskCreate(admin_mode_control_task, "admin_mode_control_task", 8192, NULL, 4, &admin_mode_control_task_handle);
    ESP_LOGI(TAG, "Admin mode control task created. Free heap: %lu bytes", esp_get_free_heap_size());

    ESP_LOGI(TAG, "Free heap after task creation: %lu bytes", esp_get_free_heap_size());

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
