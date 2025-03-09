idf_component_register(SRCS "keypad_driver.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES driver esp_driver_gpio)