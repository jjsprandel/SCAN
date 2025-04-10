#include "power_mgmt.h"
#include "mqtt.h"
#include "../../main/include/global.h"
#include "../database/include/kiosk_firebase.h"

static const char *TAG = "POWER_MGMT";

// Current power state
static power_state_t current_state = POWER_STATE_INIT;

// Task handle
static TaskHandle_t power_mgmt_task_handle = NULL;

// Initialize power management
esp_err_t power_mgmt_init(void)
{
    ESP_LOGI(TAG, "Initializing power management");
    
    // Create power management task with increased stack size for SSL/TLS operations
    BaseType_t ret = xTaskCreate(power_mgmt_task, "power_mgmt_task", 8192, NULL, 4, &power_mgmt_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create power management task");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Power management task created");
    return ESP_OK;
}

// Power management state machine task
void power_mgmt_task(void *pvParameters)
{
    uint16_t battery_voltage;
    uint16_t charge_current;
    uint16_t input_voltage;
    uint8_t charge_status;
    power_state_t new_state;
    power_state_t prev_state = POWER_STATE_INIT;
    
    // State names for logging
    const char* state_names[] = {
        "INIT",
        "BATTERY_ONLY",
        "BATTERY_USB_NOT_CHARGING",
        "BATTERY_USB_CHARGING",
        "BATTERY_USB_FULLY_CHARGED"
    };
    
    ESP_LOGI(TAG, "Power management task started");
    
    while (1) {
        // Read all parameters
        bool battery_voltage_valid = (bq25798_get_battery_voltage(&battery_voltage) == ESP_OK);
        bool charge_current_valid = (bq25798_get_charge_current(&charge_current) == ESP_OK);
        bool input_voltage_valid = (bq25798_get_input_voltage(&input_voltage) == ESP_OK);
        bool charge_status_valid = (bq25798_get_charge_status(&charge_status) == ESP_OK);
        
        // Extract charge status bits (7-5)
        uint8_t charge_state = 0;
        if (charge_status_valid) {
            charge_state = (charge_status >> 5) & 0x07;
        }
        
        // Determine new state based on current readings
        if (!battery_voltage_valid || !input_voltage_valid || !charge_status_valid) {
            // If we can't read critical values, stay in current state
            new_state = current_state;
            ESP_LOGW(TAG, "Unable to read critical values, staying in current state");
        } else {
            // Check if USB is connected (input voltage > 0)
            bool usb_connected = (input_voltage > 0);
            
            // Check if battery is charging (charge current > 0 or charge state indicates charging)
            bool battery_charging = (charge_current > 0) || 
                                   (charge_state == 1) || // Trickle charge
                                   (charge_state == 2) || // Pre-charge
                                   (charge_state == 3) || // Fast-charge
                                   (charge_state == 4);   // Taper charge
            
            // Check if battery is fully charged
            bool battery_fully_charged = (charge_state == 7) && (charge_current == 0); // Charge termination done
            
            // Determine new state based on current readings
            if (!usb_connected) {
                new_state = POWER_STATE_BATTERY_ONLY;
            } else if (battery_fully_charged) {
                new_state = POWER_STATE_BATTERY_USB_FULLY_CHARGED;
            } else if (battery_charging) {
                // Only transition to charging state if charge current is non-zero
                if (charge_current > 0) {
                    new_state = POWER_STATE_BATTERY_USB_CHARGING;
                } else {
                    // Stay in current state if charge current is zero
                    new_state = current_state;
                    ESP_LOGI(TAG, "Charge status indicates charging but current is zero, staying in current state");
                }
            } else {
                // Only transition to not charging state if we're currently in a charging state
                // and charge current is zero
                if ((current_state == POWER_STATE_BATTERY_USB_CHARGING || 
                     current_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) && 
                    charge_current == 0) {
                    new_state = POWER_STATE_BATTERY_USB_NOT_CHARGING;
                } else if (current_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                    // Stay in not charging state
                    new_state = current_state;
                } else {
                    // Default to not charging for other cases
                    new_state = POWER_STATE_BATTERY_USB_NOT_CHARGING;
                }
            }
        }
        
        // Log state transition if state changed
        if (new_state != current_state) {
            ESP_LOGI(TAG, "Power state transition: %s -> %s", 
                     state_names[current_state], state_names[new_state]);
            
            // Send MQTT status message based on state transition
            if (current_state == POWER_STATE_INIT) {
                // Initial state, just report the new state
                if (new_state == POWER_STATE_BATTERY_ONLY) {
                    mqtt_publish_status("Battery Only");
                } else if (new_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                    mqtt_publish_status("USB Connected");
                } else if (new_state == POWER_STATE_BATTERY_USB_CHARGING) {
                    mqtt_publish_status("Charging");
                } else if (new_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) {
                    mqtt_publish_status("Battery Full");
                }
            } else if (current_state == POWER_STATE_BATTERY_ONLY) {
                // Transitioning from battery only
                if (new_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                    mqtt_publish_status("USB Connected");
                } else if (new_state == POWER_STATE_BATTERY_USB_CHARGING) {
                    mqtt_publish_status("Charging");
                } else if (new_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) {
                    mqtt_publish_status("Battery Full");
                }
            } else if (current_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                // Transitioning from USB connected but not charging
                if (new_state == POWER_STATE_BATTERY_ONLY) {
                    mqtt_publish_status("USB Disconnected");
                } else if (new_state == POWER_STATE_BATTERY_USB_CHARGING) {
                    mqtt_publish_status("Charging");
                } else if (new_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) {
                    mqtt_publish_status("Battery Full");
                }
            } else if (current_state == POWER_STATE_BATTERY_USB_CHARGING) {
                // Transitioning from charging
                if (new_state == POWER_STATE_BATTERY_ONLY) {
                    mqtt_publish_status("USB Disconnected");
                } else if (new_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                    mqtt_publish_status("Charging Stopped");
                } else if (new_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) {
                    mqtt_publish_status("Battery Full");
                }
            } else if (current_state == POWER_STATE_BATTERY_USB_FULLY_CHARGED) {
                // Transitioning from fully charged
                if (new_state == POWER_STATE_BATTERY_ONLY) {
                    mqtt_publish_status("USB Disconnected");
                } else if (new_state == POWER_STATE_BATTERY_USB_NOT_CHARGING) {
                    mqtt_publish_status("Charging Stopped");
                } else if (new_state == POWER_STATE_BATTERY_USB_CHARGING) {
                    mqtt_publish_status("Charging");
                }
            }
            
            // Update Firebase database with new power state
            if (kiosk_firebase_update_power_info()) {
                ESP_LOGI(TAG, "Firebase database updated with new power state");
            } else {
                ESP_LOGW(TAG, "Failed to update Firebase database, will retry later");
            }
            
            prev_state = current_state;
            current_state = new_state;
        }
        
        // Always log current state
        ESP_LOGI(TAG, "Current Power State: %s", state_names[current_state]);
        
        // Update device_info structure with current values
        if (battery_voltage_valid) {
            device_info.battery_voltage_volts = battery_voltage / 1000.0f;
        }
        
        if (charge_current_valid) {
            // If input voltage is zero (no cable connected), force charge current to zero
            if (input_voltage_valid && input_voltage == 0) {
                device_info.charge_current_amps = 0.0f;
                ESP_LOGI(TAG, "No input voltage detected, setting charge current to zero");
            } else {
                device_info.charge_current_amps = charge_current / 1000.0f;
            }
        }
        
        if (input_voltage_valid) {
            device_info.input_voltage_volts = input_voltage / 1000.0f;
        }
        
        // Update charging status based on charge state
        if (charge_status_valid) {
            // Extract charge status bits (7-5)
            uint8_t charge_state = (charge_status >> 5) & 0x07;
            
            // Extract VBUS status bits (4-1)
            uint8_t vbus_status = (charge_status >> 1) & 0x0F;
            
            // Update is_charging flag in device_info
            // Only consider charging if input voltage is non-zero and charge current is positive
            device_info.is_charging = (input_voltage > 0) && 
                                     ((charge_state > 0 && charge_state < 7) || (charge_current > 0));
            
            // Interpret charge state
            const char* charge_state_str;
            switch (charge_state) {
                case 0: charge_state_str = "Not Charging"; break;
                case 1: charge_state_str = "Trickle Charge"; break;
                case 2: charge_state_str = "Pre-Charge"; break;
                case 3: charge_state_str = "Fast-Charge (CC Mode)"; break;
                case 4: charge_state_str = "Taper Charge (CV Mode)"; break;
                case 5: charge_state_str = "Reserved"; break;
                case 6: charge_state_str = "Top-Off Timer Active"; break;
                case 7: charge_state_str = "Charge Termination Done"; break;
                default: charge_state_str = "Unknown"; break;
            }
            
            // Interpret VBUS status
            const char* vbus_status_str;
            switch (vbus_status) {
                case 0x0: vbus_status_str = "No Input or BHOT/BCOLD in OTG mode"; break;
                case 0x1: vbus_status_str = "USB SDP (500mA)"; break;
                case 0x2: vbus_status_str = "USB CDP (1.5A)"; break;
                case 0x3: vbus_status_str = "USB DCP (3.25A)"; break;
                case 0x4: vbus_status_str = "HVDCP (1.5A)"; break;
                case 0x5: vbus_status_str = "Unknown Adapter (3A)"; break;
                case 0x6: vbus_status_str = "Non-Standard Adapter (1A/2A/2.1A/2.4A)"; break;
                case 0x7: vbus_status_str = "OTG Mode"; break;
                case 0x8: vbus_status_str = "Not Qualified Adapter"; break;
                case 0x9: vbus_status_str = "Reserved"; break;
                case 0xA: vbus_status_str = "Reserved"; break;
                case 0xB: vbus_status_str = "Device Directly Powered from VBUS"; break;
                case 0xC: vbus_status_str = "Backup Mode"; break;
                case 0xD: vbus_status_str = "Reserved"; break;
                case 0xE: vbus_status_str = "Reserved"; break;
                case 0xF: vbus_status_str = "Reserved"; break;
                default: vbus_status_str = "Unknown"; break;
            }
            
            ESP_LOGI(TAG, "Charge Status: %s, VBUS Status: %s", charge_state_str, vbus_status_str);
        }
        
        // Print device info summary
        ESP_LOGI(TAG, "Device Info Summary:");
        ESP_LOGI(TAG, "  Battery Voltage: %.2f V", device_info.battery_voltage_volts);
        ESP_LOGI(TAG, "  Charge Current: %.3f A", device_info.charge_current_amps);
        ESP_LOGI(TAG, "  Input Voltage: %.2f V", device_info.input_voltage_volts);
        ESP_LOGI(TAG, "  Is Charging: %s", device_info.is_charging ? "Yes" : "No");
        
        // Update Firebase database periodically (every 5 seconds)
        static uint32_t update_counter = 0;
        update_counter++;
        if (update_counter >= 5) { // Update every 5 iterations (25 seconds)
            if (kiosk_firebase_update_power_info()) {
                ESP_LOGI(TAG, "Firebase database updated periodically");
            } else {
                ESP_LOGW(TAG, "Failed to update Firebase database periodically, will retry later");
            }
            update_counter = 0;
        }
        
        // Delay before next check
        vTaskDelay(pdMS_TO_TICKS(5000)); // Update every 5 seconds
    }
} 