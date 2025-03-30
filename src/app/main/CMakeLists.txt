idf_component_register(SRCS "state_enum.c" "main.c"
                    INCLUDE_DIRS "." "./include"
                    
                    REQUIRES wifi esp_driver_gpio ota_update database NDEF keypad_driver display admin_mode misc buzzer

                    PRIV_REQUIRES nvs_flash)