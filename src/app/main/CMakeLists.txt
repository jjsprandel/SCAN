idf_component_register(SRCS "state_enum.c" "main.c" "global.c" "main_utils.c" "admin_mode.c"
                    INCLUDE_DIRS "." "./include" ""
                    
                    REQUIRES wifi esp_driver_gpio ota_update database NDEF keypad display misc i2c cypd3177 bq25798 pcf8574n buzzer mqtt_client power_mgmt

                    PRIV_REQUIRES nvs_flash)
