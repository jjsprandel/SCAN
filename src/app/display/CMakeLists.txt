idf_component_register(SRCS "display_frames.c" "display_config.c" "admin_frames.c" "ui_styles.c" "ui_screens.c"
                     # Add manually created image C files
                     "image_files/admin_icon.c"
                     "image_files/arrow_icon.c"
                     "image_files/database_icon.c"
                     "image_files/error_icon.c"
                     "image_files/id_card_icon.c"
                     "image_files/keypad_icon.c"
                     "image_files/wifi_icon.c"
                     "image_files/write_id_icon.c"
                     "image_files/check_icon.c"
                     "image_files/ucf_icon.c"
                     INCLUDE_DIRS "." "image_files"
                     REQUIRES "esp_timer" "esp_lcd" "lvgl")