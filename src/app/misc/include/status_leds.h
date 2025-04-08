#ifndef STATUS_LEDS_H
#define STATUS_LEDS_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "state_enum.h"
#include "admin_mode.h"

#define BLINK_GPIO 8
#define NUM_LEDS 3

void configure_led(void);
void set_kiosk_leds(state_t current_state, admin_state_t current_admin_state, bool led_toggle);

#endif // STATUS_LEDS_H