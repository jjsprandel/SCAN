#include "global.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
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
    
    // Set firmware version
    strncpy(device_info.firmware_version, "1.0.0", sizeof(device_info.firmware_version) - 1);
    device_info.firmware_version[sizeof(device_info.firmware_version) - 1] = '\0';
    
    // Initialize power information with default values
    device_info.charge_current_amps = 0.0f;
    device_info.battery_voltage_volts = 0.0f;
    device_info.input_voltage_volts = 0.0f;
    device_info.is_charging = false;
    
    // Initialize WiFi SSID (empty string)
    memset(device_info.wifi_ssid, 0, sizeof(device_info.wifi_ssid));
    
    ESP_LOGI(TAG, "Device info initialized with MAC: %s, firmware version: %s", 
             device_info.mac_addr, device_info.firmware_version);
}
