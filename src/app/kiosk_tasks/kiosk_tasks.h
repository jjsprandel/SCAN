#ifndef KIOSK_TASKS_H
#define KIOSK_TASKS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_log_buffer.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "ntag_reader.h"
#include "pn532.h"
#include "esp_heap_task_info.h"
#include "keypad_driver.h"
// #include "wifi_init.h"
// #include "ota.h"
#include "display_config.h"
#include "display_frames.h"
#include "pir.h"

#define BLINK_GPIO 8
#define TASK_DEBUG 1
#define PROXIMITY_DETECTED_BIT BIT0
#define ID_ENTERED_NFC_BIT BIT1
#define ID_AUTHENTICATED_BIT BIT2
#define EXIT_ADMIN_MODE_BIT BIT3
#define IDLE_BIT BIT4
#define ENTERING_ID_BIT BIT5
#define ID_ENTERED_KEYPAD_BIT BIT6
#define ID_LEN 8

// #define DATABASE_ENABLED

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

extern state_t current_state, prev_state;

extern keypad_buffer_t keypad_buffer; // Defined in keypad_driver.c
extern _lock_t lvgl_api_lock;         // Defined in display_config.c
extern lv_display_t *display;         // Defined in display_config.c
extern lv_obj_t *disp_obj;
extern bool check_in_successful;

extern bool idIsValid;         // Flag set by database query results
extern bool isAdministrator;   // Flag set by database query results
extern bool isCheckIn;         // Flag set by database query results
extern bool keypadEnteredFlag; // Flag set by keypad task
extern bool nfcReadFlag;       // Flag set by nfc task
extern bool keypadEntered;     // Flag set by keypad driver

extern char nfcUserID[MAX_ID_LEN]; // Used to store the ID read from NFC

void init_kiosk_tasks();
void teardown_kiosk_tasks();

#endif