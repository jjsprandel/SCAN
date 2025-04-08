#ifndef STATUS_BUZZER_H
#define STATUS_BUZZER_H
#include "buzzer.h"
#include "state_enum.h"
#include "esp_log.h"

#define BUZZER_GPIO GPIO_NUM_15
#define BUZZER_DEBUG 1

#define VOLUME_LOW 0.05f
#define VOLUME_MEDIUM 0.1f
#define VOLUME_HIGH 0.2f
#define VOLUME_MAX 1.0f
#define VOLUME_OFF 0.0f

void buzzer_init();
void play_kiosk_buzzer(state_t current_state, admin_state_t current_admin_state);

#endif // STATUS_BUZZER_Hs