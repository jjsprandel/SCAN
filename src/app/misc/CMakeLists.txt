idf_component_register(SRCS "pir_sensor.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES "esp_driver_gpio" "esp_driver_ledc" "espressif__led_strip" "main")