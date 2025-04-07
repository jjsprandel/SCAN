#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "driver/i2c_master.h"


#define I2C_MASTER_NUM              I2C_NUM_0

#define I2C_SDA_PIN                 GPIO_NUM_4
#define I2C_SCL_PIN                 GPIO_NUM_5
#define I2C_CLK_SRC                 I2C_CLK_SRC_DEFAULT
#define I2C_GLITCH_IGNORE_CNT       7
#define I2C_MASTER_FREQ_HZ          100000 // standard mode; limited by max SCL speed for PCF8574

extern i2c_master_bus_handle_t master_handle;

// Function prototypes
void i2c_master_init(i2c_master_bus_handle_t *bus_handle);
void i2c_master_add_device(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle, const i2c_device_config_t *dev_config);

#endif