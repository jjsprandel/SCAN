#include "status_buzzer.h"

static Buzzer *kiosk_buzzer = NULL;
static const char *TAG = "status_buzzer";

// Define music note frequencies (in Hz)
// C3 through B5
#define NOTE_C3 131
#define NOTE_C4 262
// #define NOTE_CS4 277
// #define NOTE_D4 294
// #define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
// #define NOTE_FS4 370
#define NOTE_G4 392
// #define NOTE_GS4 415
#define NOTE_A4 440
// #define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
// #define NOTE_CS5 554
#define NOTE_D5 587
// #define NOTE_DS5 622
#define NOTE_E5 659
// #define NOTE_F5 698
// #define NOTE_FS5 740
#define NOTE_G5 784
// #define NOTE_GS5 831
// #define NOTE_A5 880
// #define NOTE_AS5 932
// #define NOTE_B5 988

// Rest note
#define NOTE_REST 0

void buzzer_init()
{
    esp_err_t err;
    kiosk_buzzer = Buzzer_Create();
    if (kiosk_buzzer == NULL)
    {
        ESP_LOGE(TAG, "Failed to create buzzer\n");
        return;
    }
    err = Buzzer_Init(kiosk_buzzer, BUZZER_GPIO, LEDC_AUTO_CLK, LEDC_LOW_SPEED_MODE,
                      LEDC_TIMER_13_BIT, LEDC_TIMER_0, LEDC_CHANNEL_0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Buzzer_Init failed\n");
        return;
    }
#ifdef BUZZER_DEBUG
    ESP_LOGI(TAG, "Buzzer initialized");
#endif
}

// Helper function to create melody notes with short gaps between them
static BuzzerNote *create_melody_with_gaps(const int *frequencies, const int *durations, float volume, int note_count, size_t *total_notes)
{
    // Each note will have a gap, so total will be 2 * note_count
    *total_notes = note_count * 2;

    // Allocate memory for all notes plus gaps
    BuzzerNote *notes = (BuzzerNote *)malloc(*total_notes * sizeof(BuzzerNote));
    if (notes == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation failed for melody notes");
        return NULL;
    }

    for (int i = 0; i < note_count; i++)
    {
        // The note
        notes[i * 2].frequency = frequencies[i];
        notes[i * 2].duration_ms = durations[i];
        notes[i * 2].volume = (frequencies[i] == NOTE_REST) ? 0.0f : volume;

        // The gap (short silence between notes)
        notes[i * 2 + 1].frequency = NOTE_REST;
        notes[i * 2 + 1].duration_ms = 20; // 20ms gap
        notes[i * 2 + 1].volume = 0.0f;
    }

    return notes;
}

void play_kiosk_buzzer(state_t current_state, admin_state_t current_admin_state)
{
    BuzzerMusic music;
    size_t total_notes = 0;
    BuzzerNote *notes = NULL;

    if (current_state != STATE_ADMIN_MODE)
    {
        switch (current_state)
        {
        case STATE_HARDWARE_INIT:
        {
            // Hardware initialization sound - low frequency beep
            static const int frequencies[] = {NOTE_C3};
            static const int durations[] = {200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_LOW, 1, &total_notes);
            break;
        }
        case STATE_WIFI_CONNECTING:
        {
            // WiFi connecting sound - rising pattern
            static const int frequencies[] = {NOTE_C4, NOTE_E4, NOTE_G4};
            static const int durations[] = {100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 3, &total_notes);
            break;
        }
        case STATE_SOFTWARE_INIT:
        {
            // Software initialization sound - descending pattern
            static const int frequencies[] = {NOTE_G4, NOTE_E4, NOTE_C4};
            static const int durations[] = {100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 3, &total_notes);
            break;
        }
        case STATE_SYSTEM_READY:
        {
            // System ready sound - startup chime
            static const int frequencies[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
            static const int durations[] = {100, 100, 100, 250};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 4, &total_notes);
            break;
        }
        case STATE_USER_DETECTED:
        {
            // Attention-grabbing alert - quick rising beep
            static const int frequencies[] = {NOTE_C5, NOTE_E5, NOTE_G5};
            static const int durations[] = {80, 80, 150};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 3, &total_notes);
            break;
        }
        case STATE_CHECK_IN:
        {
            // Success melody - happy, uplifting
            static const int frequencies[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_C5};
            static const int durations[] = {100, 100, 100, 150, 200, 250};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 6, &total_notes);
            break;
        }
        case STATE_CHECK_OUT:
        {
            // Check-out confirmation - descending pattern
            static const int frequencies[] = {NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4};
            static const int durations[] = {150, 100, 100, 250};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 4, &total_notes);
            break;
        }
        case STATE_VALIDATION_FAILURE:
        {
            // Error sound - descending minor third
            static const int frequencies[] = {NOTE_A4, NOTE_F4, NOTE_A4, NOTE_F4};
            static const int durations[] = {200, 300, 200, 300};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 4, &total_notes);
            break;
        }
        case STATE_ADMIN_MODE:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case STATE_ERROR:
        {
            // System error - alarming, attention-grabbing
            static const int frequencies[] = {NOTE_A4, NOTE_REST, NOTE_A4, NOTE_REST, NOTE_A4};
            static const int durations[] = {200, 100, 200, 100, 400};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_HIGH, 5, &total_notes);
            break;
        }
        default:
        {
            // No beep for other states
            // Single beep for other states
            // BuzzerNote *note = (BuzzerNote *)malloc(sizeof(BuzzerNote));
            // if (note != NULL)
            // {
            //     note->frequency = NOTE_C4;
            //     note->duration_ms = 100;
            //     note->volume = VOLUME_LOW;
            //     notes = note;
            //     total_notes = 1;
            // }
            break;
        }
        }
    }
    else
    {
        switch (current_admin_state)
        {
        case ADMIN_STATE_ENTER_ID:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_VALIDATE_ID:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_TAP_CARD:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_CARD_WRITE_SUCCESS:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_ENTER_ID_ERROR:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_CARD_WRITE_ERROR:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        case ADMIN_STATE_ERROR:
        {
            // Admin mode entry - distinctive sequence
            static const int frequencies[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4};
            static const int durations[] = {100, 100, 100, 200, 100, 100, 200};
            notes = create_melody_with_gaps(frequencies, durations, VOLUME_MEDIUM, 7, &total_notes);
            break;
        }
        default:
        {
            // No beep for other states
            break;
        }
        }
    }

    // Play the generated music sequence
    if (kiosk_buzzer != NULL && notes != NULL)
    {
        music.notes = notes;
        music.count = total_notes;

        if (Buzzer_Play(kiosk_buzzer, &music) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to play buzzer melody for state %d", current_state);
        }

        // Free our local copy of the notes (Buzzer_Play makes its own copy)
        free(notes);
    }
    else
    {
        if (kiosk_buzzer == NULL)
        {
            ESP_LOGE(TAG, "Buzzer not initialized");
        }
        if (notes == NULL)
        {
            ESP_LOGE(TAG, "Failed to create melody");
        }
    }
}