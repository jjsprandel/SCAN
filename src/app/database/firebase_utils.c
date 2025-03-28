#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "firebase_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "firebase_http_client.h"
#include "firebase_utils.h"

#include "cJSON_Utils.h"

#include "esp_sntp.h"
#include <time.h>
#include <ctype.h>
#define KIOSK_LOCATION "UCF RWC"
#define ID_LEN 10

// #define FIREBASE_BASE_URL "https://scan-9ee0b-default-rtdb.firebaseio.com"

static const char *TAG = "FIREBASE_UTILS";

// Global variables for timestamps
static char activityLogIndex[20];
static char timestamp[21];

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

// Function to initialize SNTP
void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

// Function to obtain time from SNTP server
void obtain_time(void) {
    // Initialize SNTP once
    static bool sntp_initialized = false;
    if (!sntp_initialized) {
        initialize_sntp();
        sntp_initialized = true;
    }

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == retry_count) {
        ESP_LOGE(TAG, "Failed to synchronize time with NTP server");
    } else {
        ESP_LOGI(TAG, "Time synchronized with NTP server");
    }
}

// Function to get the current timestamp
void get_current_timestamp(void) {
    time_t now;
    struct tm timeinfo;

    // Set timezone to US Eastern
    setenv("TZ", "EST5EDT", 1);
    tzset();

    time(&now);
    localtime_r(&now, &timeinfo);

    // Format activityLogIndex as YYYYMMDDHHMMSS
    strftime(activityLogIndex, sizeof(activityLogIndex), "%Y%m%d%H%M%S", &timeinfo);

    // Format timestamp as YYYYMMDD_HHMMSS
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &timeinfo);
}

// Function to retrieve user data and populate the struct
bool get_user_info(const char* user_id)
{
    if (user_id == NULL) {
        ESP_LOGE("Get User Info", "Invalid input: user_id is NULL.");
        return false; // Delete task if input is invalid
    }

    // Check for valid user_id (ensure it's not binary garbage)
    for (size_t i = 0; i < ID_LEN && user_id[i] != '\0'; i++) {
        if (!isprint((unsigned char)user_id[i])) {
            ESP_LOGE("Get User Info", "Invalid user_id contains non-printable characters");
            return false;
        }
        // Check if all characters are digits
        if (!isdigit((unsigned char)user_id[i])) {
            ESP_LOGE("Get User Info", "Invalid user_id contains non-numeric characters");
            return false;
        }
    }

    // Create a sanitized copy with null termination
    char sanitized_id[ID_LEN + 1] = {0};
    strlcpy(sanitized_id, user_id, sizeof(sanitized_id));

    // Obtain time from NTP server
    obtain_time();

    char url[256];
    char response_buffer[1024 + 1];
    int http_code;

    // Use sanitized, guaranteed null-terminated ID
    if (snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s.json", sanitized_id) >= sizeof(url)) {
        ESP_LOGE("Get User Info", "URL too long, buffer overflow prevented");
        return false;
    }

    ESP_LOGI(TAG, "Retrieving user info for ID: %s", sanitized_id);
    ESP_LOGI(TAG, "URL: %s", url);
    
    // Add error handling around HTTP request
    esp_err_t err = ESP_OK;
    http_code = firebase_https_request_get(url, response_buffer, sizeof(response_buffer));

    // Check if the HTTP request was successful
    if (http_code != 200) {
        ESP_LOGE("Get User Info", "Failed to fetch user data. HTTP Code: %d", http_code);
        return false; // Exit the task if the request fails
    }

    // Parse the JSON response
    cJSON *json = cJSON_Parse(response_buffer);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        return false; // Exit the task if parsing fails
    }

    // Extract the fields from the JSON response
    cJSON *activeStudent_json = cJSON_GetObjectItem(json, "activeStudent");
    cJSON *checkInStatus_json = cJSON_GetObjectItem(json, "checkInStatus");
    cJSON *firstName_json = cJSON_GetObjectItem(json, "firstName");
    cJSON *lastName_json = cJSON_GetObjectItem(json, "lastName");
    // cJSON *lastCheckInId_json = cJSON_GetObjectItem(json, "lastCheckInId");
    // cJSON *lastCheckOutId_json= cJSON_GetObjectItem(json, "lastCheckOutId");
    // cJSON *location_json = cJSON_GetObjectItem(json, "location");
    cJSON *passkey_json = cJSON_GetObjectItem(json, "passkey");
    cJSON *role_json = cJSON_GetObjectItem(json, "role");

    // Check if the fields exist in the JSON response
    if (activeStudent_json == NULL || checkInStatus_json == NULL || firstName_json == NULL || lastName_json == NULL || passkey_json == NULL || role_json == NULL) {
        ESP_LOGE(TAG, "Required fields not found in JSON response");
        cJSON_Delete(json); // Clean up JSON object
        return false; // Exit the task if any field is missing
    }

    // Assign the values to the struct fields
    strncpy(user_info->active_student, cJSON_GetStringValue(activeStudent_json), sizeof(user_info->active_student) - 1);
    strncpy(user_info->check_in_status, cJSON_GetStringValue(checkInStatus_json), sizeof(user_info->check_in_status) - 1);
    strncpy(user_info->first_name, cJSON_GetStringValue(firstName_json), sizeof(user_info->first_name) - 1);
    strncpy(user_info->last_name, cJSON_GetStringValue(lastName_json), sizeof(user_info->last_name) - 1);
    // strncpy(user_info->location, cJSON_GetStringValue(location_json), sizeof(user_info->location) - 1);
    strncpy(user_info->role, cJSON_GetStringValue(role_json), sizeof(user_info->role) - 1);
    strncpy(user_info->passkey, cJSON_GetStringValue(passkey_json), sizeof(user_info->passkey) - 1);

    // Ensure NULL termination
    // user_info->location[sizeof(user_info->location) - 1] = '\0'; // Ensure null termination
    user_info->active_student[sizeof(user_info->active_student) - 1] = '\0'; // Ensure null termination
    user_info->check_in_status[sizeof(user_info->check_in_status) - 1] = '\0'; // Ensure null termination
    user_info->first_name[sizeof(user_info->first_name) - 1] = '\0'; // Ensure null termination
    user_info->last_name[sizeof(user_info->last_name) - 1] = '\0'; // Ensure null termination
    user_info->role[sizeof(user_info->role) - 1] = '\0'; // Ensure null termination
    user_info->passkey[sizeof(user_info->passkey) - 1] = '\0'; // Ensure null termination

    // Print user information
    ESP_LOGI(TAG, "user_info.active_student: %s", user_info->active_student);
    ESP_LOGI(TAG, "user_info.check_in_status: %s", user_info->check_in_status);
    ESP_LOGI(TAG, "user_info.first_name: %s", user_info->first_name);
    ESP_LOGI(TAG, "user_info.last_name: %s", user_info->last_name);
    ESP_LOGI(TAG, "user_info.location: %s", user_info->location);
    ESP_LOGI(TAG, "user_info.role: %s", user_info->role);
    ESP_LOGI(TAG, "user_info.passkey: %s", user_info->passkey);

    return true;
}

// Function to retrieve user data and populate the struct
bool check_in_user(const char* user_id)
{
    if (user_id == NULL) {
        ESP_LOGE("Check-In", "Invalid input: user_id is NULL.");
        return false; // Delete task if input is invalid
    }

    // Check for valid user_id (ensure it's not binary garbage)
    for (size_t i = 0; i < ID_LEN && user_id[i] != '\0'; i++) {
        if (!isprint((unsigned char)user_id[i])) {
            ESP_LOGE("Check-In", "Invalid user_id contains non-printable characters");
            return false;
        }
        // Check if all characters are digits
        if (!isdigit((unsigned char)user_id[i])) {
            ESP_LOGE("Check-In", "Invalid user_id contains non-numeric characters");
            return false;
        }
    }

    // Create a sanitized copy with null termination
    char sanitized_id[ID_LEN + 1] = {0};
    strlcpy(sanitized_id, user_id, sizeof(sanitized_id));

    // Obtain time from NTP server
    obtain_time();

    char url[256];
    char response_buffer[1024 + 1];
    int http_code;

    if (strcmp(user_info->active_student, "Yes") == 0)
    {
        // Create a JSON object for the activity log entry
        cJSON *activity_log_json = cJSON_CreateObject();
        if (activity_log_json == NULL) {
            ESP_LOGE(TAG, "Failed to create JSON object for activity log");
            return false;
        }

        get_current_timestamp();
        ESP_LOGI(TAG, "Timestamp: %s", timestamp);
        ESP_LOGI(TAG, "ActivityLogIndex: %s", activityLogIndex);

        // Add fields to the JSON object
        cJSON_AddStringToObject(activity_log_json, "action", "Check-In");
        cJSON_AddStringToObject(activity_log_json, "location", KIOSK_LOCATION);  // Replace with actual location
        cJSON_AddStringToObject(activity_log_json, "timestamp", timestamp); // Replace with actual timestamp
        cJSON_AddStringToObject(activity_log_json, "userId", sanitized_id); // Use sanitized id

        // Convert the JSON object to a string
        char *activity_log_string = cJSON_Print(activity_log_json);
        if (activity_log_string == NULL) {
            ESP_LOGE(TAG, "Failed to print JSON object for activity log");
            cJSON_Delete(activity_log_json);
            return false;
        }

        // Format the URL with overflow check
        if (snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/activityLog/%s.json", activityLogIndex) >= sizeof(url)) {
            ESP_LOGE(TAG, "ActivityLog URL too long, buffer overflow prevented");
            cJSON_Delete(activity_log_json);
            free(activity_log_string);
            return false;
        }
        
        ESP_LOGI(TAG, "ActivityLog URL: %s", url);
        ESP_LOGI(TAG, "Activity Log JSON: %s", activity_log_string);

        // Perform the PUT request with error handling
        http_code = firebase_https_request_put(url, activity_log_string, strlen(activity_log_string));
        free(activity_log_string); // Free the string once used
        cJSON_Delete(activity_log_json); // Clean up JSON object
        
        if (http_code != 200) {
            ESP_LOGE("Check-In", "Failed to check-in user %s. HTTP Code: %d", sanitized_id, http_code);
            return false; // Exit the task if any field request fails
        }

    } else {
        ESP_LOGE(TAG, "User is not an active student. Check-in not allowed.");
        return false; // Exit the task if user is not active
    }

    ESP_LOGI(TAG, "User check-in successful for user %s.", sanitized_id);

    // Delete task when done
    return true;
}

// Function to retrieve user data and populate the struct
bool check_out_user(const char* user_id)
{
    if (user_id == NULL) {
        ESP_LOGE("Check-Out", "Invalid input: user_id is NULL.");
        return false; // Delete task if input is invalid
    }

    // Check for valid user_id (ensure it's not binary garbage)
    for (size_t i = 0; i < ID_LEN && user_id[i] != '\0'; i++) {
        if (!isprint((unsigned char)user_id[i])) {
            ESP_LOGE("Check-Out", "Invalid user_id contains non-printable characters");
            return false;
        }
        // Check if all characters are digits
        if (!isdigit((unsigned char)user_id[i])) {
            ESP_LOGE("Check-Out", "Invalid user_id contains non-numeric characters");
            return false;
        }
    }

    // Create a sanitized copy with null termination
    char sanitized_id[ID_LEN + 1] = {0};
    strlcpy(sanitized_id, user_id, sizeof(sanitized_id));

    // Obtain time from NTP server
    obtain_time();

    char url[256];
    char response_buffer[1024 + 1];
    int http_code;

    if (strcmp(user_info->active_student, "Yes") == 0)
    {
        // Create a JSON object for the activity log entry
        cJSON *activity_log_json = cJSON_CreateObject();
        if (activity_log_json == NULL) {
            ESP_LOGE(TAG, "Failed to create JSON object for activity log");
            return false;
        }

        get_current_timestamp();
        ESP_LOGI(TAG, "Timestamp: %s", timestamp);
        ESP_LOGI(TAG, "ActivityLogIndex: %s", activityLogIndex);

        // Add fields to the JSON object
        cJSON_AddStringToObject(activity_log_json, "action", "Check-Out");
        cJSON_AddStringToObject(activity_log_json, "location", KIOSK_LOCATION);  // Replace with actual location
        cJSON_AddStringToObject(activity_log_json, "timestamp", timestamp); // Replace with actual timestamp
        cJSON_AddStringToObject(activity_log_json, "userId", sanitized_id); // Use sanitized id

        // Convert the JSON object to a string
        char *activity_log_string = cJSON_Print(activity_log_json);
        if (activity_log_string == NULL) {
            ESP_LOGE(TAG, "Failed to print JSON object for activity log");
            cJSON_Delete(activity_log_json);
            return false;
        }

        // Format the URL with overflow check
        if (snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/activityLog/%s.json", activityLogIndex) >= sizeof(url)) {
            ESP_LOGE(TAG, "ActivityLog URL too long, buffer overflow prevented");
            cJSON_Delete(activity_log_json);
            free(activity_log_string);
            return false;
        }
        
        ESP_LOGI(TAG, "ActivityLog URL: %s", url);
        ESP_LOGI(TAG, "Activity Log JSON: %s", activity_log_string);

        // Perform the PUT request with error handling
        http_code = firebase_https_request_put(url, activity_log_string, strlen(activity_log_string));
        free(activity_log_string); // Free the string once used
        cJSON_Delete(activity_log_json); // Clean up JSON object
        
        if (http_code != 200) {
            ESP_LOGE("Check-Out", "Failed to check-out user %s. HTTP Code: %d", sanitized_id, http_code);
            return false; // Exit the task if any field request fails
        }

    } else {
        ESP_LOGE(TAG, "User is not an active student. Check-out not allowed.");
        return false; // Exit the task if user is not active
    }

    ESP_LOGI(TAG, "User check-out successful for user %s.", sanitized_id);

    // Delete task when done
    return true;
}

// // Function to retrieve user data and populate the struct
// void check_in_user_task(void *pvParameters) {
//     const char *user_id = (const char *)pvParameters;
//     //UserInfo *user_info = (UserInfo *)pvParameters + sizeof(char *); // Pointer arithmetic to get the UserInfo struct

//     if (user_id == NULL) {
//         ESP_LOGE("Check-In", "Invalid input: user_id is NULL.");
//         vTaskDelete(NULL); // Delete task if input is invalid
//     }

//     // Obtain time from NTP server
//     obtain_time();

//     char url[256];
//     char response_buffer[1024 + 1];
//     int http_code;

//     ESP_LOGI(TAG, "dre 1");
//     snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s.json", user_id);
//     http_code = firebase_https_request_get(url, response_buffer, sizeof(response_buffer));

//     // Check if the HTTP request was successful
//     if (http_code != 200) {
//         ESP_LOGE("Check-In", "Failed to fetch user data. HTTP Code: %d", http_code);
//         check_in_successful = false;  // Mark as failed if any field fails
//         vTaskDelete(NULL); // Exit the task if the request fails
//     }

//     // Parse the JSON response
//     cJSON *json = cJSON_Parse(response_buffer);
//     if (json == NULL) {
//         ESP_LOGE(TAG, "Failed to parse JSON response");
//         check_in_successful = false;  // Mark as failed if JSON parsing fails
//         vTaskDelete(NULL); // Exit the task if parsing fails
//     }

//     // Extract the fields from the JSON response
//     cJSON *activeStudent_json = cJSON_GetObjectItem(json, "activeStudent");
//     cJSON *checkInStatus_json = cJSON_GetObjectItem(json, "checkInStatus");
//     // cJSON *lastCheckInId_json = cJSON_GetObjectItem(json, "lastCheckInId");
//     // cJSON *lastCheckOutId_json= cJSON_GetObjectItem(json, "lastCheckOutId");
//     cJSON *location_json = cJSON_GetObjectItem(json, "location");
//     cJSON *passkey_json = cJSON_GetObjectItem(json, "passkey");
//     cJSON *role_json = cJSON_GetObjectItem(json, "role");

//     // Check if the fields exist in the JSON response
//     if (activeStudent_json == NULL || checkInStatus_json == NULL || location_json == NULL || passkey_json == NULL || role_json == NULL) {
//         ESP_LOGE(TAG, "Required fields not found in JSON response");
//         cJSON_Delete(json); // Clean up JSON object
//         check_in_successful = false;  // Mark as failed if any field is missing
//         vTaskDelete(NULL); // Exit the task if any field is missing
//     }

//     // Assign the values to the struct fields
//     strncpy(user_info->active_student, cJSON_GetStringValue(activeStudent_json), sizeof(user_info->active_student) - 1);
//     strncpy(user_info->check_in_status, cJSON_GetStringValue(checkInStatus_json), sizeof(user_info->check_in_status) - 1);
//     strncpy(user_info->location, cJSON_GetStringValue(location_json), sizeof(user_info->location) - 1);
//     strncpy(user_info->role, cJSON_GetStringValue(role_json), sizeof(user_info->role) - 1);
//     strncpy(user_info->passkey, cJSON_GetStringValue(passkey_json), sizeof(user_info->passkey) - 1);
//     user_info->location[sizeof(user_info->location) - 1] = '\0'; // Ensure null termination
//     user_info->role[sizeof(user_info->role) - 1] = '\0'; // Ensure null termination
//     user_info->passkey[sizeof(user_info->passkey) - 1] = '\0'; // Ensure null termination

//     ESP_LOGI(TAG, "user_info.active_student: %s", user_info->active_student);
//     ESP_LOGI(TAG, "user_info.check_in_status: %s", user_info->check_in_status);
//     ESP_LOGI(TAG, "user_info.location: %s", user_info->location);
//     ESP_LOGI(TAG, "user_info.role: %s", user_info->role);
//     ESP_LOGI(TAG, "user_info.passkey: %s", user_info->passkey);

//     // Clean up JSON object
//     cJSON_Delete(json);
//     ESP_LOGI(TAG, "dre 2");

//     if (strcmp(user_info->active_student, "Yes") == 0)
//     {
//         // Create a JSON object for the activity log entry
//         cJSON *activity_log_json = cJSON_CreateObject();
//         if (activity_log_json == NULL) {
//             ESP_LOGE(TAG, "Failed to create JSON object for activity log");
//             check_in_successful = false;
//             vTaskDelete(NULL);
//         }

//         // Come back and format the time for indexing into the activity log correctly
//         get_current_timestamp();
//         ESP_LOGI(TAG, "Timestamp: %s", timestamp);
//         ESP_LOGI(TAG, "ActivityLogIndex: %s", activityLogIndex);

//         // Add fields to the JSON object
//         cJSON_AddStringToObject(activity_log_json, "action", (strcmp(user_info->check_in_status, "Checked In") == 0) ? "Check-Out" : "Check-In");
//         cJSON_AddStringToObject(activity_log_json, "location", KIOSK_LOCATION);  // Replace with actual location
//         cJSON_AddStringToObject(activity_log_json, "timestamp", timestamp); // Replace with actual timestamp
//         cJSON_AddStringToObject(activity_log_json, "userId", user_id);

//         // Convert the JSON object to a string
//         char *activity_log_string = cJSON_Print(activity_log_json);
//         if (activity_log_string == NULL) {
//             ESP_LOGE(TAG, "Failed to print JSON object for activity log");
//             cJSON_Delete(activity_log_json);
//             check_in_successful = false;
//             vTaskDelete(NULL);
//         }

//         // Format the URL
//         snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/activityLog/%s.json", activityLogIndex);
//         ESP_LOGI(TAG, "ActivityLog URL: %s", url);
//         ESP_LOGI(TAG, "Activity Log JSON: %s", activity_log_string);

//         // Perform the PUT request
//         http_code = firebase_https_request_put(url, activity_log_string, sizeof(activity_log_string));
//         if (http_code != 200) {
//             ESP_LOGE("Check-In", "Failed to update check-in status for user %s. HTTP Code: %d", user_id, http_code);
//             check_in_successful = false;  // Mark as failed if any field fails
//             vTaskDelete(NULL); // Exit the task if any field request fails
//         }

//     } else {
//         ESP_LOGE(TAG, "User is not an active student. Check-in/check-out not allowed.");
//         check_in_successful = false;  // Mark as failed if user is not active
//         vTaskDelete(NULL); // Exit the task if user is not active
//     }

//     // If all fields are fetched successfully, mark the check-in as successful
//     check_in_successful = true;
//     ESP_LOGI(TAG, "User check-in/check-out successful for user %s.", user_id);

//     // Delete task when done
//     vTaskDelete(NULL);
// }