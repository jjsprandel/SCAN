/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "keypad_driver.h"
#define KEYPAD_DEBUG

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
    i2c_master_write_to_device(I2C_NUM_0, keypad_address, &activate, 1, 100);
    i2c_master_read_from_device(I2C_NUM_0, keypad_address, &data, 1, 100);
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
    i2c_master_write_to_device(I2C_NUM_0, keypad_address, &activate, 1, 100);
    i2c_master_read_from_device(I2C_NUM_0, keypad_address, &data, 1, 100);
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
    // keypad_buffer_t *keypad_buffer = (keypad_buffer_t *)params;

    char c = '\0';
    uint8_t clear_pullup = 0xff;
    double prev_time = 0;
    double curr_time = 0;
    i2c_master_write_to_device(I2C_NUM_0, KEYPAD_ADDRESS, &clear_pullup, 1, 100);
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
            ESP_LOGI(KEYPAD_TAG, "Buffer Cleared");
#endif
            prev_time = curr_time;
            break;
        case '#':
            keypadEntered = true;
            xEventGroupSetBits(event_group, ID_ENTERED_KEYPAD_BIT);
            vTaskDelete(NULL);
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

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_1,
        .scl_io_num = GPIO_NUM_0,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(I2C_NUM_0, &conf);

    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}