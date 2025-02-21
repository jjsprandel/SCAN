
#include "firebase_utils.h"

bool check_in_successful = false;
UserInfo *user_info = NULL;
static const char *FIREBASE_UTILS_TAG = "FIREBASE_UTILS";

// Global variable to store check-in status
// bool check_in_successful = false;

// Function to retrieve user data and populate the struct
void check_in_user_task(void *pvParameters)
{
    const char *user_id = (const char *)pvParameters;
    // UserInfo *user_info = (UserInfo *)pvParameters + sizeof(char *); // Pointer arithmetic to get the UserInfo struct

    if (user_id == NULL)
    {
        ESP_LOGE(FIREBASE_UTILS_TAG, "Invalid input: user_id is NULL.");
        vTaskDelete(NULL); // Delete task if input is invalid
    }

    // Allocate memory for user_info only once (if not already allocated)
    if (user_info == NULL)
    {
        user_info = (UserInfo *)malloc(sizeof(UserInfo)); // Allocate memory for user_info
        if (user_info == NULL)
        {
            ESP_LOGE(FIREBASE_UTILS_TAG, "Failed to allocate memory for user_info.");
            vTaskDelete(NULL); // Exit task if allocation fails
        }
    }

    // Field mappings (Firebase key, struct field, and field size)
    UserFieldMapping field_mappings[] = {
        {"activeStudent", &user_info->active_student, sizeof(user_info->active_student)},
        {"checkInStatus", &user_info->check_in_status, sizeof(user_info->check_in_status)},
        //{"location", user_info->location, sizeof(user_info->location)},
        //{"role", user_info->role, sizeof(user_info->role)},
        //{"passkey", user_info->passkey, sizeof(user_info->passkey)}
    };

    char url[256];
    char response_buffer[1024 + 1];
    int http_code;

    // Iterate through the fields and map them to struct fields
    for (size_t i = 0; i < sizeof(field_mappings) / sizeof(field_mappings[0]); i++)
    {
        // Format the URL
        snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s/%s.json", user_id, field_mappings[i].key);

        // Perform the GET request
        http_code = firebase_https_request_get(url, response_buffer, sizeof(response_buffer));
        if (http_code != 200)
        {
            ESP_LOGE("Check-In", "Failed to fetch '%s' for user %s. HTTP Code: %d", field_mappings[i].key, user_id, http_code);
            check_in_successful = false; // Mark as failed if any field fails
            vTaskDelete(NULL);           // Exit the task if any field request fails
        }

        // Directly assign the response to the appropriate struct field
        if (field_mappings[i].field_size == sizeof(bool))
        {
            // For boolean fields (activeStudent, checkInStatus)
            ESP_LOGI(FIREBASE_UTILS_TAG, "stored in response buffer: %s", response_buffer);
            *(bool *)field_mappings[i].field = (strcmp(response_buffer, "true") == 0);
            ESP_LOGI(FIREBASE_UTILS_TAG, "user_info.active_student: %d", user_info->active_student);
            ESP_LOGI(FIREBASE_UTILS_TAG, "user_info.check_in_status: %d", user_info->check_in_status);
        }
        else
        {
            // For string fields (location, role, passkey)
            strncpy((char *)field_mappings[i].field, response_buffer, field_mappings[i].field_size - 1);
        }
    }

    if (user_info->active_student)
    {
        user_info->check_in_status = !user_info->check_in_status;

        // Convert the bool to a string
        if (user_info->check_in_status)
        {
            strcpy(response_buffer, "\"true\"");
        }
        else
        {
            strcpy(response_buffer, "\"false\"");
        }

        // Format the URL
        snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s/checkInStatus.json", user_id);
        ESP_LOGI(FIREBASE_UTILS_TAG, "%s", url);
        ESP_LOGI(FIREBASE_UTILS_TAG, "%s", response_buffer);
        // Perform the PUT request
        http_code = firebase_https_request_put(url, response_buffer, sizeof(response_buffer));
        if (http_code != 200)
        {
            ESP_LOGE("Check-In", "Failed to update check-in status for user %s. HTTP Code: %d", user_id, http_code);
            check_in_successful = false; // Mark as failed if any field fails
            vTaskDelete(NULL);           // Exit the task if any field request fails
        }
    }

    // If all fields are fetched successfully, mark the check-in as successful
    check_in_successful = true;
    ESP_LOGI(FIREBASE_UTILS_TAG, "User check-in/check-out successful for user %s.", user_id);
    xEventGroupSetBits(event_group, ID_AUTHENTICATED_BIT);

    // Delete task when done
    vTaskDelete(NULL);
}

// // Function to retrieve user data and populate the struct
// bool check_in_user(char *user_id)
// {
//     // const char *user_id = (const char *)pvParameters;
//     // UserInfo *user_info = (UserInfo *)pvParameters + sizeof(char *); // Pointer arithmetic to get the UserInfo struct

//     if (user_id == NULL)
//     {
//         ESP_LOGE("Check-In", "Invalid input: user_id is NULL.");
//         return false;
//     }

//     // Allocate memory for user_info only once (if not already allocated)
//     if (user_info == NULL)
//     {
//         user_info = (UserInfo *)malloc(sizeof(UserInfo)); // Allocate memory for user_info
//         if (user_info == NULL)
//         {
//             ESP_LOGE(FIREBASE_UTILS_TAG, "Failed to allocate memory for user_info.");
//             vTaskDelete(NULL); // Exit task if allocation fails
//         }
//     }

//     // Field mappings (Firebase key, struct field, and field size)
//     UserFieldMapping field_mappings[] = {
//         {"activeStudent", &user_info->active_student, sizeof(user_info->active_student)},
//         {"checkInStatus", &user_info->check_in_status, sizeof(user_info->check_in_status)},
//         //{"location", user_info->location, sizeof(user_info->location)},
//         //{"role", user_info->role, sizeof(user_info->role)},
//         //{"passkey", user_info->passkey, sizeof(user_info->passkey)}
//     };

//     char url[256];
//     char response_buffer[1024 + 1];
//     int http_code;

//     // Iterate through the fields and map them to struct fields
//     for (size_t i = 0; i < sizeof(field_mappings) / sizeof(field_mappings[0]); i++)
//     {
//         // Format the URL
//         snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s/%s.json", user_id, field_mappings[i].key);

//         // Perform the GET request
//         http_code = firebase_https_request_get(url, response_buffer, sizeof(response_buffer));
//         if (http_code != 200)
//         {
//             ESP_LOGE("Check-In", "Failed to fetch '%s' for user %s. HTTP Code: %d", field_mappings[i].key, user_id, http_code);
//             check_in_successful = false; // Mark as failed if any field fails
//             return check_in_successful;
//         }

//         // Directly assign the response to the appropriate struct field
//         if (field_mappings[i].field_size == sizeof(bool))
//         {
//             // For boolean fields (activeStudent, checkInStatus)
//             ESP_LOGI(FIREBASE_UTILS_TAG, "stored in response buffer: %s", response_buffer);
//             *(bool *)field_mappings[i].field = (strcmp(response_buffer, "true") == 0);
//             ESP_LOGI(FIREBASE_UTILS_TAG, "user_info.active_student: %d", user_info->active_student);
//             ESP_LOGI(FIREBASE_UTILS_TAG, "user_info.check_in_status: %d", user_info->check_in_status);
//         }
//         else
//         {
//             // For string fields (location, role, passkey)
//             strncpy((char *)field_mappings[i].field, response_buffer, field_mappings[i].field_size - 1);
//         }
//     }
//     ESP_LOGI(FIREBASE_UTILS_TAG, "HERE 4");

//     if (user_info->active_student)
//     {
//         user_info->check_in_status = !user_info->check_in_status;

//         // Convert the bool to a string
//         if (user_info->check_in_status)
//         {
//             strcpy(response_buffer, "\"true\"");
//         }
//         else
//         {
//             strcpy(response_buffer, "\"false\"");
//         }

//         // Format the URL
//         snprintf(url, sizeof(url), "https://scan-9ee0b-default-rtdb.firebaseio.com/users/%s/checkInStatus.json", user_id);
//         ESP_LOGI(FIREBASE_UTILS_TAG, "%s", url);
//         ESP_LOGI(FIREBASE_UTILS_TAG, "%s", response_buffer);
//         // Perform the PUT request
//         http_code = firebase_https_request_put(url, response_buffer, sizeof(response_buffer));
//         if (http_code != 200)
//         {
//             ESP_LOGE("Check-In", "Failed to update check-in status for user %s. HTTP Code: %d", user_id, http_code);
//             check_in_successful = false; // Mark as failed if any field fails
//             return check_in_successful;
//         }
//     }

//     // If all fields are fetched successfully, mark the check-in as successful
//     check_in_successful = true;
//     ESP_LOGI(FIREBASE_UTILS_TAG, "User check-in/check-out successful for user %s.", user_id);

//     // Delete task when done
//     return check_in_successful;
// }