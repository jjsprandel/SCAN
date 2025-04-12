#ifndef BQ25798_H
#define BQ25798_H

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Device Information
#define BQ25798_MANUFACTURER    "Texas Instruments"
#define BQ25798_NAME            "bq25798"

// I2C address
#define BQ25798_I2C_ADDR 0x6B

// BQ25798 charge states
#define BQ25798_CHARGE_STATE_NOT_CHARGING      0
#define BQ25798_CHARGE_STATE_TRICKLE_CHARGE    1
#define BQ25798_CHARGE_STATE_PRE_CHARGE        2
#define BQ25798_CHARGE_STATE_FAST_CHARGE       3
#define BQ25798_CHARGE_STATE_TAPER_CHARGE      4
#define BQ25798_CHARGE_STATE_RESERVED          5
#define BQ25798_CHARGE_STATE_TOP_OFF_TIMER     6
#define BQ25798_CHARGE_STATE_TERMINATION_DONE  7

// Mutex for protecting access to device_info
extern SemaphoreHandle_t device_info_mutex;

// Voltage values used in initialization - declared as extern const
extern const uint16_t vsysmin_mv;  // Minimum system voltage in mV (10.5V = 10500mV)
extern const uint16_t vreg_mv;     // Maximum charge voltage in mV (14.0V = 14000mV)

// Default Values
#define BQ25798_DEFAULT_CHARGE_CURRENT_MA 1000  // 1A default charge current

// Register addresses
#define BQ25798_MIN_SYS_V       0x00
#define BQ25798_CHRG_V_LIM_MSB  0x01
#define BQ25798_CHRG_V_LIM_LSB  0x02
#define BQ25798_CHRG_I_LIM_MSB  0x03
#define BQ25798_CHRG_I_LIM_LSB  0x04
#define BQ25798_INPUT_V_LIM     0x05
#define BQ25798_INPUT_I_LIM_MSB 0x06
#define BQ25798_INPUT_I_LIM_LSB 0x07
#define BQ25798_PRECHRG_CTRL    0x08
#define BQ25798_TERM_CTRL       0x09
#define BQ25798_RECHRG_CTRL     0x0a
#define BQ25798_VOTG_REG        0x0b
#define BQ25798_IOTG_REG        0x0d
#define BQ25798_TIMER_CTRL      0x0e
#define BQ25798_CHRG_CTRL_0     0x0f
#define BQ25798_CHRG_CTRL_1     0x10
#define BQ25798_CHRG_CTRL_2     0x11
#define BQ25798_CHRG_CTRL_3     0x12
#define BQ25798_CHRG_CTRL_4     0x13
#define BQ25798_CHRG_CTRL_5     0x14
#define BQ25798_MPPT_CTRL       0x15
#define BQ25798_TEMP_CTRL       0x16
#define BQ25798_NTC_CTRL_0      0x17
#define BQ25798_NTC_CTRL_1      0x18
#define BQ25798_ICO_I_LIM       0x19
#define BQ25798_CHRG_STAT_0     0x1b
#define BQ25798_CHRG_STAT_1     0x1c
#define BQ25798_CHRG_STAT_2     0x1d
#define BQ25798_CHRG_STAT_3     0x1e
#define BQ25798_CHRG_STAT_4     0x1f
#define BQ25798_FAULT_STAT_0    0x20
#define BQ25798_FAULT_STAT_1    0x21
#define BQ25798_CHRG_FLAG_0     0x22
#define BQ25798_CHRG_FLAG_1     0x23
#define BQ25798_CHRG_FLAG_2     0x24
#define BQ25798_CHRG_FLAG_3     0x25
#define BQ25798_FAULT_FLAG_0    0x26
#define BQ25798_FAULT_FLAG_1    0x27
#define BQ25798_CHRG_MSK_0      0x28
#define BQ25798_CHRG_MSK_1      0x29
#define BQ25798_CHRG_MSK_2      0x2a
#define BQ25798_CHRG_MSK_3      0x2b
#define BQ25798_FAULT_MSK_0     0x2c
#define BQ25798_FAULT_MSK_1     0x2d
#define BQ25798_ADC_CTRL        0x2e
#define BQ25798_FN_DISABE_0     0x2f
#define BQ25798_FN_DISABE_1     0x30
#define BQ25798_ADC_IBUS_MSB    0x31
#define BQ25798_ADC_IBUS_LSB    0x32
#define BQ25798_ADC_IBAT_MSB    0x33
#define BQ25798_ADC_IBAT_LSB    0x34
#define BQ25798_ADC_VBUS_MSB    0x35
#define BQ25798_ADC_VBUS_LSB    0x36
#define BQ25798_ADC_VAC1        0x37
#define BQ25798_ADC_VAC2        0x39
#define BQ25798_ADC_VBAT_MSB    0x3b
#define BQ25798_ADC_VBAT_LSB    0x3c
#define BQ25798_ADC_VSYS_MSB    0x3d
#define BQ25798_ADC_VSYS_LSB    0x3e
#define BQ25798_ADC_TS          0x3f
#define BQ25798_ADC_TDIE        0x41
#define BQ25798_ADC_DP          0x43
#define BQ25798_ADC_DM          0x45
#define BQ25798_DPDM_DRV        0x47
#define BQ25798_PART_INFO       0x48

// Control Bits
#define BQ25798_CHRG_EN         BIT(5)
#define BQ25798_ADC_EN          BIT(7)

// Charger Status 1
#define BQ25798_CHG_STAT_MSK    0b11100000  // bits 7-5
#define BQ25798_NOT_CHRGING     0
#define BQ25798_TRICKLE_CHRG    BIT(5)
#define BQ25798_PRECHRG         BIT(6)
#define BQ25798_FAST_CHRG       (BIT(5) | BIT(6))
#define BQ25798_TAPER_CHRG      BIT(7)
#define BQ25798_TOP_OFF_CHRG    (BIT(6) | BIT(7))
#define BQ25798_TERM_CHRG       (BIT(5) | BIT(6) | BIT(7))
#define BQ25798_VBUS_PRESENT    BIT(0)

// VBUS Status
#define BQ25798_VBUS_STAT_MSK   0b00011110  // bits 4-1
#define BQ25798_USB_SDP         BIT(1)
#define BQ25798_USB_CDP         BIT(2)
#define BQ25798_USB_DCP         (BIT(1) | BIT(2))
#define BQ25798_HVDCP           BIT(3)
#define BQ25798_UNKNOWN_3A      (BIT(3) | BIT(1))
#define BQ25798_NON_STANDARD    (BIT(3) | BIT(2))
#define BQ25798_OTG_MODE        (BIT(3) | BIT(2) | BIT(1))
#define BQ25798_UNQUAL_ADAPT    BIT(4)
#define BQ25798_DIRECT_PWR      (BIT(4) | BIT(2) | BIT(1))

// Charger Status 4
#define BQ25798_TEMP_HOT        BIT(0)
#define BQ25798_TEMP_WARM       BIT(1)
#define BQ25798_TEMP_COOL       BIT(2)
#define BQ25798_TEMP_COLD       BIT(3)
#define BQ25798_TEMP_MASK       0b00001111  // bits 3-0

// Additional Status Bits
#define BQ25798_OTG_OVP         BIT(5)
#define BQ25798_VSYS_OVP        BIT(6)
#define BQ25798_PG_STAT         BIT(3)

// Current Control
#define BQ25798_PRECHRG_CUR_MASK                0b00111111  // bits 5-0
#define BQ25798_PRECHRG_CURRENT_STEP_uA         40000
#define BQ25798_PRECHRG_I_MIN_uA                40000
#define BQ25798_PRECHRG_I_MAX_uA                2000000
#define BQ25798_PRECHRG_I_DEF_uA                120000
#define BQ25798_TERMCHRG_CUR_MASK               0b00011111  // bits 4-0
#define BQ25798_TERMCHRG_CURRENT_STEP_uA        40000
#define BQ25798_TERMCHRG_I_MIN_uA               40000
#define BQ25798_TERMCHRG_I_MAX_uA               1000000
#define BQ25798_TERMCHRG_I_DEF_uA               200000
#define BQ25798_ICHRG_CURRENT_STEP_uA           10000
#define BQ25798_ICHRG_I_MIN_uA                  50000
#define BQ25798_ICHRG_I_MAX_uA                  5000000
#define BQ25798_ICHRG_I_DEF_uA                  1000000

// Voltage Control
#define BQ25798_VREG_V_MAX_uV   18800000
#define BQ25798_VREG_V_MIN_uV   3000000
#define BQ25798_VREG_V_DEF_uV   3600000
#define BQ25798_VREG_V_STEP_uV  10000

// VSYSMIN Control
#define BQ25798_VSYSMIN_OFFSET_mV 2500
#define BQ25798_VSYSMIN_STEP_mV   250

// VREG Control
#define BQ25798_VREG_STEP_mV      10

// ICHG Control
#define BQ25798_ICHRG_STEP_mA      10

// IINDPM Control
#define BQ25798_IINDPM_STEP_mA     10

// Input Current Control
#define BQ25798_IINDPM_I_MIN_uA 100000
#define BQ25798_IINDPM_I_MAX_uA 3300000
#define BQ25798_IINDPM_STEP_uA  10000
#define BQ25798_IINDPM_DEF_uA   1000000

// Input Voltage Control
#define BQ25798_VINDPM_V_MIN_uV 3600000
#define BQ25798_VINDPM_V_MAX_uV 22000000
#define BQ25798_VINDPM_STEP_uV  100000
#define BQ25798_VINDPM_DEF_uV   3600000

// ADC Control
#define BQ25798_ADC_VOLT_STEP_uV        1000
#define BQ25798_ADC_CURR_STEP_uA        1000

// Watchdog Control
#define BQ25798_WATCHDOG_MASK   0b00000111  // bits 2-0
#define BQ25798_WATCHDOG_DIS    0
#define BQ25798_WATCHDOG_MAX    160000

extern i2c_master_dev_handle_t bq25798_i2c_handle;
extern i2c_device_config_t bq25798_i2c_config;

// Function declarations
esp_err_t bq25798_init(void);
esp_err_t bq25798_read_reg(uint8_t reg_addr, uint8_t *data);
esp_err_t bq25798_write_reg(uint8_t reg_addr, uint8_t data);
esp_err_t bq25798_set_charge_current(uint16_t current_ma);
esp_err_t bq25798_set_max_charge_voltage(uint16_t voltage_mv);
esp_err_t bq25798_get_charge_status(uint8_t *status);
esp_err_t bq25798_get_battery_voltage(uint16_t *voltage_mv);
esp_err_t bq25798_get_charge_current(uint16_t *current_ma);
esp_err_t bq25798_get_input_current(uint16_t *current_ma);
esp_err_t bq25798_get_input_voltage(uint16_t *voltage_mv);
esp_err_t bq25798_get_system_voltage(uint16_t *voltage_mv);
void bq25798_monitor_task(void *pvParameters);

/**
 * @brief Disable the watchdog timer in the BQ25798 charger
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t bq25798_disable_watchdog(void);

#endif // BQ25798_H 