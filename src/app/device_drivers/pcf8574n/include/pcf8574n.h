#include <driver/i2c_master.h>
#include <esp_log.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <stdbool.h>

#define PCF8574N_I2C_ADDR   0x20

extern i2c_master_dev_handle_t pcf8574n_i2c_handle;
extern i2c_device_config_t pcf8574n_i2c_config;

void set_pcf_pins(uint8_t pin_config);

void read_pcf_pins(uint8_t *pin_states);