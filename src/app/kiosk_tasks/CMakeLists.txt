idf_component_register(
    SRCS "kiosk_tasks.c"
    INCLUDE_DIRS "."
    REQUIRES "display" "esp_driver_gpio" "pn532" "keypad_driver" "NDEF" "misc" "wifi" "database" "ota_update" 
    PRIV_REQUIRES "nvs_flash"
    )
