idf_component_register(SRCS "cypd3177_i2c.c" "cypd3177_intr.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES esp_driver_gpio esp_driver_i2c)