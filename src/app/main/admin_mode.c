#include "admin_mode.h"

static const char *ADMIN_TAG = "admin_mode";
admin_state_t current_admin_state = ADMIN_STATE_BEGIN;
static uint8_t invalid_id_attempts = 0;

void admin_mode_control_task(void *param)
{
    char user_id_to_write[ID_LEN];
    while (1)
    {
        switch (current_admin_state)
        {
        case ADMIN_STATE_BEGIN:
            BaseType_t adminBegin = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Admin Mode Control Task Started");
#endif
            clear_buffer();
            current_admin_state = ADMIN_STATE_ENTER_ID;
            break;

        case ADMIN_STATE_ENTER_ID:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Enter a valid ID # on the keypad to write to the card");
#endif
            BaseType_t keypadNotify = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            if (keypadNotify > 0)
            {
                memcpy(user_id_to_write, keypad_buffer.elements, ID_LEN);
                current_admin_state = ADMIN_STATE_VALIDATE_ID;
            }
            break;

        case ADMIN_STATE_VALIDATE_ID:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Sending ID to database for validation");
#endif
#ifdef DATABASE_QUERY_ENABLED
            if (!get_user_info(user_id_to_write))
            {
                ESP_LOGE(ADMIN_TAG, "Error validating ID in database. Try again.");
                invalid_id_attempts++;
                if (invalid_id_attempts >= 3)
                {
                    ESP_LOGE(ADMIN_TAG, "Maximum number of invalid ID attempts reached. Exiting admin mode.");
                    current_admin_state = ADMIN_STATE_ERROR;
                }
                else
                {
                    current_admin_state = ADMIN_STATE_ENTER_ID_ERROR;
                }
            }
            else if (strcmp(user_info->active_user, "Yes") == 0)
            {
                ESP_LOGI(ADMIN_TAG, "ID validated in database");
                invalid_id_attempts = 0;

                // Update user info display
                char full_name[64];
                snprintf(full_name, sizeof(full_name), "%s %s", user_info->first_name, user_info->last_name);
                ui_update_user_info(full_name, user_id_to_write);
                ESP_LOGI(ADMIN_TAG, "Updated UI with name: %s, ID: %s", full_name, user_id_to_write);

                current_admin_state = ADMIN_STATE_TAP_CARD;
            }
            else
            {
                ESP_LOGE(ADMIN_TAG, "User is not an active student. Try again.");
                invalid_id_attempts++;
                if (invalid_id_attempts >= 3)
                {
                    ESP_LOGE(ADMIN_TAG, "Maximum number of invalid ID attempts reached. Exiting admin mode.");
                    current_admin_state = ADMIN_STATE_ERROR;
                }
                else
                {
                    current_admin_state = ADMIN_STATE_ENTER_ID_ERROR;
                }
            }
#else
            ESP_LOGI(ADMIN_TAG, "ID validated in database");
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_admin_state = ADMIN_STATE_TAP_CARD;
#endif
            break;

        case ADMIN_STATE_TAP_CARD:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Tap a blank NTAG213 card for writing");
#endif

            bool nfcWriteFlag = write_user_id(user_id_to_write, 50);

            if (nfcWriteFlag)
            {
                ESP_LOGI(ADMIN_TAG, "ID %s Successfully Written to Card", user_id_to_write);
                current_admin_state = ADMIN_STATE_CARD_WRITE_SUCCESS;
            }
            break;

        case ADMIN_STATE_CARD_WRITE_SUCCESS:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "ID Successfully Written to Card");
#endif
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_admin_state = ADMIN_STATE_BEGIN;
            xTaskNotifyGive(state_control_task_handle);
            break;
        case ADMIN_STATE_CARD_WRITE_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error writing ID to card. Try again.");
#endif
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_admin_state = ADMIN_STATE_TAP_CARD;
            break;
        case ADMIN_STATE_ENTER_ID_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error validating ID in database. Try again.");
#endif
            xTaskNotifyGive(keypad_task_handle);
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_admin_state = ADMIN_STATE_ENTER_ID;
            break;
        case ADMIN_STATE_ERROR:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Error encountered in Admin Mode. Returning to STATE_ADMIN_BEGIN");
#endif
            vTaskDelay(pdMS_TO_TICKS(5000));
            current_admin_state = ADMIN_STATE_BEGIN;
            xTaskNotifyGive(state_control_task_handle);
            break;

        default:
#ifdef ADMIN_DEBUG
            ESP_LOGE(ADMIN_TAG, "Unknown state encountered: %d. Exiting admin mode.", current_admin_state);
#endif
            vTaskDelay(pdMS_TO_TICKS(5000));
            vTaskDelete(NULL);
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}