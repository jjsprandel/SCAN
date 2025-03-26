#include "cypd3177.h"

// Configure the GPIO pin
gpio_config_t cypd3177_intr_config = {
    .intr_type = GPIO_INTR_NEGEDGE, // Trigger on falling edge (active low)
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << CYPD3177_INTR_PIN),
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
};

static void IRAM_ATTR gpio_isr_handler_1(void *arg)
{
    int gpio_num = CYPD3177_INTR_PIN;
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
