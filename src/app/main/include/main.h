// ESP-IDF header files
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_heap_task_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "nvs_flash.h"

// project component header files
#include "wifi_init.h"
#include "ota.h"
#include "firebase_http_client.h"
#include "display_config.h"
#include "display_frames.h"
#include "admin_frames.h"
#include "admin_mode.h"
#include "pir.h"
#include "esp_timer.h"

#include "i2c_config.h"
#include "cypd3177.h"
#include "pcf8574n.h"

#define ID_LEN 10
#define BLINK_GPIO 8
#define NUM_LEDS 3
#define MAIN_DEBUG 1
#define HEAP_WARNING_THRESHOLD 5000  // Minimum acceptable heap size in bytes
#define MONITOR_TASK_STACK_SIZE 2048 // Stack size in words (4 bytes each)
#define MONITOR_TASK_PRIORITY 5      // Task priority
#define MAIN_HEAP_DEBUG 1
#define DATABASE_QUERY_ENABLED 1
#ifdef MAIN_DEBUG
#define MAIN_DEBUG_LOG(fmt, ...)           \
    do                                     \
    {                                      \
        ESP_LOGI(TAG, fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define MAIN_DEBUG_LOG(fmt, ...) ((void)0)
#endif

#ifdef MAIN_DEBUG
#define MAIN_ERROR_LOG(fmt, ...)           \
    do                                     \
    {                                      \
        ESP_LOGE(TAG, fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define MAIN_ERROR_LOG(fmt, ...) ((void)0)
#endif

typedef enum
{
    STATE_WIFI_INIT,
    STATE_WIFI_READY,
    STATE_IDLE,
    STATE_USER_DETECTED,
    STATE_DATABASE_VALIDATION,
    STATE_CHECK_IN,
    STATE_CHECK_OUT,
    STATE_ADMIN_MODE,
    STATE_VALIDATION_FAILURE,
    STATE_ERROR
} state_t;

pn532_t nfc;           // Defined in ntag_reader.h
_lock_t lvgl_api_lock; // Defined in display_config.h
lv_display_t *display; // Defined in display_config.h
lv_obj_t *screen_objects[STATE_ERROR + 1] = {NULL};
lv_obj_t *admin_screen_objects[ADMIN_STATE_ERROR + 1] = {NULL};

extern UserInfo user_info_instance;
extern UserInfo *user_info;
extern TaskHandle_t state_control_task_handle;
extern TaskHandle_t admin_mode_control_task_handle;
extern TaskHandle_t cypd3177_task_handle;
extern admin_state_t current_admin_state, prev_admin_state;

extern int usb_connected;

char user_id[ID_LEN];
void state_control_task(void *pvParameter);
void blink_led_1_task(void *pvParameter);
void blink_led_2_task(void *pvParameter);
