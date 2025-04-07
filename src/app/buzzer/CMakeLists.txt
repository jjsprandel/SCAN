idf_component_register(SRCS "buzzer.c" "status_buzzer.c"
                    INCLUDE_DIRS "." "./include"
                    REQUIRES "driver" "main")