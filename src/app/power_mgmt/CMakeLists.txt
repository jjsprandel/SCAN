idf_component_register(SRCS "power_mgmt.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES bq25798 cypd3177 mqtt_client database) 