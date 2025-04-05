idf_component_register(SRCS "mqtt.c"
                    INCLUDE_DIRS "." "./include"
                    EMBED_TXTFILES ${project_dir}/server_certs/hivemq_cert.pem
                    REQUIRES mqtt esp_partition nvs_flash esp_netif app_update esp_https_ota ota_update)