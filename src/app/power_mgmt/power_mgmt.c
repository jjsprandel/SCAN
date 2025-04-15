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
#include "esp_timer.h"
#include <math.h>

static const char *TAG = "POWER_MGMT";

// Task handles
static TaskHandle_t power_mgmt_task_handle = NULL;
static TaskHandle_t bq25798_monitor_task_handle = NULL;

// Battery state tracking
static bool was_charging = false;
static uint8_t last_battery_percentage = 0;
static uint32_t last_percentage_update_time = 0;
static const uint32_t MIN_UPDATE_INTERVAL_MS = 1000; // 1 second
static const uint8_t MAX_PERCENTAGE_CHANGE = 2; // Max 2% change per update
static const uint8_t FAST_CHARGE_PERCENTAGE_CHANGE = 5; // Max 5% change when charging fast
static uint32_t last_voltage_mv = 0;
static uint8_t stable_percentage = 0;
static const uint16_t SIGNIFICANT_VOLTAGE_CHANGE = 100; // 100mV change is significant
static bool first_percentage_update = true; // Flag to track first percentage update


// External declaration of the mutex from bq25798.c
extern SemaphoreHandle_t device_info_mutex;

// External declarations of voltage values from bq25798.c
extern const uint16_t vsysmin_mv;  // Minimum system voltage in mV (10.5V = 10500mV)
extern const uint16_t vreg_mv;     // Maximum charge voltage in mV (14.0V = 14000mV)

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
            
            // Get current time for rate limiting
            uint32_t current_time = esp_timer_get_time() / 1000; // Convert to milliseconds
            
            // Determine if we're currently charging
            bool is_charging = (device_info.charge_state >= 1 && device_info.charge_state <= 4 && 
                               device_info.charge_current_amps > 0.2f && 
                               device_info.input_voltage_volts > 0.0f);
            
            // Calculate battery percentage based on voltage with improved algorithm
            uint16_t battery_voltage_mv = (uint16_t)(device_info.battery_voltage_volts * 1000.0f);  // Convert V to mV
            uint8_t calculated_percentage;
            
            // Special case: fully charged
            if (device_info.charge_state == 7 && device_info.charge_current_amps == 0) {
                calculated_percentage = 100;
            }
            // Special case: voltage too low
            else if (battery_voltage_mv <= vsysmin_mv) {
                calculated_percentage = 0;
            }
            // Special case: voltage at or above max
            else if (battery_voltage_mv >= vreg_mv) {
                calculated_percentage = 100;
            }
            // Normal case: calculate percentage based on voltage
            else {
                // Calculate percentage using a non-linear curve to better match LiFePO4 behavior
                // This helps prevent jumps when charging starts
                
                // First, calculate a base percentage using the voltage
                uint16_t voltage_range = vreg_mv - vsysmin_mv;
                uint16_t voltage_above_min = battery_voltage_mv - vsysmin_mv;
                
                // Use a non-linear curve (square root) to flatten the middle range
                // This makes the percentage less sensitive to voltage changes in the middle range
                float normalized_voltage = (float)voltage_above_min / (float)voltage_range;
                float adjusted_percentage = sqrtf(normalized_voltage) * 100.0f;
                
                calculated_percentage = (uint8_t)adjusted_percentage;
                
                // Clamp to valid range
                if (calculated_percentage > 100) calculated_percentage = 100;
                // No need to check for < 0 since uint8_t is always >= 0
            }
            
            // Apply smoothing to prevent jumps
            // Only update if enough time has passed since last update
            if (current_time - last_percentage_update_time >= MIN_UPDATE_INTERVAL_MS) {
                // Detect charging state change
                bool charging_state_changed = (is_charging != was_charging);
                was_charging = is_charging;
                
                // If charging state just changed, use the last stable percentage
                if (charging_state_changed) {
                    // When charging starts, keep the last stable percentage for a while
                    // This prevents the percentage from jumping up immediately
                    if (is_charging) {
                        calculated_percentage = stable_percentage;
                    }
                } else {
                    // Check if voltage has increased significantly (charging)
                    bool significant_voltage_increase = (battery_voltage_mv > last_voltage_mv + SIGNIFICANT_VOLTAGE_CHANGE);
                    
                    // Determine the maximum allowed change based on conditions
                    uint8_t max_change = MAX_PERCENTAGE_CHANGE;
                    if (is_charging && significant_voltage_increase) {
                        max_change = FAST_CHARGE_PERCENTAGE_CHANGE;
                    }
                    
                    // Skip percentage change limiting on first update
                    if (!first_percentage_update) {
                        // Normal operation - limit the rate of change
                        if (calculated_percentage > last_battery_percentage + max_change) {
                            calculated_percentage = last_battery_percentage + max_change;
                        } else if (calculated_percentage < last_battery_percentage - MAX_PERCENTAGE_CHANGE) {
                            calculated_percentage = last_battery_percentage - MAX_PERCENTAGE_CHANGE;
                        }
                    } else {
                        // First update - set stable percentage directly
                        stable_percentage = calculated_percentage;
                        first_percentage_update = false;
                    }
                    
                    // Update stable percentage if voltage hasn't changed much
                    if (abs((int)battery_voltage_mv - (int)last_voltage_mv) < 50) {
                        stable_percentage = calculated_percentage;
                    }
                }
                
                // Update tracking variables
                last_battery_percentage = calculated_percentage;
                last_percentage_update_time = current_time;
                last_voltage_mv = battery_voltage_mv;
                
                // Update device info
                device_info.battery_percentage = calculated_percentage;
            }
            
            //ESP_LOGI(TAG, "Battery voltage: %.2fV, Percentage: %d%%", device_info.battery_voltage_volts, device_info.battery_percentage);

            // Check charge state and is_charging flag
            if (is_charging) {
                new_state = POWER_STATE_CHARGING;

                // Enable high power charging when entering charging state
/*                 if (current_state != POWER_STATE_CHARGING) {
                    ESP_LOGI(TAG, "Entering charging state, enabling high power charging");
                    if (enable_high_power_charging() != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to enable high power charging");
                    } 
                   
                } */
            } else if (device_info.charge_state == 7 && device_info.charge_current_amps == 0) {
                new_state = POWER_STATE_FULLY_CHARGED;
            } else if (device_info.input_voltage_volts > 0.0f) {
                new_state = POWER_STATE_POWERED;
            } else {
                new_state = POWER_STATE_BATTERY;
            }

            xSemaphoreGive(device_info_mutex);

            // Update Firebase periodically (every 120 seconds) or on state changes
            update_counter++;
            if (update_counter == 10 || new_state != current_state) {  // 60 iterations * 2 second delay = 120 seconds
                
                
                if (!kiosk_firebase_update_power_info()) {
                    ESP_LOGE(TAG, "Failed to update power info in Firebase");
                }
                
                if (new_state != current_state) {
                    current_state = new_state;
                }
            }
            if (update_counter >= 20) {
                update_counter = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Check every 2 seconds
    }
} 