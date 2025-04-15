#include "global.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_app_desc.h"
#include <string.h>

static const char *TAG = "GLOBAL";

// Global device info instance
device_info_t device_info;

void init_device_info(void)
{
    // Get MAC address
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    // Create string representation of MAC (without colons)
    snprintf(device_info.mac_addr, sizeof(device_info.mac_addr), 
             "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    // Set kiosk name based on MAC address
    if (strcmp(device_info.mac_addr, "ccba97e1dd80") == 0) {
        strncpy(device_info.kiosk_name, "Kiosk 1", sizeof(device_info.kiosk_name) - 1);
        strncpy(device_info.kiosk_location, "UCF Library", sizeof(device_info.kiosk_location) - 1);
    } else if (strcmp(device_info.mac_addr, "ccba97e1dd84") == 0) {
        strncpy(device_info.kiosk_name, "Kiosk 2", sizeof(device_info.kiosk_name) - 1);
        strncpy(device_info.kiosk_location, "UCF RWC", sizeof(device_info.kiosk_location) - 1);
    } else {
        strncpy(device_info.kiosk_name, "Kiosk 3", sizeof(device_info.kiosk_name) - 1);
        strncpy(device_info.kiosk_location, "UCF Arena", sizeof(device_info.kiosk_location) - 1);
    }
    device_info.kiosk_name[sizeof(device_info.kiosk_name) - 1] = '\0';
    device_info.kiosk_location[sizeof(device_info.kiosk_location) - 1] = '\0';
    
    // Get firmware version from app description
    const esp_app_desc_t *app_desc = esp_app_get_description();
    if (app_desc != NULL) {
        strncpy(device_info.firmware_version, app_desc->version, sizeof(device_info.firmware_version) - 1);
    } else {
        // Fallback to default version if app description can't be retrieved
        strncpy(device_info.firmware_version, "Unknown", sizeof(device_info.firmware_version) - 1);
    }
    device_info.firmware_version[sizeof(device_info.firmware_version) - 1] = '\0';
    
    // Initialize power information with default values
    device_info.charge_current_amps = 0.0f;
    device_info.battery_voltage_volts = 0.0f;
    device_info.input_voltage_volts = 0.0f;
    device_info.is_charging = false;
    
    // Initialize WiFi SSID (empty string)
    memset(device_info.wifi_ssid, 0, sizeof(device_info.wifi_ssid));
    
    ESP_LOGI(TAG, "Device info initialized with MAC: %s, firmware version: %s, kiosk name: %s", 
             device_info.mac_addr, device_info.firmware_version, device_info.kiosk_name);
}
