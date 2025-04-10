#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdbool.h>

// Device information structure
typedef struct {
    // Device identification
    char mac_addr[32];           // MAC address as string
    char firmware_version[16];    // Current firmware version
    
    // Power information
    float charge_current_amps;    // Battery charge current in amperes
    float battery_voltage_volts;  // Battery voltage in volts
    float input_voltage_volts;    // Input voltage in volts
    bool is_charging;             // Charge status (true if charging)
    
    // WiFi information
    char wifi_ssid[32];           // WiFi SSID
} device_info_t;

// Global device info instance
extern device_info_t device_info;

// Function declarations
void init_device_info(void);

#endif // GLOBAL_H 