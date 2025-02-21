#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#if CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
#include "esp_efuse.h"
#endif

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

// #define CONFIG_EXAMPLE_SKIP_VERSION_CHECK
#define OTA_URL_SIZE 256

extern const uint8_t github_server_cert_pem_start[] asm("_binary_github_cert_pem_start");
extern const uint8_t github_server_cert_pem_end[] asm("_binary_github_cert_pem_end");

void ota_update_fw_task(void *pvParameter);
void advanced_ota_example_task(void *pvParameter);
