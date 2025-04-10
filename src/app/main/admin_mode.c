#include "admin_mode.h"

static const char *ADMIN_TAG = "admin_mode";
admin_state_t current_admin_state = ADMIN_STATE_BEGIN;
static uint8_t invalid_id_attempts = 0;
char user_id_to_write[ID_LEN+1];

void admin_mode_control_task(void *param)
{
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
            invalid_id_attempts = 0;
            current_admin_state = ADMIN_STATE_ENTER_ID;
            break;

        case ADMIN_STATE_ENTER_ID:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Enter a valid ID # on the keypad to write to the card");
#endif
            BaseType_t keypadNotify = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            if (keypadNotify > 0)
            {
                if (keypad_buffer.occupied != ID_LEN){
#ifdef ADMIN_DEBUG
                    ESP_LOGE(ADMIN_TAG, "Invalid ID length. Expected %d characters, got %d", ID_LEN, keypad_buffer.occupied);
#endif
                    current_admin_state = (invalid_id_attempts >= NUM_ID_ATTEMPTS - 1) ? ADMIN_STATE_ERROR : ADMIN_STATE_ENTER_ID_ERROR;
                }
                else{
                    memcpy(user_id_to_write, keypad_buffer.elements, ID_LEN);
                    user_id_to_write[ID_LEN] = '\0';

                    current_admin_state = ADMIN_STATE_VALIDATE_ID;
                }
            }
            break;

        case ADMIN_STATE_VALIDATE_ID:
#ifdef ADMIN_DEBUG
            ESP_LOGI(ADMIN_TAG, "Sending ID %s to database for validation", user_id_to_write);
#endif
#ifdef DATABASE_QUERY_ENABLED
            if (!get_user_info(user_id_to_write))
            {
                ESP_LOGE(ADMIN_TAG, "Error validating ID in database. Try again.");
                if (invalid_id_attempts >= NUM_ID_ATTEMPTS)
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
                current_admin_state = ADMIN_STATE_TAP_CARD;
            }
            else
            {
                ESP_LOGE(ADMIN_TAG, "User is not an active student. Try again.");
                if (invalid_id_attempts >= NUM_ID_ATTEMPTS)
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
            ESP_LOGI(ADMIN_TAG, "Tap a blank NTAG213 card for writing ID %s for user %s %s", user_id_to_write, user_info->first_name, user_info->last_name);
#endif
            const char *card_id_to_write = user_id_to_write;
            bool nfcWriteFlag = write_user_id(card_id_to_write, 50);

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
            if (keypad_buffer.occupied != ID_LEN)
            {
                #ifdef ADMIN_DEBUG
                ESP_LOGE(ADMIN_TAG, "Invalid ID length. ID length got %d, expected %d", keypad_buffer.occupied, ID_LEN);
                #endif
            } else 
            {
                #ifdef ADMIN_DEBUG
                ESP_LOGE(ADMIN_TAG, "Error validating ID in database. Try again.");
                #endif
            }
            invalid_id_attempts++;
            clear_buffer();
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