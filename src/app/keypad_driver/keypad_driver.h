#ifndef keypad_driver_h
#define keypad_driver_h

#include <driver/i2c.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <freertos/task.h>
#include <driver/timer.h>
#include "freertos/FreeRTOS.h"
#include <stdbool.h>

#define _KP
#define DEBOUNCE_PERIOD_MS 150
#define MAX_BUFFER_SIZE 21
#define BUFFER_TIMEOUT 2
#define TIMER_DEVIDER 16

#define KEYPAD_ADDRESS 0x20
#define KEYPAD_TAG "keypad_driver"
#define ID_ENTERED_SUCCESS_BIT BIT1

typedef struct
{
    char elements[MAX_BUFFER_SIZE];
    uint8_t occupied;
} keypad_buffer_t;

extern keypad_buffer_t keypad_buffer;
extern EventGroupHandle_t event_group;

char poll_keypad(uint8_t keypad_address);

void init_timer();

void clear_buffer();

void add_to_buffer(char val);

void keypad_handler(void *params);
esp_err_t i2c_master_init(void);

#endif