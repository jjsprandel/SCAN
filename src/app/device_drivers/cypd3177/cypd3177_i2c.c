#include "cypd3177.h"
#include "esp_err.h"

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

//static const uint8_t data_mem_addr[] = FORMAT(WRITE_DATA_MEM_REG_ADDR);




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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Block until ISR signals
    
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
    if (pd_response_reg.response_code == 0x84)
    {
        usb_connected = 1;
    }

    else if (pd_response_reg.response_code == 0x85)
    {
        usb_connected = 0;
    }
    ESP_LOGI(TAG, "pd response code: 0x%02X", pd_response_reg.response_code);
    ESP_LOGI(TAG, "pd response type: %d", pd_response_reg.response_type);
    //ESP_LOGI(TAG, "pd length1: %d", pd_response_reg.length1);
    //ESP_LOGI(TAG, "pd length2: %d", pd_response_reg.length2);
    return;
}


esp_err_t cypd3177_change_pdo(void)
{
    // Define high power PDOs
    const uint32_t high_power_pdos[] = {
        0x5A900102,  // Fixed PDO: 5V, 0.9A (5V = 100 * 50mV, 0.9A = 90 * 10mA)
        0x5A900302,  // Fixed PDO: 5V, 3.0A (5V = 100 * 50mV, 3.0A = 300 * 10mA)
        0xB4900302,  // Fixed PDO: 9V, 3.0A (9V = 180 * 50mV, 3.0A = 300 * 10mA)
        0xF0900302,  // Fixed PDO: 12V, 3.0A (12V = 240 * 50mV, 3.0A = 300 * 10mA)
        0x2C900302,  // Fixed PDO: 15V, 3.0A (15V = 300 * 50mV, 3.0A = 300 * 10mA)
        0x90900302,  // Fixed PDO: 20V, 3.0A (20V = 400 * 50mV, 3.0A = 300 * 10mA)
        0x90900502   // Fixed PDO: 20V, 5.0A (20V = 400 * 50mV, 5.0A = 500 * 10mA)
    };

    // Step 1: Write the PDO data to the data memory address (0x1800-0x19FF)
    
    // Prepare the complete 32-byte data buffer
    uint8_t pdo_data[32];
    
    // Add "SNKP" ASCII string (little endian)
    pdo_data[0] = 0x50; // 'P'
    pdo_data[1] = 0x4B; // 'K'
    pdo_data[2] = 0x4E; // 'N'
    pdo_data[3] = 0x53; // 'S'

    
    // Write the complete 32-byte data to the data memory
    /*esp_err_t ret = i2c_master_transmit(cypd3177_i2c_handle, data_memory_addr, 2, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set data memory address");
        return ret;
    }
        
    
    ret = i2c_master_transmit(cypd3177_i2c_handle, pdo_data, 32, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write PDO data");
        return ret;
    }
    */

    // Step 2: Send 1 byte with data 0xFF to register address 0x1005
    uint8_t cmd_addr[] = {0x05, 0x10}; // Register address 0x1005
    uint8_t cmd_data[] = {0xFF};       // Data to write
    
    /*
    ret = i2c_master_transmit(cypd3177_i2c_handle, cmd_addr, 2, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set command address");
        return ret;
    }
    
    ret = i2c_master_transmit(cypd3177_i2c_handle, cmd_data, 1, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send command data");
        return ret;
    }
    */
    return ESP_OK;
}


esp_err_t enable_high_power_charging(void)
{
    // Change PDOs to enable high power charging
    return cypd3177_change_pdo();
}