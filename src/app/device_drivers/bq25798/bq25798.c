#include "bq25798.h"
#include "../../main/include/global.h"

static const char *TAG = "BQ25798";

// Define the voltage values as const (not static) to match the extern declarations
const uint16_t vsysmin_mv = 10500;  // Minimum system voltage in mV (10.5V = 10500mV)
const uint16_t vreg_mv = 14000;     // Maximum charge voltage in mV (14.0V = 14000mV)

// Move these string constants to flash memory
static const char *charge_state_strings[] = {
    "Not Charging",
    "Trickle Charge",
    "Pre-Charge",
    "Fast-Charge (CC Mode)",
    "Taper Charge (CV Mode)",
    "Reserved",
    "Top-Off Timer Active",
    "Charge Termination Done",
    "Unknown"
};

static const char *vbus_status_strings[] = {
    "No Input or BHOT/BCOLD in OTG mode",
    "USB SDP (500mA)",
    "USB CDP (1.5A)",
    "USB DCP (3.25A)",
    "HVDCP (1.5A)",
    "Unknown Adapter (3A)",
    "Non-Standard Adapter (1A/2A/2.1A/2.4A)",
    "OTG Mode",
    "Not Qualified Adapter",
    "Reserved",
    "Reserved",
    "Device Directly Powered from VBUS",
    "Backup Mode",
    "Reserved",
    "Reserved",
    "Reserved",
    "Unknown"
};

// Mutex for protecting access to device_info
SemaphoreHandle_t device_info_mutex;

// I2C device handle
i2c_master_dev_handle_t bq25798_i2c_handle;

// I2C device configuration
i2c_device_config_t bq25798_i2c_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = BQ25798_I2C_ADDR,
    .scl_speed_hz = 100000,
    .scl_wait_us = 2000,
};


// Register read function
esp_err_t bq25798_read_reg(uint8_t reg_addr, uint8_t *data)
{
    uint8_t write_buf[1] = {reg_addr};
    return i2c_master_transmit_receive(bq25798_i2c_handle, write_buf, 1, data, 1, -1);
}

// Register write function
esp_err_t bq25798_write_reg(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(bq25798_i2c_handle, write_buf, 2, -1);
}

// Mutex for protecting access to device_info
SemaphoreHandle_t device_info_mutex = NULL;

// Initialize the BQ25798 charger
esp_err_t bq25798_init(void)
{
    // Create the mutex for device_info access
    device_info_mutex = xSemaphoreCreateMutex();
    if (device_info_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create device_info mutex");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Initializing BQ25798 charger");
    
    // Read and verify device ID from PART_INFO register
    uint8_t part_info;
    esp_err_t ret = bq25798_read_reg(BQ25798_PART_INFO, &part_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read part info");
        return ret;
    }
    
    ESP_LOGI(TAG, "Part Info: 0x%02X", part_info);
    
    // 1. Disable the watchdog timer (REG10)
    ESP_LOGI(TAG, "Disabling watchdog timer");
    uint8_t reg_value;
    ret = bq25798_read_reg(BQ25798_CHRG_CTRL_1, &reg_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Charger Control 1 register");
        return ret;
    }
    
    ESP_LOGI(TAG, "Current Charger Control 1 value: 0x%02X", reg_value);
    
    // Clear bits 2-0 (WATCHDOG_2:0) to disable the watchdog timer
    reg_value &= ~(0x07); // Clear bits 2-0
    
    ret = bq25798_write_reg(BQ25798_CHRG_CTRL_1, reg_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable watchdog timer");
        return ret;
    }
    
    // 2. Set VSYSMIN to 10.5V (REG00)
    ESP_LOGI(TAG, "Setting VSYSMIN to 10.5V");
    // 10.5V = 10500mV, convert to register value using offset and step size
    uint8_t vsysmin_reg = (vsysmin_mv - BQ25798_VSYSMIN_OFFSET_mV) / BQ25798_VSYSMIN_STEP_mV;
    ret = bq25798_write_reg(BQ25798_MIN_SYS_V, vsysmin_reg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set VSYSMIN");
        return ret;
    }
    
    // 3. Set VREG to 14.0V (REG01-02)
    ESP_LOGI(TAG, "Setting VREG to 14V");
    // 14.0V = 14000mV, convert to register value using step size of 3.84mV
    // Only bits 10-0 are used for VREG, bits 15-11 are reserved
    uint16_t vreg_reg = vreg_mv / BQ25798_VREG_STEP_mV;
    // Ensure we only use bits 10-0 (0x7FF mask)
    vreg_reg &= 0x7FF;
    
    // Split the 11-bit value into MSB and LSB
    uint8_t vreg_msb = (vreg_reg >> 8) & 0xFF;  // Upper 3 bits
    uint8_t vreg_lsb = vreg_reg & 0xFF;         // Lower 8 bits
    
    ESP_LOGI(TAG, "VREG register value: 0x%03X, MSB: 0x%02X, LSB: 0x%02X", 
             vreg_reg, vreg_msb, vreg_lsb);
    
    // Write to both MSB and LSB registers
    ret = bq25798_write_reg(BQ25798_CHRG_V_LIM_MSB, vreg_msb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set VREG MSB");
        return ret;
    }
    
    ret = bq25798_write_reg(BQ25798_CHRG_V_LIM_LSB, vreg_lsb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set VREG LSB");
        return ret;
    }
    
    // 4. Set ICHG to 5A (REG03-04)
    ESP_LOGI(TAG, "Setting ICHG to 5A");
    // 3A = 3000mA, convert to register value using step size of 10mA
    // Only bits 8-0 are used for ICHG, bits 15-9 are reserved
    uint16_t ichg_ma = 5000;
    uint16_t ichg_reg = ichg_ma / BQ25798_ICHRG_STEP_mA;
    // Ensure we only use bits 8-0 (0x1FF mask)
    ichg_reg &= 0x1FF;
    
    // Split the 9-bit value into MSB and LSB
    uint8_t ichg_msb = (ichg_reg >> 8) & 0xFF;  // Upper 1 bit
    uint8_t ichg_lsb = ichg_reg & 0xFF;         // Lower 8 bits
    
    ESP_LOGI(TAG, "ICHG register value: 0x%03X, MSB: 0x%02X, LSB: 0x%02X", 
             ichg_reg, ichg_msb, ichg_lsb);
    
    // Write to both MSB and LSB registers
    ret = bq25798_write_reg(BQ25798_CHRG_I_LIM_MSB, ichg_msb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ICHG MSB");
        return ret;
    }
    
    ret = bq25798_write_reg(BQ25798_CHRG_I_LIM_LSB, ichg_lsb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ICHG LSB");
        return ret;
    }
    
    // 5. Set IINDPM to 3.3A (REG06-07)
    ESP_LOGI(TAG, "Setting IINDPM to 3.3A");
    // 2A = 2000mA, convert to register value using step size of 10mA
    // Only bits 8-0 are used for IINDPM, bits 15-9 are reserved
    uint16_t iindpm_ma = 3300;
    uint16_t iindpm_reg = iindpm_ma / BQ25798_IINDPM_STEP_mA;
    // Ensure we only use bits 8-0 (0x1FF mask)
    iindpm_reg &= 0x1FF;
    
    // Split the 9-bit value into MSB and LSB
    uint8_t iindpm_msb = (iindpm_reg >> 8) & 0xFF;  // Upper 1 bit
    uint8_t iindpm_lsb = iindpm_reg & 0xFF;         // Lower 8 bits
    
    ESP_LOGI(TAG, "IINDPM register value: 0x%03X, MSB: 0x%02X, LSB: 0x%02X", 
             iindpm_reg, iindpm_msb, iindpm_lsb);
    
    // Write to both MSB and LSB registers
    ret = bq25798_write_reg(BQ25798_INPUT_I_LIM_MSB, iindpm_msb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set IINDPM MSB");
        return ret;
    }
    
    ret = bq25798_write_reg(BQ25798_INPUT_I_LIM_LSB, iindpm_lsb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set IINDPM LSB");
        return ret;
    }
    
    // Enable ADC
    uint8_t charge_ctrl;
    ret = bq25798_read_reg(BQ25798_CHRG_CTRL_0, &charge_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read charge control register");
        return ret;
    }
    
    ESP_LOGI(TAG, "Charge Control Register (before): 0x%02X", charge_ctrl);
    
    // Enable only ADC
    charge_ctrl |= BQ25798_ADC_EN;
    ret = bq25798_write_reg(BQ25798_CHRG_CTRL_0, charge_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable ADC");
        return ret;
    }
    
    // Read back to verify
    ret = bq25798_read_reg(BQ25798_CHRG_CTRL_0, &charge_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read charge control register after write");
        return ret;
    }
    ESP_LOGI(TAG, "Charge Control Register (after): 0x%02X", charge_ctrl);

    // Configure ADC control register
    uint8_t adc_ctrl = 0;
    ret = bq25798_read_reg(BQ25798_ADC_CTRL, &adc_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ADC control register");
        return ret;
    }
    
    ESP_LOGI(TAG, "ADC Control Register (before): 0x%02X", adc_ctrl);

    // Enable all ADC channels
    adc_ctrl = 0xA0;  // Enable all ADC channels
    ret = bq25798_write_reg(BQ25798_ADC_CTRL, adc_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC");
        return ret;
    }
    
    // Read back to verify
    ret = bq25798_read_reg(BQ25798_ADC_CTRL, &adc_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ADC control register after write");
        return ret;
    }
    ESP_LOGI(TAG, "ADC Control Register (after): 0x%02X", adc_ctrl);
    
    ESP_LOGI(TAG, "BQ25798 initialization complete");
    return ESP_OK;
}

// Set charge current
esp_err_t bq25798_set_charge_current(uint16_t current_ma)
{
    uint8_t reg_value = (current_ma * 1000) / BQ25798_ICHRG_CURRENT_STEP_uA;
    return bq25798_write_reg(BQ25798_CHRG_I_LIM_MSB, reg_value);
}

// Set maximum charge voltage
esp_err_t bq25798_set_max_charge_voltage(uint16_t voltage_mv)
{
    uint8_t reg_value = (voltage_mv * 1000) / BQ25798_VREG_V_STEP_uV;
    return bq25798_write_reg(BQ25798_CHRG_V_LIM_MSB, reg_value);
}

// Get charge status
esp_err_t bq25798_get_charge_status(uint8_t *status)
{
    return bq25798_read_reg(BQ25798_CHRG_STAT_1, status);
}

// Get battery voltage
esp_err_t bq25798_get_battery_voltage(uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(BQ25798_ADC_VBAT_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(BQ25798_ADC_VBAT_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Get charge current
esp_err_t bq25798_get_charge_current(uint16_t *current_ma)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(BQ25798_ADC_IBAT_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(BQ25798_ADC_IBAT_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *current_ma = (adc_value * BQ25798_ADC_CURR_STEP_uA) / 1000;
    return ESP_OK;
}

// Get input current
esp_err_t bq25798_get_input_current(uint16_t *current_ma)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(BQ25798_ADC_IBUS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(BQ25798_ADC_IBUS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *current_ma = (adc_value * BQ25798_ADC_CURR_STEP_uA) / 1000;
    return ESP_OK;
}

// Get input voltage
esp_err_t bq25798_get_input_voltage(uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(BQ25798_ADC_VBUS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(BQ25798_ADC_VBUS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Get system voltage
esp_err_t bq25798_get_system_voltage(uint16_t *voltage_mv)
{
    uint8_t msb, lsb;
    esp_err_t ret = bq25798_read_reg(BQ25798_ADC_VSYS_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = bq25798_read_reg(BQ25798_ADC_VSYS_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Combine MSB and LSB into 16-bit value
    uint16_t adc_value = (msb << 8) | lsb;
    *voltage_mv = (adc_value * BQ25798_ADC_VOLT_STEP_uV) / 1000;
    return ESP_OK;
}

// Monitor charging status task
void bq25798_monitor_task(void *pvParameters)
{
    // Initialize variables with safe default values
    uint16_t battery_voltage = 0;
    uint16_t charge_current = 0;
    uint16_t input_voltage = 0;
    uint16_t input_current = 0;
    uint8_t charge_status = 0;
    
    ESP_LOGI(TAG, "BQ25798 monitor task started");
    
    while (1) {
        // Read all parameters
        bq25798_get_battery_voltage(&battery_voltage);
        bq25798_get_charge_current(&charge_current);
        bq25798_get_input_voltage(&input_voltage);
        bq25798_get_input_current(&input_current);
        bq25798_get_charge_status(&charge_status);
        
        // Take mutex before updating device_info
        if (xSemaphoreTake(device_info_mutex, portMAX_DELAY) == pdTRUE) {
            // Update device_info structure with current values
            device_info.battery_voltage_volts = battery_voltage / 1000.0f;
            
            // Check for abnormally high charge current and set to zero if detected
            if (charge_current > 10000) {
                ESP_LOGW(TAG, "Abnormally high charge current detected: %d mA, setting to zero", charge_current);
                charge_current = 0;
            }
            
            // If input voltage is zero (no cable connected), force charge current to zero
            if (input_voltage == 0) {
                device_info.charge_current_amps = 0.0f;
                device_info.input_current_amps = 0.0f;
                ESP_LOGI(TAG, "No input voltage detected, setting charge current to zero");
            } else {
                device_info.charge_current_amps = charge_current / 1000.0f;
                device_info.input_current_amps = input_current / 1000.0f;
            }
            
            device_info.input_voltage_volts = input_voltage / 1000.0f;
            
            // Extract charge status bits (7-5)
            device_info.charge_state = (charge_status >> 5) & 0x07;
            
            // Extract VBUS status bits (4-1)
            device_info.vbus_status = (charge_status >> 1) & 0x0F;
            
            // Update is_charging flag in device_info
            // Only consider charging if:
            // 1. Input voltage is non-zero
            // 2. Charge current is positive
            // 3. Charge state is active (1-4: Trickle, Pre, Fast, or Taper charge)
            device_info.is_charging = (input_voltage > 0) && 
                                    (charge_current > 0) && 
                                    (device_info.charge_state >= 1 && device_info.charge_state <= 4);
            
            // Release the mutex
            xSemaphoreGive(device_info_mutex);
        }
        
        // Interpret charge state
        char* charge_state_str = charge_state_strings[device_info.charge_state];
        
        // Interpret VBUS status
        char* vbus_status_str = vbus_status_strings[device_info.vbus_status];
        
        //ESP_LOGI(TAG, "Charge Status: %s, VBUS Status: %s", charge_state_str, vbus_status_str);
        
        // Print device info summary
        //ESP_LOGI(TAG, "Device Info Summary:");
        //ESP_LOGI(TAG, "  Battery Voltage: %.2f V", device_info.battery_voltage_volts);
        //ESP_LOGI(TAG, "  Charge Current: %.3f A", device_info.charge_current_amps);
        //ESP_LOGI(TAG, "  Input Voltage: %.2f V", device_info.input_voltage_volts);
        //ESP_LOGI(TAG, "  Is Charging: %s", device_info.is_charging ? "Yes" : "No");
        
        // Delay before next check - update every 1 second for more responsive monitoring
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


esp_err_t bq25798_disable_watchdog(void)
{
    ESP_LOGI(TAG, "Setting Charger Control 1 register to 0x80");
    
    // Set the Charger Control 1 register to 0x80 (10000000 in binary)
    esp_err_t ret = bq25798_write_reg(BQ25798_CHRG_CTRL_1, 0x80);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to Charger Control 1 register");
        return ret;
    }
    
    // Verify the change
    uint8_t verify_value;
    ret = bq25798_read_reg(BQ25798_CHRG_CTRL_1, &verify_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to verify Charger Control 1 register");
        return ret;
    }
    
    ESP_LOGI(TAG, "New Charger Control 1 value: 0x%02X", verify_value);
    
    // Check if the value matches what we set
    if (verify_value != 0x80) {
        ESP_LOGW(TAG, "Charger Control 1 register value does not match expected value");
    } else {
        ESP_LOGI(TAG, "Charger Control 1 register set successfully");
    }
    
    return ESP_OK;
} 