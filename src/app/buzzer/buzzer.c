#include "buzzer.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/ledc_ll.h"  // Note: adjust the path if needed
#include <stdlib.h>

static const char* TAG = "buzzer";
static const uint32_t STACK_SIZE = 2048;

/* Internal structure that holds buzzer state */
typedef struct {
    TaskHandle_t task_handle;
    SemaphoreHandle_t semaphore_handle;
    ledc_mode_t ledc_speed_mode;
    ledc_timer_bit_t ledc_timer_bit;
    ledc_timer_t ledc_timer_num;
    ledc_channel_t ledc_channel;
    uint32_t ledc_idle_level;
    const BuzzerMusic* music;
} BuzzerInternal;

struct Buzzer {
    BuzzerInternal* internal;
};

static uint32_t clamp_uint32(uint32_t value, uint32_t min, uint32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float clamp_float(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static void set_buzzer_note(BuzzerInternal* internal, uint32_t frequency, float volume) {
    /* Clamp frequency between 1<<LEDC_LL_FRACTIONAL_BITS and 1<<timer_bit */
    frequency = clamp_uint32(frequency, (1UL << LEDC_LL_FRACTIONAL_BITS), (1UL << internal->ledc_timer_bit));
    volume = clamp_float(volume, 0.0f, 1.0f);
    uint32_t duty = (1UL << internal->ledc_timer_bit) * volume;
    ESP_ERROR_CHECK(ledc_set_duty(internal->ledc_speed_mode, internal->ledc_channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(internal->ledc_speed_mode, internal->ledc_channel));
    ESP_ERROR_CHECK(ledc_set_freq(internal->ledc_speed_mode, internal->ledc_timer_num, frequency));
}

static void stop_buzzer(BuzzerInternal* internal) {
    ESP_ERROR_CHECK(ledc_set_duty(internal->ledc_speed_mode, internal->ledc_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(internal->ledc_speed_mode, internal->ledc_channel));
    ESP_ERROR_CHECK(ledc_stop(internal->ledc_speed_mode, internal->ledc_channel, internal->ledc_idle_level));
}

static void buzzer_task(void* pvParameters) {
    BuzzerInternal* internal = (BuzzerInternal*) pvParameters;
    while (1) {
        if (xSemaphoreTake(internal->semaphore_handle, portMAX_DELAY) == pdTRUE) {
            int new_task = 1;
            while (new_task) {
                new_task = 0;
                const BuzzerMusic* playing = internal->music;
                if (playing != NULL) {
                    for (size_t i = 0; i < playing->count; i++) {
                        BuzzerNote note = playing->notes[i];
                        set_buzzer_note(internal, note.frequency, note.volume);
                        /* Wait for the duration of the note. If a new request comes in during this time, break early to process the new task. */
                        if (xSemaphoreTake(internal->semaphore_handle, pdMS_TO_TICKS(note.duration_ms)) == pdTRUE) {
                            new_task = 1;
                            break;
                        }
                    }

                    free((void*)playing->notes);
                    free((void*)playing);
                    internal->music = NULL;
                }
                stop_buzzer(internal);
            }
        }
    }
}

/* Internal initialization for BuzzerInternal */
static esp_err_t buzzer_internal_init(BuzzerInternal* internal, const gpio_num_t gpio_num, ledc_clk_cfg_t clk_cfg,
                                        ledc_mode_t speed_mode, ledc_timer_bit_t timer_bit, ledc_timer_t timer_num,
                                        ledc_channel_t ledc_channel, uint32_t idle_level) {
    internal->ledc_speed_mode = speed_mode;
    internal->ledc_timer_bit = timer_bit;
    internal->ledc_timer_num = timer_num;
    internal->ledc_channel = ledc_channel;
    internal->ledc_idle_level = idle_level;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = speed_mode,
        .duty_resolution = timer_bit,
        .timer_num = timer_num,
        .freq_hz = 6000, // Changed from 4000
        .clk_cfg = clk_cfg,
        .deconfigure = false,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t channel_config = {
        .gpio_num = gpio_num,
        .speed_mode = speed_mode,
        .channel = ledc_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer_num,
        .duty = 0,
        .hpoint = 0,
        .flags = {0},
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
    ESP_ERROR_CHECK(ledc_stop(speed_mode, ledc_channel, idle_level));

    internal->semaphore_handle = xSemaphoreCreateBinary();
    if (internal->semaphore_handle == NULL) {
        return ESP_FAIL;
    }
    BaseType_t res = xTaskCreate(buzzer_task, "buzzer_task", STACK_SIZE, internal, uxTaskPriorityGet(NULL), &(internal->task_handle));
    if (res == pdPASS) {
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

/* Create a new Buzzer object */
Buzzer* Buzzer_Create(void) {
    Buzzer* buzzer = (Buzzer*) malloc(sizeof(Buzzer));
    if (buzzer != NULL) {
        buzzer->internal = NULL;
    }
    return buzzer;
}

/* Initialize the buzzer hardware and internal task */
esp_err_t Buzzer_Init(Buzzer* buzzer, const gpio_num_t gpio_num, ledc_clk_cfg_t clk_cfg,
                      ledc_mode_t speed_mode, ledc_timer_bit_t timer_bit, ledc_timer_t timer_num,
                      ledc_channel_t ledc_channel, uint32_t idle_level) {
    if (buzzer == NULL) {
        return ESP_FAIL;
    }
    if (buzzer->internal != NULL) {
        ESP_LOGE(TAG, "Buzzer already initialized, deinit before init again.");
        return ESP_FAIL;
    }
    buzzer->internal = (BuzzerInternal*) malloc(sizeof(BuzzerInternal));
    if (buzzer->internal == NULL) {
        return ESP_FAIL;
    }
    buzzer->internal->task_handle = NULL;
    buzzer->internal->semaphore_handle = NULL;
    buzzer->internal->music = NULL;
    esp_err_t err = buzzer_internal_init(buzzer->internal, gpio_num, clk_cfg, speed_mode, timer_bit, timer_num, ledc_channel, idle_level);
    if (err != ESP_OK) {
        free(buzzer->internal);
        buzzer->internal = NULL;
    }
    return err;
}

/* Deinitialize the buzzer, stopping its task and releasing resources */
esp_err_t Buzzer_Deinit(Buzzer* buzzer) {
    if (buzzer == NULL || buzzer->internal == NULL) {
        return ESP_OK;
    }
    if (buzzer->internal->task_handle != NULL) {
        vTaskDelete(buzzer->internal->task_handle);
    }
    if (buzzer->internal->semaphore_handle != NULL) {
        vSemaphoreDelete(buzzer->internal->semaphore_handle);
    }
    stop_buzzer(buzzer->internal);
    free(buzzer->internal);
    buzzer->internal = NULL;
    return ESP_OK;
}

/* Play a single note (beep) by creating a one-note music sequence */
esp_err_t Buzzer_Beep(Buzzer* buzzer, const BuzzerNote* note) {
    if (buzzer == NULL || buzzer->internal == NULL) {
        ESP_LOGE(TAG, "Buzzer not initialized");
        return ESP_FAIL;
    }

    BuzzerNote* note_copy = (BuzzerNote*) malloc(sizeof(BuzzerNote));
    if (!note_copy) return ESP_FAIL;
    *note_copy = *note;
    /* Allocate a BuzzerMusic structure for one note */
    BuzzerMusic* music_ptr = (BuzzerMusic*) malloc(sizeof(BuzzerMusic));
    if (!music_ptr) {
        free(note_copy);
        return ESP_FAIL;
    }
    music_ptr->notes = note_copy;
    music_ptr->count = 1;
    buzzer->internal->music = music_ptr;
    xSemaphoreGive(buzzer->internal->semaphore_handle);
    return ESP_OK;
}

/* Play a sequence of notes.
   The caller provides a BuzzerMusic structure (with an array of notes and count).
   This function makes a copy of the music so the caller’s memory can be reused. */
esp_err_t Buzzer_Play(Buzzer* buzzer, const BuzzerMusic* music) {
    if (buzzer == NULL || buzzer->internal == NULL) {
        ESP_LOGE(TAG, "Buzzer not initialized");
        return ESP_FAIL;
    }
    BuzzerMusic* new_music = (BuzzerMusic*) malloc(sizeof(BuzzerMusic));
    if (new_music == NULL) {
        return ESP_FAIL;
    }
    new_music->count = music->count;
    new_music->notes = malloc(sizeof(BuzzerNote) * music->count);
    if (new_music->notes == NULL) {
        free(new_music);
        return ESP_FAIL;
    }
    for (size_t i = 0; i < music->count; i++) {
        new_music->notes[i] = music->notes[i];
    }
    buzzer->internal->music = new_music;
    xSemaphoreGive(buzzer->internal->semaphore_handle);
    return ESP_OK;
}

/* Stop playing by setting the music pointer to NULL and giving the semaphore. */
esp_err_t Buzzer_Stop(Buzzer* buzzer) {
    if (buzzer == NULL || buzzer->internal == NULL) {
        ESP_LOGE(TAG, "Buzzer not initialized");
        return ESP_FAIL;
    }
    buzzer->internal->music = NULL;
    xSemaphoreGive(buzzer->internal->semaphore_handle);
    return ESP_OK;
}

void example_buzzer_program(Buzzer* kiosk_buzzer){
    esp_err_t err;

    /* Create and initialize the buzzer.
       We assume LEDC_AUTO_CLK, LEDC_LOW_SPEED_MODE, etc. are defined in ESP-IDF. */
    kiosk_buzzer = Buzzer_Create();
    if (kiosk_buzzer == NULL) {
        ESP_LOGE(TAG,"Failed to create buzzer\n");
        return;
    }
    err = Buzzer_Init(kiosk_buzzer, GPIO_NUM_11, LEDC_AUTO_CLK, LEDC_LOW_SPEED_MODE,
                      LEDC_TIMER_13_BIT, LEDC_TIMER_0, LEDC_CHANNEL_0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"Buzzer_Init failed\n");
        return;
    }

    /* Frequency test: Beep with increasing frequency. In C we can use a compound literal to create a BuzzerNote. */
    for (int i = 1; i < 800; i++) {
        BuzzerNote note = { .frequency = 10 * i, .duration_ms = 10, .volume = 0.01f };
        err = Buzzer_Beep(kiosk_buzzer, &note);
        if (err != ESP_OK) {
            ESP_LOGE(TAG,"Buzzer_Beep failed at frequency %d\n", 10 * i);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /* Simple interactive sound: play two short notes. */
    BuzzerNote simpleNotes[2] = {
        { .frequency = 800, .duration_ms = 200, .volume = 0.10f },
        { .frequency = 500, .duration_ms = 200, .volume = 0.10f }
    };
    BuzzerMusic simpleMusic;
    simpleMusic.notes = simpleNotes;
    simpleMusic.count = 2;

    for (int i = 0; i < 20; i++){
        err = Buzzer_Play(kiosk_buzzer, &simpleMusic);
        if (err != ESP_OK) {
            ESP_LOGE(TAG,"Buzzer_Play (simple) failed\n");
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    int music_keys[] = {
        0,    // index 0, not used for sound
        262,  294,  330,  349,  392,  440,  494,   // low
        523,  587,  659,  698,  784,  880,  988,   // mid
        1046, 1175, 1318, 1397, 1568, 1760, 1976   // high
    };

    /* Define a melody as an array of note index–duration pairs.
       For example, {3, 4} means use music_keys[3] as the note frequency,
       with a duration factor of 4. */
    MelodyPair melody[] = {
        {3, 4}, {3, 2}, {5, 2}, {6, 2}, {8, 2}, {8, 2}, {6, 2}, {5, 4}, {5, 2}, {6, 2}, {5, 8}
    };
    size_t num_pairs = sizeof(melody) / sizeof(melody[0]);

    size_t total_notes = num_pairs * 2;
    BuzzerNote* notes = malloc(total_notes * sizeof(BuzzerNote));
    if (notes == NULL) {
        ESP_LOGE(TAG,"Memory allocation failed for melody notes\n");
        return;
    }

    uint32_t total_length = 0;
    for (size_t i = 0; i < num_pairs; i++) {
        int note_idx = melody[i].note_index;
        int duration_factor = melody[i].duration_factor;
        
        notes[i * 2].frequency = music_keys[note_idx];
        notes[i * 2].duration_ms = 150 * duration_factor;
        notes[i * 2].volume = 0.01f;
        total_length += notes[i * 2].duration_ms;

        notes[i * 2 + 1].frequency = 0;
        notes[i * 2 + 1].duration_ms = 10;
        notes[i * 2 + 1].volume = 0.0f;
        total_length += notes[i * 2 + 1].duration_ms;
    }

    BuzzerMusic music;
    music.notes = notes;
    music.count = total_notes;
    err = Buzzer_Play(kiosk_buzzer, &music);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"Buzzer_Play (melody) failed\n");
    }

    vTaskDelay(pdMS_TO_TICKS(total_length / 10));
    err = Buzzer_Stop(kiosk_buzzer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"Buzzer_Stop failed\n");
    }
    err = Buzzer_Play(kiosk_buzzer, &music);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"Buzzer_Play (repeat) failed\n");
    }

    free(notes);
}