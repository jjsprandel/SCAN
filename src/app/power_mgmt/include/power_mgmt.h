#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bq25798.h"

// Power management states
typedef enum {
    POWER_STATE_INIT = 0,
    POWER_STATE_BATTERY_ONLY,
    POWER_STATE_BATTERY_USB_NOT_CHARGING,
    POWER_STATE_BATTERY_USB_CHARGING,
    POWER_STATE_BATTERY_USB_FULLY_CHARGED
} power_state_t;

// Function prototypes
void power_mgmt_task(void *pvParameters);
esp_err_t power_mgmt_init(void);

#endif /* POWER_MGMT_H */ 