#ifndef MAIN_H
#define MAIN_H

#include "led_strip.h"
#include "keypad_driver.h"
#include "wifi_init.h"
#include "display_config.h"
#include "kiosk_tasks.h"
#include "buzzer.h"
#include "admin_tasks.h"
#include "display_config.h"
#include "display_frames.h"

#define HEAP_WARNING_THRESHOLD 5000  // Minimum acceptable heap size in bytes
#define MONITOR_TASK_STACK_SIZE 2048 // Stack size in words (4 bytes each)
#define MONITOR_TASK_PRIORITY 5      // Task priority
// #define MAIN_HEAP_DEBUG
#define MAIN_DEBUG

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

extern char user_id[ID_LEN];
extern bool isAdministrator;

#endif