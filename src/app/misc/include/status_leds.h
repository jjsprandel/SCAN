#ifndef STATUS_LEDS_H
#define STATUS_LEDS_H

#include "led_strip.h"
#include "state_enum.h"

#define BLINK_GPIO 8
#define NUM_LEDS 3

void configure_led();
void set_kiosk_leds(state_t current_state, bool led_toggle);

#endif // STATUS_LEDS_H