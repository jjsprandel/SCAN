#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdbool.h>

// Device information structure
typedef struct {
    // Device identification
    char mac_addr[18];           // MAC address as string
    char firmware_version[32];    // Current firmware version
    char kiosk_name[16];
    char kiosk_location[16];
    
    // Power information
    float charge_current_amps;    // Battery charge current in amperes
    float battery_voltage_volts;  // Battery voltage in volts
    float input_voltage_volts;    // Input voltage in volts
    float input_current_amps;     // Input current in amperes
    bool is_charging;             // Charge status (true if charging)
    uint8_t charge_state;         // BQ25798 charge state (0-7)
    uint8_t vbus_status;          // BQ25798 VBUS status
    uint8_t battery_percentage;      // Calculated battery percentage
    
    // WiFi information
    char wifi_ssid[32];           // WiFi SSID
} device_info_t;

// Global device info instance
extern device_info_t device_info;

// Function declarations
void init_device_info(void);

#endif // GLOBAL_H 