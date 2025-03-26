idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "." "./include" ""
                    REQUIRES wifi esp_driver_gpio ota_update database NDEF keypad display admin_mode misc i2c cypd3177 pcf8574n
                    PRIV_REQUIRES nvs_flash)