#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID "iPhone (43)"
#define EXAMPLE_ESP_WIFI_PASS "brynds8899"
#define EXAMPLE_ESP_MAXIMUM_RETRY 10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

void wifi_init_sta(void);
void wifi_init_task(void *pvParameter);

extern SemaphoreHandle_t wifi_init_semaphore;
/* FreeRTOS event group to signal when we are connected*/