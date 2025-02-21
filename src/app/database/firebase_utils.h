#ifndef FIREBASE_UTILS_H
#define FIREBASE_UTILS_H

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_tls.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "firebase_http_client.h"

#define ID_AUTHENTICATED_BIT BIT2

// Struct to store user information
typedef struct
{
    bool active_student;
    bool check_in_status;
    char location[64];
    char role[32];
    char passkey[64];
} UserInfo;

// Struct for mapping Firebase fields to struct fields
typedef struct
{
    const char *key;
    void *field;
    size_t field_size;
} UserFieldMapping;

extern UserInfo *user_info;

extern EventGroupHandle_t event_group;
extern bool check_in_successful;

void check_in_user_task(void *pvParameters);
bool check_in_user(char *user_id);

#endif