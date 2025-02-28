#include "buzzer.h"

static const char *TAG = "BUZZER";

uint8_t BuzzerState = 0;

EventGroupHandle_t alarm_eventgroup;

void sound(int gpio_num, uint32_t freq, uint32_t duration)
{

    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = GPIO_OUTPUT_SPEED;
    timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = freq;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.gpio_num = gpio_num;
    ledc_conf.speed_mode = GPIO_OUTPUT_SPEED;
    ledc_conf.channel = LEDC_CHANNEL_0;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.timer_sel = LEDC_TIMER_0;
    ledc_conf.duty = 0x0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
                          // 50%=0x01FF, 100%=0x03FF for 10 Bit
    ledc_channel_config(&ledc_conf);

    // start
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0x7F); // 12% duty - play here for your speaker or buzzer
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    // stop
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0);
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
}

void play_on(void)
{
    sound(GPIO_BUZZER, 4000, 200);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    // sound(GPIO_BUZZER, 4000, 200);
    // vTaskDelay(300 / portTICK_PERIOD_MS);
    BuzzerState = 0;
}

void play_off(void)
{
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    BuzzerState = 0;
}

void play_bat_1(void)
{
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 4000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    BuzzerState = 0;
}

void play_bat_2(void)
{
    sound(GPIO_BUZZER, 2000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 3000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 3000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 3000, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    BuzzerState = 0;
}

void play_alarm1(void) // Power ON
{
    sound(GPIO_BUZZER, 2000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    // sound(GPIO_BUZZER, 2000, 100);
    // vTaskDelay(100 / portTICK_PERIOD_MS);
    // sound(GPIO_BUZZER, 2000, 100);
    // vTaskDelay(100 / portTICK_PERIOD_MS);
}

void play_alarm2(void) // Power Off
{
    sound(GPIO_BUZZER, 2000, 200);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 200);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 200);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 2000, 200);
    vTaskDelay(200 / portTICK_PERIOD_MS);
}

// based on https://wiki.mikrotik.com/wiki/Super_Mario_Theme
void play_theme(void)
{
    sound(GPIO_BUZZER, 660, 100);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 660, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 660, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 510, 100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 660, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 770, 100);
    vTaskDelay(550 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 380, 100);
    vTaskDelay(575 / portTICK_PERIOD_MS);

    sound(GPIO_BUZZER, 510, 100);
    vTaskDelay(450 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 380, 100);
    vTaskDelay(400 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 320, 100);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 440, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 480, 80);
    vTaskDelay(330 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 450, 100);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 430, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 380, 100);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 660, 80);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 760, 50);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 860, 100);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 700, 80);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 760, 50);
    vTaskDelay(350 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 660, 80);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 520, 80);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 580, 80);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    sound(GPIO_BUZZER, 480, 80);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // BuzzerState = 0;
}
// based on http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March#Code
// original composed by John Williams for the film Star Wars: The Empire Strikes Back
void play_march(uint8_t longplay)
{

    sound(GPIO_BUZZER, a, 500);
    sound(GPIO_BUZZER, a, 500);
    sound(GPIO_BUZZER, a, 500);
    sound(GPIO_BUZZER, f, 350);
    sound(GPIO_BUZZER, cH, 150);
    sound(GPIO_BUZZER, a, 500);
    sound(GPIO_BUZZER, f, 350);
    sound(GPIO_BUZZER, cH, 150);
    sound(GPIO_BUZZER, a, 650);

    vTaskDelay(150 / portTICK_PERIOD_MS);
    // end of first bit

    sound(GPIO_BUZZER, eH, 500);
    sound(GPIO_BUZZER, eH, 500);
    sound(GPIO_BUZZER, eH, 500);
    sound(GPIO_BUZZER, fH, 350);
    sound(GPIO_BUZZER, cH, 150);
    sound(GPIO_BUZZER, gS, 500);
    sound(GPIO_BUZZER, f, 350);
    sound(GPIO_BUZZER, cH, 150);
    sound(GPIO_BUZZER, a, 650);

    vTaskDelay(150 / portTICK_PERIOD_MS);
    // end of second bit...

    sound(GPIO_BUZZER, aH, 500);
    sound(GPIO_BUZZER, a, 300);
    sound(GPIO_BUZZER, a, 150);
    sound(GPIO_BUZZER, aH, 400);
    sound(GPIO_BUZZER, gSH, 200);
    sound(GPIO_BUZZER, gH, 200);
    sound(GPIO_BUZZER, fSH, 125);
    sound(GPIO_BUZZER, fH, 125);
    sound(GPIO_BUZZER, fSH, 250);

    vTaskDelay(250 / portTICK_PERIOD_MS);

    sound(GPIO_BUZZER, aS, 250);
    sound(GPIO_BUZZER, dSH, 400);
    sound(GPIO_BUZZER, dH, 200);
    sound(GPIO_BUZZER, cSH, 200);
    sound(GPIO_BUZZER, cH, 125);
    sound(GPIO_BUZZER, b, 125);
    sound(GPIO_BUZZER, cH, 250);

    vTaskDelay(250 / portTICK_PERIOD_MS);

    sound(GPIO_BUZZER, f, 125);
    sound(GPIO_BUZZER, gS, 500);
    sound(GPIO_BUZZER, f, 375);
    sound(GPIO_BUZZER, a, 125);
    sound(GPIO_BUZZER, cH, 500);
    sound(GPIO_BUZZER, a, 375);
    sound(GPIO_BUZZER, cH, 125);
    sound(GPIO_BUZZER, eH, 650);

    // end of third bit... (Though it doesn't play well)
    // let's repeat it
    if (longplay >= 1)
    {
        sound(GPIO_BUZZER, aH, 500);
        sound(GPIO_BUZZER, a, 300);
        sound(GPIO_BUZZER, a, 150);
        sound(GPIO_BUZZER, aH, 400);
        sound(GPIO_BUZZER, gSH, 200);
        sound(GPIO_BUZZER, gH, 200);
        sound(GPIO_BUZZER, fSH, 125);
        sound(GPIO_BUZZER, fH, 125);
        sound(GPIO_BUZZER, fSH, 250);

        vTaskDelay(250 / portTICK_PERIOD_MS);

        sound(GPIO_BUZZER, aS, 250);
        sound(GPIO_BUZZER, dSH, 400);
        sound(GPIO_BUZZER, dH, 200);
        sound(GPIO_BUZZER, cSH, 200);
        sound(GPIO_BUZZER, cH, 125);
        sound(GPIO_BUZZER, b, 125);
        sound(GPIO_BUZZER, cH, 250);

        vTaskDelay(250 / portTICK_PERIOD_MS);

        sound(GPIO_BUZZER, f, 250);
        sound(GPIO_BUZZER, gS, 500);
        sound(GPIO_BUZZER, f, 375);
        sound(GPIO_BUZZER, cH, 125);
        sound(GPIO_BUZZER, a, 500);
        sound(GPIO_BUZZER, f, 375);
        sound(GPIO_BUZZER, cH, 125);
        sound(GPIO_BUZZER, a, 650);
        // end of the song
    }
    // BuzzerState = 0;
}