#include "keypad_driver.h"

i2c_master_dev_handle_t pcf8574n_i2c_handle;

i2c_device_config_t pcf8574n_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = PCF8574N_I2C_ADDR,
    .scl_speed_hz = 100000,
};


keypad_buffer_t keypad_buffer;
static const char *KEYPAD_TAG = "keypad_driver";
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

char poll_keypad(uint8_t keypad_address)
{
    uint8_t data = 0x00;
    uint8_t activate = 0xf0;
    uint8_t lines = 0;
    uint8_t cols = 0;

    // Detect active line
    ESP_ERROR_CHECK(i2c_master_transmit(pcf8574n_i2c_handle, &activate, 1, 50));
    ESP_ERROR_CHECK(i2c_master_receive(pcf8574n_i2c_handle, &data, 1, 50));
    switch ((data ^ 0xff) >> 4)
    {
    case 8:
        lines = 1;
        break;
    case 4:
        lines = 2;
        break;
    case 2:
        lines = 3;
        break;
    case 1:
        lines = 4;
        break;
    }

    // Detect active column
    activate = 0x0f;
    ESP_ERROR_CHECK(i2c_master_transmit(pcf8574n_i2c_handle, &activate, 1, 50));
    ESP_ERROR_CHECK(i2c_master_receive(pcf8574n_i2c_handle, &data, 1, 50));
    switch ((data ^ 0xff) & 0x0f)
    {
    case 8:
        cols = 1;
        break;
    case 4:
        cols = 2;
        break;
    case 2:
        cols = 3;
        break;
    case 1:
        cols = 4;
        break;
    }

    // Return detected key
    if (lines && cols)
    {
        vTaskDelay(DEBOUNCE_PERIOD_MS / portTICK_PERIOD_MS);
        return (keypad_array[lines - 1][cols - 1]);
    }
    return '\0';
}

void keypad_handler(void *params)
{
    char c = '\0';
    uint8_t clear_pullup = 0xff;
    double prev_time = 0;
    double curr_time = 0;
    ESP_ERROR_CHECK(i2c_master_transmit(pcf8574n_i2c_handle, &clear_pullup, 1, 50));
    init_timer();

    while (1)
    {
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &curr_time);

        c = poll_keypad(KEYPAD_ADDRESS);

        if ((prev_time - curr_time) > 10)
            clear_buffer();

        switch (c)
        {
        case '*':
            keypad_buffer.occupied -= 1;
            (keypad_buffer.elements)[keypad_buffer.occupied] = '\0';

#ifdef KEYPAD_DEBUG
            ESP_LOGI(KEYPAD_TAG, "Backspace pressed");
#endif
            prev_time = curr_time;
            break;
        case '#':
            if (keypad_buffer.occupied == ID_LEN)
            {
#ifdef KEYPAD_DEBUG
                ESP_LOGI(KEYPAD_TAG, "ID of valid length is entered");
#endif
                if (state_control_task_handle != NULL)
                {
                    xTaskNotifyGive(state_control_task_handle);
#ifdef KEYPAD_DEBUG
                    ESP_LOGI(KEYPAD_TAG, "Notification sent to state control task");
#endif
                }
                else
                {
                    ESP_LOGW(KEYPAD_TAG, "Cannot notify - state_control_task_handle is NULL");
                }
            }
            else
            {
#ifdef KEYPAD_DEBUG
                ESP_LOGI(KEYPAD_TAG, "ID %s invalid, please entered an ID of length %d", keypad_buffer.elements, ID_LEN);
#endif
            }
#ifdef KEYPAD_DEBUG
            ESP_LOGI(KEYPAD_TAG, "[Buffer]> %s", keypad_buffer.elements);
#endif
            prev_time = curr_time;
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
