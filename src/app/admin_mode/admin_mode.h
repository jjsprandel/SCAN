#ifndef ADMIN_MODE_H
#define ADMIN_MODE_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ntag_reader.h"
#include "pn532.h"
#include "esp_heap_task_info.h"
#include "keypad_driver.h"
#include "firebase_utils.h"

#define ADMIN_DEBUG
#define DATABASE_QUERY_ENABLED

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

extern keypad_buffer_t keypad_buffer; // Defined in keypad_driver.c
extern TaskHandle_t state_control_task_handle;
extern admin_state_t current_admin_state;
extern admin_state_t prev_admin_state;
extern UserInfo user_info_instance;
extern UserInfo *user_info;
extern TaskHandle_t keypad_task_handle;
void admin_mode_control_task(void *param);
#endif // ADMIN_MODE_H