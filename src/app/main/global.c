#include "global.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_log.h"
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
    
    // Get chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    device_info.chip_model = chip_info.model;
    device_info.chip_revision = chip_info.revision;
    device_info.chip_cores = chip_info.cores;
    device_info.chip_features = chip_info.features;
    
    // Set firmware version (using a default value for now)
    strncpy(device_info.firmware_version, "1.0.0", sizeof(device_info.firmware_version) - 1);
    device_info.firmware_version[sizeof(device_info.firmware_version) - 1] = '\0';
    
    // Set active status
    strncpy(device_info.active, "true", sizeof(device_info.active) - 1);
    device_info.active[sizeof(device_info.active) - 1] = '\0';
    
    // WiFi SSID is not set yet
    
    ESP_LOGI(TAG, "Device info initialized:");
    ESP_LOGI(TAG, "  MAC Address: %s", device_info.mac_addr);
    ESP_LOGI(TAG, "  Firmware Version: %s", device_info.firmware_version);
    ESP_LOGI(TAG, "  Active Status: %s", device_info.active);
    ESP_LOGI(TAG, "  WiFi SSID: %s", device_info.wifi_ssid[0] ? device_info.wifi_ssid : "Not set");
    ESP_LOGI(TAG, "  Chip Model: %d", device_info.chip_model);
    ESP_LOGI(TAG, "  Chip Revision: %lu", device_info.chip_revision);
    ESP_LOGI(TAG, "  Chip Cores: %d", device_info.chip_cores);
    ESP_LOGI(TAG, "  Chip Features: 0x%x", device_info.chip_features);
}
