/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "firebase_utils.h"
#include "firebase_http_client.h"

#include "cJSON_Utils.h"

#define FIREBASE_BASE_URL "https://scan-9ee0b-default-rtdb.firebaseio.com/users/"
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 1024
static const char *TAG = "FIREBASE_HTTP_CLIENT";

extern const char firebase_server_cert_pem_start[] asm("_binary_firebase_cert_pem_start");
extern const char firebase_server_cert_pem_end[]   asm("_binary_firebase_cert_pem_end");


static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    int content_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (content_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
#if CONFIG_EXAMPLE_ENABLE_RESPONSE_BUFFER_DUMP
                ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
#endif
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}

int firebase_https_request_get(char *url, char *response, size_t response_size)
{
    int http_code = -1; // Default to error code
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};

    // Ensure url is not NULL and has reasonable length
    if (url == NULL || strlen(url) < 10 || strlen(url) > 512) {
        ESP_LOGE(TAG, "Invalid URL: %s", url ? url : "NULL");
        if (response && response_size > 0) {
            response[0] = '\0'; // Set response to an empty string on failure
        }
        return http_code;
    }

    // Check if URL starts with https://
    if (strncmp(url, "https://", 8) != 0) {
        ESP_LOGE(TAG, "URL must start with https://");
        if (response && response_size > 0) {
            response[0] = '\0';
        }
        return http_code;
    }

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
        .disable_auto_redirect = true,
        .cert_pem = (char *)firebase_server_cert_pem_start,
    };

    esp_http_client_handle_t client = NULL;
    client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        if (response && response_size > 0) {
            response[0] = '\0';
        }
        return http_code;
    }

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        http_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                http_code, esp_http_client_get_content_length(client));

        // Safely copy the response to the user-provided buffer
        if (response && response_size > 0) {
            strncpy(response, local_response_buffer, response_size - 1);
            response[response_size - 1] = '\0';

            // Parse and print the JSON response
            cJSON *json = cJSON_Parse(response);
            if (json == NULL) {
                ESP_LOGE(TAG, "Failed to parse JSON response");
            } else {
                char *json_string = cJSON_Print(json);
                if (json_string == NULL) {
                    ESP_LOGE(TAG, "Failed to print JSON response");
                } else {
                    ESP_LOGI(TAG, "JSON Response:\n%s", json_string);
                    free(json_string);
                }
                    cJSON_Delete(json);
                }
                }
            } else {
                ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
                if (response && response_size > 0) {
                    response[0] = '\0';
                }
            }

    http_code = esp_http_client_get_status_code(client);

    if (client != NULL) {
    esp_http_client_cleanup(client);
    }

    return http_code;
}

int firebase_https_request_put(char *url, char *data, size_t data_size)
{
    int http_code = -1;
    
    // Ensure url is not NULL and has reasonable length
    if (url == NULL || strlen(url) < 10 || strlen(url) > 512) {
        ESP_LOGE(TAG, "Invalid URL: %s", url ? url : "NULL");
        return http_code;
    }
    
    // Check if URL starts with https://
    if (strncmp(url, "https://", 8) != 0) {
        ESP_LOGE(TAG, "URL must start with https://");
        return http_code;
    }
    
    if (data == NULL) {
        ESP_LOGE(TAG, "PUT data is NULL");
        return http_code;
    }
    
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
    size_t local_response_buffer_size = sizeof(local_response_buffer);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
        .cert_pem = (char *)firebase_server_cert_pem_start,
        .method = HTTP_METHOD_PUT
    };
    
    esp_http_client_handle_t client = NULL;
    
    client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return http_code;
    }
    
    size_t actual_data_len = strlen(data);
    esp_http_client_set_post_field(client, data, actual_data_len);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        http_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP PUT Status = %d, content_length = %"PRId64,
                http_code, esp_http_client_get_content_length(client));
        ESP_LOGI(TAG, "Response: %s", local_response_buffer);
    } else {
        ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

    if (client != NULL) {
        esp_http_client_cleanup(client);
    }
    
    return http_code;
}