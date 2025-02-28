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
#define MAIN_DEBUG 1
#define PROXIMITY_DETECTED_BIT BIT0
#define ID_ENTERED_SUCCESS_BIT BIT1
#define ID_AUTHENTICATED_BIT BIT2
#define ADMIN_MODE_BIT BIT3
#define IDLE_BIT BIT4
#define ENTERING_ID_BIT BIT5
#define NEW_ID_ENTERED_SUCCESS_BIT BIT6
#define ID_LEN 7

typedef enum
{
    STATE_WIFI_INIT,
    STATE_WIFI_READY,
    STATE_IDLE,
    STATE_USER_DETECTED,
    STATE_VALIDATING,
    STATE_VALIDATION_SUCCESS,
    STATE_VALIDATION_FAILURE,
    STATE_ADMIN,
    STATE_ERROR
} state_t;

// extern EventGroupHandle_t event_group;
extern TaskHandle_t keypad_task_handle;

extern state_t current_state, prev_state;

extern keypad_buffer_t keypad_buffer; // Defined in keypad_driver.c
extern _lock_t lvgl_api_lock;         // Defined in display_config.c
extern lv_display_t *display;         // Defined in display_config.c
extern lv_obj_t *disp_obj;
extern bool check_in_successful;

extern bool idIsValid;       // Flag set by database query results
extern bool isAdministrator; // Flag set by database query results
extern bool keypadEnteredFlag;
extern bool nfcReadFlag;

extern char nfcUserID[MAX_ID_LEN]; // Used to store the ID read from NFC

void proximity_task(void *param);
void nfc_scan_id_task(void *param);
void keypad_enter_id_task(void *param);
void validation_task(void *param);
void display_task(void *param);

#endif