#ifndef OTA_H
#define OTA_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Function declarations
void ota_update_fw_task(void *pvParameter);
void advanced_ota_example_task(void *pvParameter);

#endif // OTA_H