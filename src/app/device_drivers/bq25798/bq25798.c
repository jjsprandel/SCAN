#include "bq25798.h"

static const char *TAG = "BQ25798";

// I2C device handle and configuration
i2c_master_dev_handle_t bq25798_i2c_handle;

i2c_device_config_t bq25798_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = BQ25798_I2C_ADDR,
    .scl_speed_hz = 100000,
    .scl_wait_us = 2000,
};

// Register read function
esp_err_t bq25798_read_reg(i2c_port_t port, uint8_t reg_addr, uint8_t *data)
{
    uint8_t write_buf[1] = {reg_addr};
    return i2c_master_transmit_receive(bq25798_i2c_handle, write_buf, 1, data, 1, -1);
}

// Register write function
esp_err_t bq25798_write_reg(i2c_port_t port, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(bq25798_i2c_handle, write_buf, 2, -1);
}

// Initialize the BQ25798 charger
esp_err_t bq25798_init(void)
{
    ESP_LOGI(TAG, "Initializing BQ25798 charger");
    
    // Read and verify device ID from PART_INFO register
    uint8_t part_info;
    esp_err_t ret = bq25798_read_reg(I2C_NUM_0, BQ25798_PART_INFO, &part_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read part info");
        return ret;
    }
    
    ESP_LOGI(TAG, "Part Info: 0x%02X", part_info);
    
    // Configure charge current and voltage limits
    ret = bq25798_set_charge_current(I2C_NUM_0, BQ25798_DEFAULT_CHARGE_CURRENT_MA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set charge current");
        return ret;
    }

    
    // Enable charging
    uint8_t charge_ctrl = BQ25798_CHRG_EN | BQ25798_ADC_EN;
    ret = bq25798_write_reg(I2C_NUM_0, BQ25798_CHRG_CTRL_0, charge_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable charging");
        return ret;
    }
    
    ESP_LOGI(TAG, "BQ25798 initialization complete");
    return ESP_OK;
}

// Set charge current
esp_err_t bq25798_set_charge_current(i2c_port_t port, uint16_t current_ma)
{
    uint8_t reg_value = (current_ma * 1000) / BQ25798_ICHRG_CURRENT_STEP_uA;
    return bq25798_write_reg(port, BQ25798_CHRG_I_LIM_MSB, reg_value);
}

// Set maximum charge voltage
esp_err_t bq25798_set_max_charge_voltage(i2c_port_t port, uint16_t voltage_mv)
{
    uint8_t reg_value = (voltage_mv * 1000) / BQ25798_VREG_V_STEP_uV;
    return bq25798_write_reg(port, BQ25798_CHRG_V_LIM_MSB, reg_value);
}

// Get charge status
esp_err_t bq25798_get_charge_status(i2c_port_t port, uint8_t *status)
{
    return bq25798_read_reg(port, BQ25798_CHRG_STAT_1, status);
}

// Get battery voltage
esp_err_t bq25798_get_battery_voltage(i2c_port_t port, uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_VBAT_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(port, BQ25798_ADC_VBAT_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Get charge current
esp_err_t bq25798_get_charge_current(i2c_port_t port, uint16_t *current_ma)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_IBAT_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(port, BQ25798_ADC_IBAT_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *current_ma = (adc_value * BQ25798_ADC_CURR_STEP_uA) / 1000;
    return ESP_OK;
}

// Get input current
esp_err_t bq25798_get_input_current(i2c_port_t port, uint16_t *current_ma)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_IBUS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(port, BQ25798_ADC_IBUS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *current_ma = (adc_value * BQ25798_ADC_CURR_STEP_uA) / 1000;
    return ESP_OK;
}

// Get input voltage
esp_err_t bq25798_get_input_voltage(i2c_port_t port, uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_VBUS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(port, BQ25798_ADC_VBUS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Get system voltage
esp_err_t bq25798_get_system_voltage(i2c_port_t port, uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_VSYS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(port, BQ25798_ADC_VSYS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Get temperature
esp_err_t bq25798_get_temperature(i2c_port_t port, int16_t *temperature_c)
{
    uint8_t reg_value;
    esp_err_t ret = bq25798_read_reg(port, BQ25798_ADC_TDIE, &reg_value);
    if (ret != ESP_OK) {
        return ret;
    }
    *temperature_c = reg_value;
    return ESP_OK;
}

// Monitor charging status task
void bq25798_monitor_task(void *pvParameters)
{
    uint16_t battery_voltage;
    uint16_t charge_current;
    uint16_t input_voltage;
    uint16_t input_current;
    int16_t temperature;
    uint8_t charge_status;
    
    while (1) {
        // Read all parameters
        if (bq25798_get_battery_voltage(I2C_NUM_0, &battery_voltage) == ESP_OK) {
            ESP_LOGI(TAG, "Battery Voltage: %d mV", battery_voltage);
        }
        
        if (bq25798_get_charge_current(I2C_NUM_0, &charge_current) == ESP_OK) {
            ESP_LOGI(TAG, "Charge Current: %d mA", charge_current);
        }
        
        if (bq25798_get_input_voltage(I2C_NUM_0, &input_voltage) == ESP_OK) {
            ESP_LOGI(TAG, "Input Voltage: %d mV", input_voltage);
        }
        
        if (bq25798_get_input_current(I2C_NUM_0, &input_current) == ESP_OK) {
            ESP_LOGI(TAG, "Input Current: %d mA", input_current);
        }
        
        if (bq25798_get_temperature(I2C_NUM_0, &temperature) == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %d°C", temperature);
        }
        
        if (bq25798_get_charge_status(I2C_NUM_0, &charge_status) == ESP_OK) {
            ESP_LOGI(TAG, "Charge Status: 0x%02X", charge_status);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // Update every 10 seconds
    }
} 