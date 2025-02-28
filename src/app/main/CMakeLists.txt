idf_component_register(
    SRCS "main.c" 
    INCLUDE_DIRS "."
    REQUIRES "kiosk_tasks" "wifi" "misc"
    PRIV_REQUIRES "nvs_flash"
    )
