idf_component_register(SRCS "pir.c" "buzzer.c"
                    INCLUDE_DIRS "."
                    REQUIRES "esp_driver_gpio" "esp_driver_ledc")