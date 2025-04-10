#include "power_mgmt.h"
#include "mqtt.h"
#include "../../main/include/global.h"
#include "../database/include/kiosk_firebase.h"
#include "../device_drivers/bq25798/include/bq25798.h"
#include "../device_drivers/cypd3177/include/cypd3177.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/semphr.h"

static const char *TAG = "POWER_MGMT";

// Task handles
static TaskHandle_t power_mgmt_task_handle = NULL;
static TaskHandle_t bq25798_monitor_task_handle = NULL;

// External declaration of the mutex from bq25798.c
extern SemaphoreHandle_t device_info_mutex;

// External declarations of voltage values from bq25798.c
extern uint16_t vsysmin_mv;  // Minimum system voltage in mV (10.5V = 10500mV)
extern uint16_t vreg_mv;     // Maximum charge voltage in mV (14.0V = 14000mV)

// Initialize power management
esp_err_t power_mgmt_init(void)
{
    ESP_LOGI(TAG, "Initializing power management");
    
    // Create BQ25798 monitor task with higher priority
    xTaskCreate(
        bq25798_monitor_task, 
        "bq25798_monitor_task", 
        4096, 
        NULL, 
        3,  // Higher priority
        &bq25798_monitor_task_handle
    );

    ESP_LOGI(TAG, "BQ25798 monitor task created");
    
    // Create power management task with increased stack size for SSL/TLS operations
    xTaskCreate(
        power_mgmt_task, 
        "power_mgmt_task", 
        8192, 
        NULL, 
        2,  // Lower priority
        &power_mgmt_task_handle
    );
    
    ESP_LOGI(TAG, "Power management task created");
    return ESP_OK;
}

// Power management task
void power_mgmt_task(void *pvParameters)
{
    power_state_t current_state = POWER_STATE_INIT;
    power_state_t new_state;
    int update_counter = 0;
    
    ESP_LOGI(TAG, "Power management task started");
    
    while (1) {
        // Get current power state based on device_info
        if (xSemaphoreTake(device_info_mutex, portMAX_DELAY) == pdTRUE) {
            
            // Calculate battery percentage based on voltage
            // Linear interpolation between min and max voltage
            uint16_t battery_voltage_mv = (uint16_t)(device_info.battery_voltage_volts * 1000.0f);  // Convert V to mV
            
            if (battery_voltage_mv <= vsysmin_mv) {
                device_info.battery_percentage = 0;
            } else if (battery_voltage_mv >= vreg_mv) {
                device_info.battery_percentage = 100;
            } else {
                // Use integer arithmetic with proper order of operations to avoid precision loss
                // (battery_voltage_mv - vsysmin_mv) * 100 / (vreg_mv - vsysmin_mv)
                uint16_t voltage_range = vreg_mv - vsysmin_mv;
                uint16_t voltage_above_min = battery_voltage_mv - vsysmin_mv;
                device_info.battery_percentage = (uint8_t)((voltage_above_min * 100) / voltage_range);
            }
            
            //ESP_LOGI(TAG, "Battery voltage: %.2fV, Percentage: %d%%", device_info.battery_voltage_volts, device_info.battery_percentage);

            // Check charge state and is_charging flag
            if (device_info.charge_state >= 1 && device_info.charge_state <= 4 && device_info.charge_current_amps > 0.0f && device_info.input_voltage_volts > 0.0f) {
                new_state = POWER_STATE_CHARGING;
                // Enable high power charging when entering charging state
                if (current_state != POWER_STATE_CHARGING) {
                    ESP_LOGI(TAG, "Entering charging state, enabling high power charging");
                    if (enable_high_power_charging() != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to enable high power charging");
                    }
                }
            } else if (device_info.charge_state == 7 && device_info.charge_current_amps == 0) {
                //device_info.battery_percentage = 100;
                new_state = POWER_STATE_FULLY_CHARGED;
            } else if (device_info.input_voltage_volts > 0.0f) {
                new_state = POWER_STATE_POWERED;
            } else {
                new_state = POWER_STATE_BATTERY;
            }

            xSemaphoreGive(device_info_mutex);

            // Update Firebase periodically (every 120 seconds) or on state changes
            update_counter++;
            if (update_counter >= 60 || new_state != current_state) {  // 60 iterations * 2 second delay = 120 seconds
                
                
                if (!kiosk_firebase_update_power_info()) {
                    ESP_LOGE(TAG, "Failed to update power info in Firebase");
                }
                
                if (update_counter >= 60) {
                    update_counter = 0;
                }
                if (new_state != current_state) {
                    current_state = new_state;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Check every 2 seconds
    }
} 