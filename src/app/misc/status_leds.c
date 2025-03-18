#include "status_leds.h"

static led_strip_handle_t led_strip;

void configure_led(void)
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = NUM_LEDS, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void set_kiosk_leds(state_t current_state, bool led_toggle)
{
    // Clear the LED strip before setting new colors
    led_strip_clear(led_strip);

    // Set LED colors based on the current state
    switch (current_state)
    {
    case STATE_WIFI_INIT:
        for (int i = 0; i < NUM_LEDS; i++)
            led_strip_set_pixel(led_strip, i, (led_toggle ? 100 : 0), 0, 0); // Red
        break;

    case STATE_USER_DETECTED:
        led_strip_set_pixel(led_strip, 0, 0, 0, 100); // Blue
        break;

    case STATE_DATABASE_VALIDATION:
        led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
        led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
        break;

    case STATE_CHECK_IN:
    case STATE_CHECK_OUT:
        led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
        led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
        led_strip_set_pixel(led_strip, 2, 0, 100, 0);   // Green
        break;

    case STATE_VALIDATION_FAILURE:
        led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
        led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
        led_strip_set_pixel(led_strip, 2, 100, 0, 0);   // Red
        break;

    case STATE_IDLE:
        led_strip_clear(led_strip); // Turn off all LEDs
        break;

    default:
        led_strip_clear(led_strip); // Turn off all LEDs
        break;
    }

    // Refresh the LED strip to update the changes
    led_strip_refresh(led_strip);
}
