#include "status_buzzer.h"

static Buzzer* kiosk_buzzer = NULL;
static const char* TAG = "status_buzzer";

void buzzer_init()
{
    esp_err_t err;
    kiosk_buzzer = Buzzer_Create();
    if (kiosk_buzzer == NULL) 
    {
        ESP_LOGE(TAG,"Failed to create buzzer\n");
        return;
    }
    err = Buzzer_Init(kiosk_buzzer, BUZZER_GPIO, LEDC_AUTO_CLK, LEDC_LOW_SPEED_MODE,
                      LEDC_TIMER_13_BIT, LEDC_TIMER_0, LEDC_CHANNEL_0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,"Buzzer_Init failed\n");
        return;
    }
    #ifdef BUZZER_DEBUG
    ESP_LOGI(TAG,"Buzzer initialized");
    #endif
}

void play_kiosk_buzzer(state_t current_state)
{
    // BuzzerMusic music;
    BuzzerNote note;

    switch (current_state)
    {
        case STATE_WIFI_INIT:
        {
            note.frequency = 4000;
            note.duration_ms = 200;
            note.volume = VOLUME_LOW;
            break;
        }
        case STATE_USER_DETECTED:
        {
            note.frequency = 4000;
            note.duration_ms = 150;
            note.volume = VOLUME_HIGH;
            break;
        }
        case STATE_CHECK_IN:
        {
            note.frequency = 800;
            note.duration_ms = 500;
            note.volume = VOLUME_HIGH;
            break;
        }
        case STATE_CHECK_OUT:
        {
            note.frequency = 800;
            note.duration_ms = 500;
            note.volume = VOLUME_HIGH;
            break;
        }
        case STATE_VALIDATION_FAILURE:
        {
            note.frequency = 800;
            note.duration_ms = 500;
            note.volume = VOLUME_HIGH;
            break;
        }
        default:
        {
            note.frequency = 0;
            note.duration_ms = 100;
            note.volume = VOLUME_OFF;
            break;
        }
    }

    /* Play the generated music sequence */
    if (kiosk_buzzer != NULL) {
        if (Buzzer_Beep(kiosk_buzzer, &note) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to play buzzer tone for state %d", current_state);
        }
    } else {
        ESP_LOGE(TAG, "Buzzer not initialized");
    }
}