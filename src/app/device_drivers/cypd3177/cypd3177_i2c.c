#include "cypd3177_i2c.h"

i2c_master_dev_handle_t cypd3177_i2c_handle;

i2c_device_config_t cypd3177_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = CYPD3177_I2C_ADDR,
    .scl_speed_hz = I2C_MASTER_FREQ_HZ
};