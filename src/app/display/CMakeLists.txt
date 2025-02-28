idf_component_register(SRCS "lvgl_demo_ui.c" "display_frames.c" "display_config.c" 
                    INCLUDE_DIRS "."
                    REQUIRES "esp_timer" "esp_lcd" "lvgl")