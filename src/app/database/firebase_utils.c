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

#include "cJSON_Utils.h"

// #define FIREBASE_BASE_URL "https://scan-9ee0b-default-rtdb.firebaseio.com"

static const char *TAG = "FIREBASE_UTILS";

// Global variable to store check-in status
static bool check_in_successful = false;

// Struct to store user information
typedef struct {
    bool active_student;
    bool check_in_status;
    char location[64];
    char role[32];
    char passkey[64];
} UserInfo;

// UserInfo *user_info = NULL;
// Static instance of UserInfo
UserInfo user_info_instance;
UserInfo* user_info = &user_info_instance;

// Struct for mapping Firebase fields to struct fields
typedef struct {
    const char *key;
    void *field;
    size_t field_size;
} UserFieldMapping;


// Function to retrieve user data and populate the struct
void check_in_user_task(void *pvParameters) {
    const char *user_id = (const char *)pvParameters;
    //UserInfo *user_info = (UserInfo *)pvParameters + sizeof(char *); // Pointer arithmetic to get the UserInfo struct

    if (user_id == NULL) {
        ESP_LOGE("Check-In", "Invalid input: user_id is NULL.");
        vTaskDelete(NULL); // Delete task if input is invalid
    }

    char url[256];
    char response_buffer[1024 + 1];
    int http_code;

    ESP_LOGI(TAG, "dre 1");
    snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s.json", user_id);
    http_code = firebase_https_request_get(url, response_buffer, sizeof(response_buffer));

    // Check if the HTTP request was successful
    if (http_code != 200) {
        ESP_LOGE("Check-In", "Failed to fetch user data. HTTP Code: %d", http_code);
        check_in_successful = false;  // Mark as failed if any field fails
        vTaskDelete(NULL); // Exit the task if the request fails
    }

    // Parse the JSON response
    cJSON *json = cJSON_Parse(response_buffer);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        check_in_successful = false;  // Mark as failed if JSON parsing fails
        vTaskDelete(NULL); // Exit the task if parsing fails
    }

    // Extract the fields from the JSON response
    cJSON *activeStudent_json = cJSON_GetObjectItem(json, "activeStudent");
    cJSON *checkInStatus_json = cJSON_GetObjectItem(json, "checkInStatus");
    // cJSON *lastCheckInId_json = cJSON_GetObjectItem(json, "lastCheckInId");
    // cJSON *lastCheckOutId_json= cJSON_GetObjectItem(json, "lastCheckOutId");
    cJSON *location_json = cJSON_GetObjectItem(json, "location");
    cJSON *passkey_json = cJSON_GetObjectItem(json, "passkey");
    cJSON *role_json = cJSON_GetObjectItem(json, "role");

    // Check if the fields exist in the JSON response
    if (activeStudent_json == NULL || checkInStatus_json == NULL || location_json == NULL || passkey_json == NULL || role_json == NULL) {
        ESP_LOGE(TAG, "Required fields not found in JSON response");
        cJSON_Delete(json); // Clean up JSON object
        check_in_successful = false;  // Mark as failed if any field is missing
        vTaskDelete(NULL); // Exit the task if any field is missing
    }

    // Assign the values to the struct fields
    user_info->active_student = cJSON_IsTrue(activeStudent_json);
    user_info->check_in_status = cJSON_IsTrue(checkInStatus_json);
    strncpy(user_info->location, cJSON_GetStringValue(location_json), sizeof(user_info->location) - 1);
    strncpy(user_info->role, cJSON_GetStringValue(role_json), sizeof(user_info->role) - 1);
    strncpy(user_info->passkey, cJSON_GetStringValue(passkey_json), sizeof(user_info->passkey) - 1);
    user_info->location[sizeof(user_info->location) - 1] = '\0'; // Ensure null termination
    user_info->role[sizeof(user_info->role) - 1] = '\0'; // Ensure null termination
    user_info->passkey[sizeof(user_info->passkey) - 1] = '\0'; // Ensure null termination

    ESP_LOGI(TAG, "user_info.active_student: %d", user_info->active_student);
    ESP_LOGI(TAG, "user_info.check_in_status: %d", user_info->check_in_status);
    ESP_LOGI(TAG, "user_info.location: %s", user_info->location);
    ESP_LOGI(TAG, "user_info.role: %s", user_info->role);
    ESP_LOGI(TAG, "user_info.passkey: %s", user_info->passkey);

    // Clean up JSON object
    cJSON_Delete(json);
    ESP_LOGI(TAG, "dre 2");

    // Bring this guy back but make him write to activity log

    if (user_info->active_student)
    {
        // Create a JSON object for the activity log entry
        cJSON *activity_log_json = cJSON_CreateObject();
        if (activity_log_json == NULL) {
            ESP_LOGE(TAG, "Failed to create JSON object for activity log");
            check_in_successful = false;
            vTaskDelete(NULL);
        }

        // Come back and format the time for indexing into the activity log correctly

        // Add fields to the JSON object
        cJSON_AddStringToObject(activity_log_json, "action", user_info->check_in_status ? "Check-Out" : "Check-In");
        cJSON_AddStringToObject(activity_log_json, "location", "UCF RWC");  // Replace with actual location
        cJSON_AddStringToObject(activity_log_json, "timestamp", "20250227_170200"); // Replace with actual timestamp
        cJSON_AddStringToObject(activity_log_json, "userId", user_id);

        // Convert the JSON object to a string
        char *activity_log_string = cJSON_Print(activity_log_json);
        if (activity_log_string == NULL) {
            ESP_LOGE(TAG, "Failed to print JSON object for activity log");
            cJSON_Delete(activity_log_json);
            check_in_successful = false;
            vTaskDelete(NULL);
        }

        char *activityTimestamp = "20250227170200"; // Replace with actual timestamp

        // Format the URL
        snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/activityLog/%s.json", activityTimestamp);
        ESP_LOGI(TAG, "ActivityLog URL: %s", url);
        ESP_LOGI(TAG, "Activity Log JSON: %s", activity_log_string);

        // Perform the PUT request
        http_code = firebase_https_request_put(url, activity_log_string, sizeof(activity_log_string));
        if (http_code != 200) {
            ESP_LOGE("Check-In", "Failed to update check-in status for user %s. HTTP Code: %d", user_id, http_code);
            check_in_successful = false;  // Mark as failed if any field fails
            vTaskDelete(NULL); // Exit the task if any field request fails
        }

    } else {
        ESP_LOGE(TAG, "User is not an active student. Check-in/check-out not allowed.");
        check_in_successful = false;  // Mark as failed if user is not active
        vTaskDelete(NULL); // Exit the task if user is not active
    }

    // If all fields are fetched successfully, mark the check-in as successful
    check_in_successful = true;
    ESP_LOGI(TAG, "User check-in/check-out successful for user %s.", user_id);

    // Delete task when done
    vTaskDelete(NULL);
}