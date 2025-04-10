#include "i2c_config.h"

static const char *TAG = "i2c_master";

// variable definitions
i2c_master_bus_handle_t master_handle;


void i2c_master_init(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC,
        .glitch_ignore_cnt = I2C_GLITCH_IGNORE_CNT,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void i2c_master_add_device(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle, const i2c_device_config_t *dev_config)
{
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, dev_config, dev_handle));
}