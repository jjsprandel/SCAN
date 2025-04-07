#include "cypd3177.h"

static const char *TAG = "CYPD3177_INTR";

// GPIO pin config
gpio_config_t cypd3177_intr_config = {
    .intr_type = GPIO_INTR_NEGEDGE, // Trigger on falling edge (active low)
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << CYPD3177_INTR_PIN),
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
};

// ISR: Notify the task
void IRAM_ATTR cypd3177_isr_handler(void *arg)
{
    ESP_LOGI(TAG, "Interrupt triggered");
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // check the interrupt pin level after interrupt is triggered to double check that the interrupt signal is still active (debouncing/transient handling)
    if (gpio_get_level(CYPD3177_INTR_PIN) == 0)
    {
        vTaskNotifyGiveFromISR(cypd3177_task_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);  // Yield if needed
}
