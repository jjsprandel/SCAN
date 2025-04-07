#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>

#define CONFIG_BROKER_URI "mqtts://0ec065087cf84d309f1c73b00c9441f8.s1.eu.hivemq.cloud:8883"

void mqtt_init(void);
void mqtt_start_ping_task(void);
void mqtt_publish_status(const char *status);

#endif // MQTT_H

