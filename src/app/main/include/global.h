#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include "esp_chip_info.h"

// Device information structure
typedef struct {
    // Device identification
    char mac_addr[32];       // MAC address as string
    char firmware_version[16]; // Current firmware version
    char active[8];         // Device active status
    char wifi_ssid[32];     // WiFi SSID
    
    // Chip information
    esp_chip_model_t chip_model;
    uint32_t chip_revision;
    uint8_t chip_cores;
    uint8_t chip_features;
} device_info_t;

// Global device info instance
extern device_info_t device_info;

// Function declarations
void init_device_info(void);

#endif // GLOBAL_H 