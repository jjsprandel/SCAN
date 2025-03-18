#ifndef keypad_driver_h
#define keypad_driver_h


#include <freertos/task.h>
#include <esp_log.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <driver/timer.h>
#include "freertos/FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c_master.h"

#define _KP
#define DEBOUNCE_PERIOD_MS 150
#define MAX_BUFFER_SIZE 21
#define BUFFER_TIMEOUT 2
#define TIMER_DEVIDER 16

#define KEYPAD_DEBUG 1

#define PCF8574N_I2C_ADDR   0x20
#define KEYPAD_ADDRESS 0x20
#define ID_LEN 10

typedef struct
{
    char elements[MAX_BUFFER_SIZE];
    uint8_t occupied;
} keypad_buffer_t;


extern i2c_master_dev_handle_t pcf8574n_i2c_handle;
extern i2c_device_config_t pcf8574n_i2c_config;


extern keypad_buffer_t keypad_buffer;
extern EventGroupHandle_t event_group;
extern TaskHandle_t state_control_task_handle;

char poll_keypad(uint8_t keypad_address);

void init_timer();

void clear_buffer();

void add_to_buffer(char val);

void keypad_handler(void *params);

#endif