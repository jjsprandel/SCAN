#ifndef BUZZER_H
#define BUZZER_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include <esp_err.h>
#include <stdint.h>
#include <stddef.h>
#include "admin_mode.h"
/**
 * @brief Structure representing a buzzer note.
 */
typedef struct
{
    int frequency;   // e.g. default 4000 Hz
    int duration_ms; // e.g. default 500 ms
    float volume;    // Range: 0.0f ~ 1.0f (e.g. default 0.1f)
} BuzzerNote;

/**
 * @brief Structure representing a sequence (music) for the buzzer.
 *        It holds an array of notes and the number of notes.
 */
typedef struct
{
    BuzzerNote *notes;
    size_t count;
} BuzzerMusic;

/* Structure to hold a melody pair.
   The first element is an index into the music_keys array,
   and the second is a duration factor. */
typedef struct
{
    int note_index;
    int duration_factor;
} MelodyPair;

/**
 * @brief Opaque buzzer handle.
 */
typedef struct Buzzer Buzzer;

/**
 * @brief Create a buzzer object.
 */
Buzzer *Buzzer_Create(void);

/**
 * @brief Initialize the buzzer.
 *
 * @param buzzer Pointer returned by Buzzer_Create().
 * @param gpio_num Buzzer GPIO number.
 * @param clk_cfg Clock configuration for LEDC.
 * @param speed_mode LEDC speed mode.
 * @param timer_bit LEDC timer resolution.
 * @param timer_num LEDC timer number.
 * @param ledc_channel LEDC channel.
 * @param idle_level LEDC idle level.
 *
 * @return ESP_OK on success, ESP_FAIL on error.
 */
esp_err_t Buzzer_Init(Buzzer *buzzer, const gpio_num_t gpio_num, ledc_clk_cfg_t clk_cfg,
                      ledc_mode_t speed_mode, ledc_timer_bit_t timer_bit, ledc_timer_t timer_num,
                      ledc_channel_t ledc_channel, uint32_t idle_level);

/**
 * @brief Deinitialize (release) the buzzer.
 */
esp_err_t Buzzer_Deinit(Buzzer *buzzer);

/**
 * @brief Play a single beep note.
 */
esp_err_t Buzzer_Beep(Buzzer *buzzer, const BuzzerNote *note);

/**
 * @brief Play a sequence of notes.
 */
esp_err_t Buzzer_Play(Buzzer *buzzer, const BuzzerMusic *music);

/**
 * @brief Stop any currently playing buzzer sound.
 */
esp_err_t Buzzer_Stop(Buzzer *buzzer);

void example_buzzer_program();

#endif // BUZZER_H
