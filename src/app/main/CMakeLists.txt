idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES wifi esp_driver_gpio ota_update database NDEF keypad_driver display misc
                    PRIV_REQUIRES nvs_flash)