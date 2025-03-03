#ifndef ADMIN_TASKS_H
#define ADMIN_TASKS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ntag_reader.h"
#include "pn532.h"
#include "esp_heap_task_info.h"
#include "keypad_driver.h"
#include "admin_display_frames.h"

#define ADMIN_DEBUG
// #define DATABASE_ENABLED

// From admin_event_group
#define ADMIN_READY_BIT BIT0
#define ADMIN_ID_ENTERED_BIT BIT1
#define ADMIN_VALIDATION_RESPONSE_BIT BIT2
#define ADMIN_TAP_CARD_BIT BIT3
#define ADMIN_CARD_WRITTEN_BIT BIT4

// From main event_group
#define EXIT_ADMIN_MODE_BIT BIT3
#define ID_ENTERED_SUCCESS_BIT BIT1

extern EventGroupHandle_t event_group;
extern keypad_buffer_t keypad_buffer; // Defined in keypad_driver.c
extern char nfcUserID[MAX_ID_LEN];
extern _lock_t lvgl_api_lock; // Defined in display_config.c
extern lv_display_t *display; // Defined in display_config.c
extern lv_obj_t *disp_obj;

typedef enum
{
    ADMIN_STATE_BEGIN,
    ADMIN_STATE_ENTER_ID,
    ADMIN_STATE_VALIDATE_ID,
    ADMIN_STATE_TAP_CARD,
    ADMIN_STATE_CARD_WRITE_SUCCESS,
    ADMIN_STATE_ENTER_ID_ERROR,
    ADMIN_STATE_CARD_WRITE_ERROR,
    ADMIN_STATE_ERROR
} admin_state_t;

void init_admin_mode();
void teardown_admin_mode();

#endif