#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_netif.h>
#include "kiosk_firebase.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "../main/include/global.h"

static const char *TAG = "kiosk_firebase";

// Firebase database base URL
#define FIREBASE_BASE_URL "https://scan-9ee0b-default-rtdb.firebaseio.com"

// HTTP client handle
static esp_http_client_handle_t http_client = NULL;
static bool network_initialized = false;

// Reference to the embedded Firebase certificate
extern const char firebase_server_cert_pem_start[] asm("_binary_firebase_cert_pem_start");
extern const char firebase_server_cert_pem_end[]   asm("_binary_firebase_cert_pem_end");

// Function to create JSON payload for kiosk power info
static char* create_kiosk_power_json(void)
{
    cJSON *root = NULL;
    char *json_str = NULL;
    char mac_str[18];  // MAC address string (XX:XX:XX:XX:XX:XX + null terminator)
    char formatted_value[16]; // Buffer for formatted floating-point values

    // Create JSON object
    root = cJSON_CreateObject();
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        return NULL;
    }

    // Format MAC address
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             device_info.mac_addr[0], device_info.mac_addr[1],
             device_info.mac_addr[2], device_info.mac_addr[3],
             device_info.mac_addr[4], device_info.mac_addr[5]);

    // Add device information to JSON with the specified key names
    cJSON_AddStringToObject(root, "name", device_info.kiosk_name);
    cJSON_AddStringToObject(root, "location", device_info.kiosk_location);
    cJSON_AddStringToObject(root, "esp32Type", "ESP32-C6");
    cJSON_AddStringToObject(root, "firmwareVersion", device_info.firmware_version);
    cJSON_AddStringToObject(root, "networkSSID", "Jonah Hotspot");
    
    // Power information with formatted values
    snprintf(formatted_value, sizeof(formatted_value), "%.2f", device_info.battery_voltage_volts);
    cJSON_AddStringToObject(root, "batteryVoltage", formatted_value);
    
    // Convert battery percentage to integer (whole number) before sending
    int battery_percentage_int = (int)device_info.battery_percentage;
    cJSON_AddNumberToObject(root, "batteryLevel", battery_percentage_int);
    cJSON_AddBoolToObject(root, "charging", device_info.is_charging);
    cJSON_AddBoolToObject(root, "active", true);
    
    // USB PD contract information with formatted values
    snprintf(formatted_value, sizeof(formatted_value), "%.2f", device_info.input_voltage_volts);
    cJSON_AddStringToObject(root, "USB_PD_contract_v", formatted_value);
    
    snprintf(formatted_value, sizeof(formatted_value), "%.2f", device_info.input_current_amps);
    cJSON_AddStringToObject(root, "USB_PD_contract_i", formatted_value);

    // Convert JSON to string
    json_str = cJSON_PrintUnformatted(root);
    if (json_str == NULL) {
        ESP_LOGE(TAG, "Failed to convert JSON to string");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_Delete(root);
    return json_str;
}

// HTTP event handler
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                ESP_LOGI(TAG, "HTTP response: %.*s", evt->data_len, (char *)evt->data);
            }
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP request error");
            break;
        default:
            break;
    }
    return ESP_OK;
}

bool kiosk_firebase_update_power_info(void)
{
    // Check if network is initialized
    if (!network_initialized) {
        ESP_LOGW(TAG, "Network not initialized, skipping Firebase update");
        return false;
    }

    bool success = false;
    char *json_str = NULL;
    char url[256];
    esp_err_t err;

    // Create JSON payload
    json_str = create_kiosk_power_json();
    if (json_str == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON payload");
        return false;
    }

    // Construct URL with MAC address from device_info
    snprintf(url, sizeof(url), "%s/kiosks/%s.json", FIREBASE_BASE_URL, device_info.mac_addr);

    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_PATCH,
        .event_handler = http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = 1024,
        .buffer_size_tx = 1024,
        .cert_pem = (char *)firebase_server_cert_pem_start,
    };

    // Create HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        free(json_str);
        return false;
    }

    // Set content type header
    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Set post data
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    // Perform HTTP request
    err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "Successfully updated kiosk power info in Firebase");
            success = true;
        } else {
            ESP_LOGE(TAG, "HTTP request failed with status code: %d", status_code);
        }
    }

    // Clean up
    esp_http_client_cleanup(client);
    free(json_str);

    return success;
}

bool kiosk_firebase_init(void)
{
    // Check if network stack is initialized
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGW(TAG, "Network interface not initialized yet, Firebase client will be initialized later");
        return true; // Return true to allow the system to continue, but mark as not ready
    }

    // Construct full URL with MAC address from device_info
    char url[256];
    snprintf(url, sizeof(url), "%s/kiosks/%s.json", FIREBASE_BASE_URL, device_info.mac_addr);

    // Initialize HTTP client configuration
    esp_http_client_config_t config = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = 1024,
        .buffer_size_tx = 1024,
        .cert_pem = (char *)firebase_server_cert_pem_start,
    };

    // Create HTTP client
    http_client = esp_http_client_init(&config);
    if (http_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return false;
    }

    network_initialized = true;
    ESP_LOGI(TAG, "Kiosk Firebase client initialized with URL: %s", url);
    return true;
}

// Function to be called when network is ready
void kiosk_firebase_network_ready(void)
{
    if (!network_initialized) {
        ESP_LOGI(TAG, "Network is now ready, initializing Firebase client");
        kiosk_firebase_init();
    }
} 