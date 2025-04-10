#ifndef KIOSK_FIREBASE_H
#define KIOSK_FIREBASE_H

#include <stdbool.h>
#include <stdint.h>
#include "../../main/include/global.h"

/**
 * @brief Update kiosk power information in Firebase database
 * 
 * This function updates the kiosk's power information in the Firebase database
 * using the device_info structure. It formats the data as JSON and sends it
 * to the Firebase Realtime Database.
 * 
 * @return bool true if update was successful, false otherwise
 */
bool kiosk_firebase_update_power_info(void);

/**
 * @brief Initialize kiosk Firebase client
 * 
 * This function initializes the Firebase client for kiosk management.
 * 
 * @return bool true if initialization was successful, false otherwise
 */
bool kiosk_firebase_init(void);

/**
 * @brief Notify that the network is ready
 * 
 * This function should be called when the network stack is fully initialized
 * to complete the Firebase client initialization.
 */
void kiosk_firebase_network_ready(void);

#endif /* KIOSK_FIREBASE_H */ 