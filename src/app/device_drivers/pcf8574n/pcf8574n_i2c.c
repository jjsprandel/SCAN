#include "pcf8574n.h"

static const char *TAG = "PCF8574N";

i2c_master_dev_handle_t pcf8574n_i2c_handle;

i2c_device_config_t pcf8574n_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = PCF8574N_I2C_ADDR,
    .scl_speed_hz = 100000,
};

void set_pcf_pins(uint8_t pin_config)
{
    i2c_master_transmit(pcf8574n_i2c_handle, &pin_config, 1, 50);
    return;
}

void read_pcf_pins(uint8_t *pin_states)
{
    i2c_master_receive(pcf8574n_i2c_handle, pin_states, 1, 50);
    return;
}