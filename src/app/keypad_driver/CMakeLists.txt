idf_component_register(SRCS "keypad_driver.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_driver_gpio)