#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_io_interface.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#define EXAMPLE_LCD_HOST SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define TEST_LCD_BIT_PER_PIXEL 16
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define USING_MAIN_PCB 1

#ifdef USING_MAIN_PCB
#define EXAMPLE_PIN_NUM_SCLK 7
#define EXAMPLE_PIN_NUM_MOSI 6
#define EXAMPLE_PIN_NUM_LCD_DC 12
#define EXAMPLE_PIN_NUM_LCD_RST 10
#define EXAMPLE_PIN_NUM_LCD_CS 11
#else
#define EXAMPLE_PIN_NUM_SCLK 12
#define EXAMPLE_PIN_NUM_MOSI 13
#define EXAMPLE_PIN_NUM_LCD_DC 15
#define EXAMPLE_PIN_NUM_LCD_RST 21
#define EXAMPLE_PIN_NUM_LCD_CS 18
#endif

#define EXAMPLE_PIN_NUM_BK_LIGHT 2
#define EXAMPLE_PIN_NUM_TOUCH_CS 15

#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8
#define TEST_DELAY_TIME_MS (3000)

#define EXAMPLE_LVGL_DRAW_BUF_LINES 20 // Changed from 20 to 10// number of display lines in each draw buffer
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

#define CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01 1
// #define DISPLAY_CONFIG_DEBUG

extern _lock_t lvgl_api_lock;
extern lv_display_t *display;

bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
void example_lvgl_port_update_callback(lv_display_t *disp);
void example_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void example_increase_lvgl_tick(void *arg);
void lvgl_port_task(void *arg);
void gc9a01_init();

#endif