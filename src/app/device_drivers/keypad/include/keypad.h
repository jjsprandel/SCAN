#ifndef keypad_driver_h
#define keypad_driver_h

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_log.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/task.h>
#include <driver/timer.h>
#include <stdbool.h>

#include "pcf8574n.h"

#define _KP
#define DEBOUNCE_PERIOD_MS 150
#define MAX_BUFFER_SIZE 21
#define BUFFER_TIMEOUT 2
#define TIMER_DEVIDER 16

#define KEYPAD_DEBUG 1

#define ID_LEN 10
#define USING_MAIN_PCB 1
#ifdef USING_MAIN_PCB
#define KEYPAD_I2C_SDA GPIO_NUM_4
#define KEYPAD_I2C_SCL GPIO_NUM_5
#else
#define KEYPAD_I2C_SDA GPIO_NUM_1
#define KEYPAD_I2C_SCL GPIO_NUM_0
#endif

typedef struct
{
    char elements[MAX_BUFFER_SIZE];
    uint8_t occupied;
} keypad_buffer_t;

extern keypad_buffer_t keypad_buffer;
extern EventGroupHandle_t event_group;
extern TaskHandle_t state_control_task_handle;
extern TaskHandle_t admin_mode_control_task_handle;

extern void set_pcf_pins(uint8_t pin_config);

extern void read_pcf_pins(uint8_t *pin_states);

void init_timer();

void clear_buffer();

void add_to_buffer(char val);

uint8_t detect_active_row();

uint8_t detect_active_column();

char get_active_key();

void keypad_handler(void *params);

#endif