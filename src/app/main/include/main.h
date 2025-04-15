#ifndef MAIN_H
#define MAIN_H

// ESP-IDF header files
#include <stdio.h>
#include <ctype.h>
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
#include "main_utils.h"
#include "state_enum.h"
#include "wifi_init.h"
#include "ota.h"
#include "firebase_http_client.h"
#include "display_config.h"
#include "ui_screens.h"
#include "ui_styles.h"
#include "pir_sensor.h"
#include "status_buzzer.h"
#include "esp_timer.h"
#include "mqtt.h"
#include "i2c_config.h"
#include "cypd3177.h"
#include "bq25798.h"
#include "pcf8574n.h"
#include "admin_mode.h"
#include "firebase_utils.h"
#include "display_utils.h"

#define ID_LEN 10
#define BLINK_GPIO 8
#define NUM_LEDS 3
#define MAIN_DEBUG 1
#define HEAP_WARNING_THRESHOLD 5000  // Minimum acceptable heap size in bytes
#define MONITOR_TASK_STACK_SIZE 2048 // Stack size in words (4 bytes each)
#define MONITOR_TASK_PRIORITY 5      // Task priority
#define MAIN_HEAP_DEBUG 1
#define DATABASE_QUERY_ENABLED 1
#define USING_MAIN_PCB 1
#define ID_ENTRY_TIMEOUT_SEC 10
#define STATE_CONTROL_TASK_PRIORITY 1

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

pn532_t nfc;           // Defined in ntag_reader.h
_lock_t lvgl_api_lock; // Defined in display_config.h
lv_display_t *display; // Defined in display_config.h
extern lv_obj_t *screen_objects[STATE_ERROR + 1];
extern lv_obj_t *admin_screen_objects[ADMIN_STATE_ERROR + 1];

extern TaskHandle_t state_control_task_handle;
extern TaskHandle_t admin_mode_control_task_handle;
extern TaskHandle_t cypd3177_task_handle;
extern admin_state_t current_admin_state;
extern int usb_connected;

extern char user_id[ID_LEN + 1];
extern char user_id_to_write[ID_LEN+1];

void state_control_task(void *pvParameter);
void blink_led_1_task(void *pvParameter);
void blink_led_2_task(void *pvParameter);

#endif // MAIN_H
