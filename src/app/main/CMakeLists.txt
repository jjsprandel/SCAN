idf_component_register(
    SRCS "main.c" 
    INCLUDE_DIRS "."
    REQUIRES "kiosk_tasks" "wifi" "ota_update"
    PRIV_REQUIRES "nvs_flash"
    )
