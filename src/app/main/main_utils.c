#include "main_utils.h"

static const char *TAG = "MAIN_UTILS";
// Helper function to check for non-printable characters
bool is_valid_id_string(const char *str, size_t max_len)
{
    for (size_t i = 0; i < max_len && str[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)str[i];
        if (!isprint(c))
        {
            return false;
        }
    }
    return true;
}

// Helper function to check if a string contains only numeric characters
bool is_numeric_string(const char *str, size_t max_len)
{
    if (str == NULL)
    {
        return false;
    }

    if (str[0] == '\0')
    {
        return false;
    }

    for (size_t i = 0; i < max_len && str[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)str[i]))
        {
            return false;
        }
    }
    return true;
}

void log_elapsed_time(char *auth_type, int64_t start_time)
{
    int64_t end_time = esp_timer_get_time();
    double elapsed_time_sec = (end_time - start_time) / 1000000.0;
#ifdef UTILS_DEBUG
    ESP_LOGI(TAG, ANSI_COLOR_BLUE "Time taken for %s: %.6f seconds" ANSI_COLOR_RESET, auth_type, elapsed_time_sec);
#endif
}

void heap_monitor_task(void *pvParameters)
{
    while (1)
    {
        long unsigned int free_heap = (long unsigned int)esp_get_free_heap_size();
        #ifdef UTILS_DEBUG
        ESP_LOGI(TAG, ANSI_COLOR_BLUE "Free heap: %lu bytes" ANSI_COLOR_RESET, free_heap);
        #endif
        if (free_heap < HEAP_WARNING_THRESHOLD)
        {
#ifdef UTILS_DEBUG
            ESP_LOGE(TAG, "CRITICAL: Heap size below safe limit! Free heap: %lu bytes", free_heap);
#endif
        }

        vTaskDelay(pdMS_TO_TICKS(HEAP_MONITOR_INTERVAL));
    }
}

void check_task_creation(char *taskName, TaskHandle_t taskHandle)
{
    if (taskHandle == NULL)
    {
#ifdef UTILS_DEBUG
        ESP_LOGE(TAG, "%s creation failed!", taskName);
#endif
    }
    else
    {
#ifdef UTILS_DEBUG
        ESP_LOGE(TAG, "%s created successfully!", taskName);
#endif
    }
}

void teardown_task(TaskHandle_t *taskHandle)
{
    if (*taskHandle != NULL)
    {
        vTaskDelete(*taskHandle);
        *taskHandle = NULL;
    }
}