#ifndef MAIN_H
#define MAIN_H

// INCLUDES
// #include <stdio.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "freertos/semphr.h"
// #include "freertos/event_groups.h"
// #include "esp_system.h"
// #include "driver/gpio.h"
// #include "esp_log.h"
// #include "esp_log_buffer.h"
// #include "sdkconfig.h"
// #include "nvs_flash.h"
// #include "ntag_reader.h"
// #include "pn532.h"
// #include "esp_heap_task_info.h"
// #include "display_frames.h"
// #include "pir.h"
#include "led_strip.h"
#include "keypad_driver.h"
#include "wifi_init.h"
#include "ota.h"
#include "display_config.h"
#include "kiosk_tasks.h"

// MACROS
// #define BLINK_GPIO 8
// #define MAIN_DEBUG 1
// #define PROXIMITY_DETECTED_BIT BIT0
// #define ID_ENTERED_SUCCESS_BIT BIT1
// #define ID_AUTHENTICATED_BIT BIT2
// #define ADMIN_MODE_BIT BIT3
// #define IDLE_BIT BIT4
// #define ENTERING_ID_BIT BIT5
// #define NEW_ID_ENTERED_SUCCESS_BIT BIT6
// #define ID_LEN 7

#define HEAP_WARNING_THRESHOLD 5000  // Minimum acceptable heap size in bytes
#define MONITOR_TASK_STACK_SIZE 2048 // Stack size in words (4 bytes each)
#define MONITOR_TASK_PRIORITY 5      // Task priority

// GLOBAL VARIABLES

TaskHandle_t keypad_task_handle = NULL;

// not static because it is being used in wifi_init.c as extern variable
SemaphoreHandle_t wifi_init_semaphore = NULL; // Semaphore to signal Wi-Fi init completion
EventGroupHandle_t event_group;

// STATE VARIABLES
state_t current_state = STATE_WIFI_INIT;

// EXTERN VARIABLES
pn532_t nfc;                          // Defined in ntag_reader.h
extern keypad_buffer_t keypad_buffer; // Defined in keypad_driver.h
_lock_t lvgl_api_lock;                // Defined in display_config.h
lv_display_t *display;                // Defined in display_config.h
lv_obj_t *disp_obj;                   // Defined in kiosk_states.h

char nfcUserID[MAX_ID_LEN];
extern bool check_in_successful;

#endif