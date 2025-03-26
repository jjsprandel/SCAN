#include "cypd3177.h"

static const char *TAG = "CYPD3177";

i2c_master_dev_handle_t cypd3177_i2c_handle;

i2c_device_config_t cypd3177_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = CYPD3177_I2C_ADDR,
    .scl_speed_hz = 100000,
    .scl_wait_us = 2000,
};

static device_mode_reg_t device_mode_reg;
static silicon_id_reg_t silicon_id_reg;
static interrupt_type_reg_t interrupt_type_reg;

static bus_voltage_reg_t bus_voltage_reg;

static dev_response_reg_t dev_response_reg;
static pd_response_reg_t pd_response_reg;



//static const uint8_t device_mode_addr[] = FORMAT(DEVICE_MODE_REG);
//static const uint8_t silicon_id_addr[] = FORMAT(SILICON_ID_REG);
static const uint8_t interrupt_type_addr[] = FORMAT(INTERRUPT_TYPE_REG_ADDR);

static const uint8_t bus_voltage_addr[] = FORMAT(BUS_VOLTAGE_REG_ADDR);

static const uint8_t dev_response_addr[] = FORMAT(DEV_RESPONSE_REG_ADDR);
static const uint8_t pd_response_addr[] = FORMAT(PD_RESPONSE_REG_ADDR);




void power_check(void *pvParameter)
{
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    while (1) 
    {
    i2c_master_transmit_receive(cypd3177_i2c_handle, bus_voltage_addr, 2, (uint8_t *)&bus_voltage_reg, sizeof(bus_voltage_reg), -1);

    ESP_LOGI(TAG, "Bus voltage: %d mV", bus_voltage_reg.voltage * 100);

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void get_interrupt_response_code(void *pvParameter)
{
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    uint8_t reset_command[4] = {0x08, 0x00, 0x52, 0x00 };
    uint8_t clear_dev_intr[3] = {0x06, 0x00, 0x03 };
    uint8_t event_mask[6] = {0x24, 0x10, 0x18, 0x00, 0x00, 0x00 };
    i2c_master_transmit(cypd3177_i2c_handle, event_mask, 6, -1);
    
    while (1) 
    {
    // get interrupt type
    i2c_master_transmit_receive(cypd3177_i2c_handle, interrupt_type_addr, 2, (uint8_t *)&interrupt_type_reg, sizeof(interrupt_type_reg), -1);

    ESP_LOGI(TAG, "device interrupt value: %d", interrupt_type_reg.device_interrupt);
    ESP_LOGI(TAG, "PD port interrupt value: %d", interrupt_type_reg.pd_port_interrupt);

    if (interrupt_type_reg.device_interrupt)
    {
        //vTaskDelay(10000 / portTICK_PERIOD_MS);
        get_dev_response(NULL);
        //i2c_master_transmit(cypd3177_i2c_handle, event_mask, 6, -1);
        
        //i2c_master_transmit(cypd3177_i2c_handle, reset_command, 4, -1);
    }

    if (interrupt_type_reg.pd_port_interrupt)
    {
        get_pd_response(NULL);
    }
    
    i2c_master_transmit(cypd3177_i2c_handle, clear_dev_intr, 3, -1);


    vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

}

void get_dev_response(void *pvParameter)
{
    i2c_master_transmit_receive(cypd3177_i2c_handle, dev_response_addr, 2, (uint8_t *)&dev_response_reg, 2, -1);
    ESP_LOGI(TAG, "dev response code: 0x%02X", dev_response_reg.response_code);
    ESP_LOGI(TAG, "dev response type: %d", dev_response_reg.response_type);
    ESP_LOGI(TAG, "dev response length: %d", dev_response_reg.length);
    return;
}

void get_pd_response(void *pvParameter)
{
    i2c_master_transmit_receive(cypd3177_i2c_handle, pd_response_addr, 2, (uint8_t *)&pd_response_reg, 4, -1);
    ESP_LOGI(TAG, "pd response code: 0x%02X", pd_response_reg.response_code);
    ESP_LOGI(TAG, "pd response type: %d", pd_response_reg.response_type);
    //ESP_LOGI(TAG, "pd length1: %d", pd_response_reg.length1);
    //ESP_LOGI(TAG, "pd length2: %d", pd_response_reg.length2);
    return;
}