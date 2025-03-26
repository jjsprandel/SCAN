idf_component_register(SRCS "keypad.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES driver esp_driver_gpio esp_driver_i2c pcf8574n)