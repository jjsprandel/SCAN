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

    led_strip_clear(led_strip);
}

void set_kiosk_leds(state_t current_state, admin_state_t current_admin_state, bool led_toggle)
{
    led_strip_clear(led_strip);

    if (current_state == STATE_ADMIN_MODE) {
        switch (current_admin_state) {
            case ADMIN_STATE_BEGIN:
                led_strip_clear(led_strip);
                break;

            case ADMIN_STATE_ENTER_ID:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 0, 0, led_toggle ? 100 : 0);
                }
                break;

            case ADMIN_STATE_VALIDATE_ID:
                led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
                led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
                break;

            case ADMIN_STATE_TAP_CARD:
                led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
                led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
                led_strip_set_pixel(led_strip, 2, 0, 100, 0);   // Green
                break;

            case ADMIN_STATE_CARD_WRITE_SUCCESS:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 0, 100, 0);
                }
                break;

            case ADMIN_STATE_ENTER_ID_ERROR:
            case ADMIN_STATE_CARD_WRITE_ERROR:
                led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
                led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
                led_strip_set_pixel(led_strip, 2, 100, 0, 0);   // Red
                break;

            case ADMIN_STATE_ERROR:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 100, 0, 0);
                }
                break;

            default:
                led_strip_clear(led_strip);
                break;
        }
    } else {
        switch (current_state) {
            case STATE_HARDWARE_INIT:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, led_toggle ? 100 : 0, 0, 0);
                }
                break;

            case STATE_WIFI_CONNECTING:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 0, 0, led_toggle ? 100 : 0);
                }
                break;

            case STATE_SOFTWARE_INIT:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 0, led_toggle ? 100 : 0, 0);
                }
                break;

            case STATE_SYSTEM_READY:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 0, led_toggle ? 100 : 0, 0);
                }
                break;

            case STATE_IDLE:
                led_strip_clear(led_strip);
                break;

            case STATE_USER_DETECTED:
                led_strip_set_pixel(led_strip, 0, 0, 0, 100);
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

            case STATE_KEYPAD_ENTRY_ERROR:
            case STATE_VALIDATION_FAILURE:
                led_strip_set_pixel(led_strip, 0, 0, 0, 100);  // Blue
                led_strip_set_pixel(led_strip, 1, 100, 100, 0); // Yellow
                led_strip_set_pixel(led_strip, 2, 100, 0, 0);   // Red
                break;

            case STATE_ERROR:
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, 100, 0, 0);
                }
                break;

            default:
                led_strip_clear(led_strip);
                break;
        }
    }

    led_strip_refresh(led_strip);
}
