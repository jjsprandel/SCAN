#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_heap_task_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "wifi_init.h"
#include "ota.h"
#include "firebase_utils.h"
#include "ntag_reader.h"
#include "firebase_http_client.h"
#include "keypad_driver.h"
#include "display_config.h"
#include "display_frames.h"
#include "pir_sensor.h"
#include "state_enum.h"
#include "status_buzzer.h"

#define ID_LEN 10
#define BLINK_GPIO 8
#define MAIN_DEBUG 1
#define NUM_LEDS 3

pn532_t nfc;           // Defined in ntag_reader.h
_lock_t lvgl_api_lock; // Defined in display_config.h
lv_display_t *display; // Defined in display_config.h
lv_obj_t *screen_objects[STATE_ERROR] = {NULL};

extern UserInfo user_info_instance;
extern UserInfo *user_info;
extern TaskHandle_t state_control_task_handle;

char user_id[ID_LEN];
void state_control_task(void *pvParameter);
void blink_led_1_task(void *pvParameter);
void blink_led_2_task(void *pvParameter);
