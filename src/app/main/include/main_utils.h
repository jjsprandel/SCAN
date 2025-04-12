#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "esp_timer.h"
#include "esp_log.h"

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define HEAP_WARNING_THRESHOLD 5000  // Minimum acceptable heap size in bytes
#define MONITOR_TASK_STACK_SIZE 2048 // Stack size in words (4 bytes each)
#define MONITOR_TASK_PRIORITY 5      // Task priority
#define UTILS_DEBUG 1
#define HEAP_MONITOR_INTERVAL 2000 // Interval in milliseconds

bool is_valid_id_string(const char *str, size_t max_len);
bool is_numeric_string(const char *str, size_t max_len);
void log_elapsed_time(char *auth_type, int64_t start_time);
void heap_monitor_task(void *pvParameters);
void check_task_creation(char *taskName, TaskHandle_t taskHandle);
void teardown_task(TaskHandle_t *taskHandle);
#endif