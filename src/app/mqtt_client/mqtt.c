#include "mqtt.h"
#include "../ota_update/include/ota.h"
#include "../main/include/global.h"

static const char *TAG = "MQTT";

extern const uint8_t hivemq_server_cert_pem_start[] asm("_binary_hivemq_cert_pem_start");
extern const uint8_t hivemq_server_cert_pem_end[] asm("_binary_hivemq_cert_pem_end");

// Declare external OTA task handle
extern TaskHandle_t ota_update_task_handle;

// MQTT credentials using device MAC address
const char *mqtt_username;

const char *mqtt_password = "Pass1234";

// Static MQTT client handle for ping task
static esp_mqtt_client_handle_t mqtt_client = NULL;


static void handle_update_topic(esp_mqtt_event_handle_t event, esp_mqtt_client_handle_t client)
{
    ESP_LOGI(TAG, "Handling update topic message");
    
    // Create a null-terminated copy of the data for easier parsing
    char *data = malloc(event->data_len + 1);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for message data");
        return;
    }
    memcpy(data, event->data, event->data_len);
    data[event->data_len] = '\0';

    // Find the download_url field
    char *url_start = strstr(data, "download_url:");
    if (url_start != NULL) {
        url_start += strlen("download_url:"); // Move past the field name
        char *url_end = strchr(url_start, '}');
        if (url_end != NULL) {
            // Calculate URL length and create a string for it
            size_t url_len = url_end - url_start;
            char *url = malloc(url_len + 1);
            if (url != NULL) {
                memcpy(url, url_start, url_len);
                url[url_len] = '\0';
                ESP_LOGI(TAG, "Received update URL: %s", url);
                
                // Create OTA task with the received URL
                if (ota_update_task_handle == NULL) {
                    ESP_LOGI(TAG, "Creating OTA update task");
                    ESP_LOGI(TAG, "Free heap before OTA task: %lu bytes", esp_get_free_heap_size());
                    ESP_LOGI(TAG, "Minimum free heap: %lu bytes", esp_get_minimum_free_heap_size());
                    esp_err_t err = xTaskCreate(ota_update_fw_task, "OTA UPDATE TASK", 1024 * 4, url, 8, &ota_update_task_handle);
                    if (err != pdPASS) {
                        return;
                    }
                    ESP_LOGI(TAG, "Free heap after OTA task: %lu bytes", esp_get_free_heap_size());
                } else {
                    ESP_LOGW(TAG, "OTA task already running");
                }
                
                free(url);
            }
        }
    }

    free(data);
}

static void handle_mqtt_event_data(esp_mqtt_event_handle_t event, esp_mqtt_client_handle_t client)
{
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    // Check topic and route to appropriate handler
    char topic[64];
    snprintf(topic, sizeof(topic), "kiosks/%s/update", device_info.mac_addr);
    if (strncmp(event->topic, topic, event->topic_len) == 0) {
        handle_update_topic(event, client);
    }
    // Add more topic handlers here as needed
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        char topic[64];
        snprintf(topic, sizeof(topic), "kiosks/%s/update", device_info.mac_addr);
        msg_id = esp_mqtt_client_subscribe(client, topic, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        handle_mqtt_event_data(event, client);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_ping_task(void *pvParameters)
{
    while (1) {
        if (mqtt_client != NULL) {
            char topic[64];
            snprintf(topic, sizeof(topic), "kiosks/%s/ping", device_info.mac_addr);
            int msg_id = esp_mqtt_client_publish(mqtt_client, topic, "ping", 0, 0, 1);
            if (msg_id < 0) {
                ESP_LOGE(TAG, "Failed to send ping message");
            } else {
                ESP_LOGI(TAG, "Sent ping message");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // Send ping every 10 seconds
    }
}

void mqtt_init(void)
{
    // Set MQTT username to device MAC address
    mqtt_username = device_info.mac_addr;
    
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = CONFIG_BROKER_URI,
            .verification.certificate = (const char *)hivemq_server_cert_pem_start
        },

        .credentials = {
            .username = mqtt_username,

            .authentication = {
                .password = mqtt_password,
            },
        },

    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// Function to start the ping task
void mqtt_start_ping_task(void)
{
    if (mqtt_client != NULL) {
        xTaskCreate(mqtt_ping_task, "mqtt_ping_task", 2048, NULL, 10, NULL);
        ESP_LOGI(TAG, "MQTT ping task started");
    } else {
        ESP_LOGE(TAG, "Cannot start ping task: MQTT client not initialized");
    }
}

// Function to publish a status message to the device-specific topic
void mqtt_publish_status(const char *status)
{
    if (mqtt_client != NULL) {
        char topic[64];
        snprintf(topic, sizeof(topic), "kiosks/%s/status", device_info.mac_addr);
        int msg_id = esp_mqtt_client_publish(mqtt_client, topic, status, 0, 0, 0);
        if (msg_id < 0) {
            ESP_LOGE(TAG, "Failed to publish status message");
        } else {
            ESP_LOGI(TAG, "Published status message with msg_id=%d", msg_id);
        }
    } else {
        ESP_LOGE(TAG, "Cannot publish status: MQTT client not initialized");
    }
}
