#ifndef STATE_ENUM_H
#define STATE_ENUM_H

typedef enum
{
    STATE_HARDWARE_INIT,   // Initialize hardware components (GPIO, I2C, etc.)
    STATE_WIFI_CONNECTING, // Connect to WiFi
    STATE_SOFTWARE_INIT,   // Initialize software services that need WiFi
    STATE_SYSTEM_READY,    // System is fully initialized
    STATE_IDLE,
    STATE_USER_DETECTED,
    STATE_DATABASE_VALIDATION,
    STATE_CHECK_IN,
    STATE_CHECK_OUT,
    STATE_ADMIN_MODE,
    STATE_KEYPAD_ENTRY_ERROR,
    STATE_VALIDATION_FAILURE,
    STATE_ERROR
} state_t;

#endif // STATE_ENUM_H