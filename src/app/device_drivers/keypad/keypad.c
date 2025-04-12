#include "keypad.h"

static const char *TAG = "KEYPAD";


keypad_buffer_t keypad_buffer;
bool keypadEntered = false;

char keypad_array[4][4] = {
    "123A",
    "456B",
    "789C",
    "*0#D"};

void clear_buffer()
{
    keypad_buffer.occupied = 0;
    for (int i = 0; i < MAX_BUFFER_SIZE; i++)
        (keypad_buffer.elements)[i] = '\0';
}

void add_to_buffer(char val)
{
    if ((keypad_buffer.occupied) >= (MAX_BUFFER_SIZE - 2))
        clear_buffer();
    (keypad_buffer.elements)[keypad_buffer.occupied] = val;
    keypad_buffer.occupied += 1;
}

void init_timer()
{
    timer_config_t timer_conf =
        {
            .divider = TIMER_DEVIDER,
            .counter_en = true,
            .alarm_en = false,
            .auto_reload = false,
            .clk_src = TIMER_SRC_CLK_XTAL,
        };
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    timer_start(TIMER_GROUP_0, TIMER_0);
}

uint8_t detect_active_row()
{
    uint8_t data = 0x00;
    uint8_t pin_config_row = 0xF0;
    uint8_t row = 0;
    
    set_pcf_pins(pin_config_row);
    read_pcf_pins(&data);
    switch ((data ^ 0xff) >> 4)
    {
    case 8:
        row = 4;
        break;
    case 4:
        row = 3;
        break;
    case 2:
        row = 2;
        break;
    case 1:
        row = 1;
        break;
    }
    return row;
}

uint8_t detect_active_column()
{
    uint8_t data = 0x00;
    uint8_t pin_config_col = 0x0F;
    uint8_t col = 0;
    set_pcf_pins(pin_config_col);
    read_pcf_pins(&data);
    switch ((data ^ 0xff) & 0x0f)
    {
    case 8:
        col = 4;
        break;
    case 4:
        col = 3;
        break;
    case 2:
        col = 2;
        break;
    case 1:
        col = 1;
        break;
    }
    return col;
}

char get_active_key()
{
    uint8_t row = 0;
    uint8_t col = 0;

    row = detect_active_row();
    col = detect_active_column();

    // Return active key
    if (row && col)
    {
        vTaskDelay(DEBOUNCE_PERIOD_MS / portTICK_PERIOD_MS);
        return (keypad_array[row - 1][col - 1]);
    }
    return '\0';   
}

void keypad_handler(void *params)
{
    char c = '\0';
    uint8_t clear_pullup = 0xff;
    double prev_time = 0;
    double curr_time = 0;
    i2c_master_transmit(pcf8574n_i2c_handle, &clear_pullup, 1, 50);
    // init_timer();

    while (1)
    {
        // timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &curr_time);

        c = get_active_key();

        // if ((prev_time - curr_time) > 10)
        //     clear_buffer();

        switch (c)
        {
        case '*':
            keypad_buffer.occupied -= 1;
            (keypad_buffer.elements)[keypad_buffer.occupied] = '\0';

#ifdef KEYPAD_DEBUG
            ESP_LOGI(TAG, "Backspace pressed");
#endif
            prev_time = curr_time;
            break;
        case '#':
        {                
             BaseType_t adminNotify = ulTaskNotifyTake(pdTRUE, 0);
                if (adminNotify > 0)
                {
                    if (admin_mode_control_task_handle != NULL)
                    {
#ifdef KEYPAD_DEBUG
                        ESP_LOGI(TAG, "Notification sent to admin mode control task");
#endif
                        xTaskNotifyGive(admin_mode_control_task_handle);
                    }
                }
                else if (state_control_task_handle != NULL)
                {
                    xTaskNotifyGive(state_control_task_handle);
#ifdef KEYPAD_DEBUG
                    ESP_LOGI(TAG, "Notification sent to state control task");
#endif
                }
                else
                {
                    ESP_LOGW(TAG, "Cannot notify - state_control_task_handle is NULL");
                }
#ifdef KEYPAD_DEBUG
            ESP_LOGI(TAG, "[Buffer]> %s", keypad_buffer.elements);
#endif
            prev_time = curr_time;
            break;
            }
        case 'A':
            esp_restart();
            ESP_LOGI(TAG, "Reset");
            break;
        case '\0':
            break;
        default:
            prev_time = curr_time;
            putchar(c);
            putchar('\n');
            add_to_buffer(c);
            break;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
