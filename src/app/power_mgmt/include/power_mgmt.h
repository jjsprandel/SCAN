#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../device_drivers/bq25798/include/bq25798.h"

// Power states
typedef enum {
    POWER_STATE_INIT,            // Initial state, no functionality
    POWER_STATE_BATTERY,         // Running on battery only
    POWER_STATE_POWERED,         // USB connected but not charging
    POWER_STATE_CHARGING,        // Device is charging
    POWER_STATE_FULLY_CHARGED    // Battery is fully charged
} power_state_t;

// Function prototypes
void power_mgmt_task(void *pvParameters);
esp_err_t update_power_state(power_state_t state);
esp_err_t update_power_info(void);
esp_err_t power_mgmt_init(void);

#endif /* POWER_MGMT_H */ 