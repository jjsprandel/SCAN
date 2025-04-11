/*
 * Header file for I2C communication with the CYPD3177 EZ-PD BCR
 *
 * This file defines register addresses, commands, macros, and structs
 * for interacting with the CYPD3177 over I2C.
 */

#ifndef CYPD3177_I2C_H
#define CYPD3177_I2C_H
 
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include <driver/gpio.h>
#include <stdint.h>

extern TaskHandle_t cypd3177_task_handle;
extern int usb_connected;

#define CYPD3177_INTR_PIN               GPIO_NUM_5 // GPIO_NUM_18
#define FORMAT(i2c_addr)                    { (uint8_t)((i2c_addr) & 0xFF), (uint8_t)(((i2c_addr) >> 8) & 0xFF) }
#define CYPD3177_I2C_ADDR               0x08  // 7-bit I2C address of the BCR

/* Status Registers */
#define DEVICE_MODE_REG_ADDR            0x0000  // Always returns 0x95
#define SILICON_ID_REG_ADDR             0x0002  // Read-only Silicon ID (0x11B0)
#define INTERRUPT_TYPE_REG_ADDR         0x0006  // Interrupt status register
#define PD_STATUS_REG_ADDR              0x1008  // Power Delivery status
#define TYPE_C_STATUS_REG_ADDR          0x100C  // Type-C port status
#define BUS_VOLTAGE_REG_ADDR            0x100D  // VBUS voltage in 100mV units
#define CURRENT_PDO_REG_ADDR            0x1010  // Active Power Data Object (PDO)
#define CURRENT_RDO_REG_ADDR            0x1014  // Active Request Data Object (RDO)
#define SWAP_RESPONSE_REG_ADDR          0x1028  // Swap response control
#define EVENT_STATUS_REG_ADDR           0x1044  // Type-C/PD event status
#define DEV_RESPONSE_REG_ADDR           0x007E  // Device response
#define PD_RESPONSE_REG_ADDR            0x1400  // Power Delivery response

/* Command Registers */
#define RESET_REG_ADDR                  0x0008  // Reset device or I2C module
#define EVENT_MASK_REG_ADDR             0x1024  // Event mask configuration
#define DM_CONTROL_REG_ADDR             0x1000  // Send Power Delivery Data Message
#define SELECT_SINK_PDO_REG_ADDR        0x1005  // Select Sink PDO settings
#define PD_CONTROL_REG_ADDR             0x1006  // Send PD control message
#define REQUEST_REG_ADDR                0x1050  // Send custom PD Request Data Object

/* Data Memory Registers */
#define READ_DATA_MEM_REG_ADDR          0x1404
#define WRITE_DATA_MEM_REG_ADDR         0x1800

/* PDO Type Definitions */
#define PDO_TYPE_FIXED_SUPPLY           0x0
#define PDO_TYPE_BATTERY                0x1
#define PDO_TYPE_VARIABLE               0x2

/* PDO Bit Masks */
#define PDO_TYPE_MASK                   0xC0000000
#define PDO_TYPE_SHIFT                  30
#define PDO_VOLTAGE_MASK                0x3FF00000
#define PDO_VOLTAGE_SHIFT               20
#define PDO_CURRENT_MASK                0x000FFC00
#define PDO_CURRENT_SHIFT               10

/* Special Reset Commands */
#define RESET_SIGNATURE        0x52  // 'R' character required for reset

/* Expected Response Codes */
#define RESPONSE_SUCCESS       0x02  // Command executed successfully
#define RESPONSE_INVALID_CMD   0x09  // Invalid command or argument
#define RESPONSE_TRANS_FAILED  0x0C  // Transaction failed
#define RESPONSE_PD_FAILED     0x0D  // PD command failed
#define RESPONSE_PORT_BUSY     0x12  // PD port busy

/* DEVICE MODE Register Bit Definitions*/
typedef struct {
    uint8_t current_mode;
} device_mode_reg_t;

/* SILICON ID Register Bit Definitions*/
typedef struct {
    uint16_t silicon_id;
} silicon_id_reg_t;

/* INTERRUPT Register Bit Definitions */
typedef struct __attribute__((packed)) {
    uint8_t device_interrupt : 1;
    uint8_t pd_port_interrupt : 1;
    uint8_t reserved : 6;
} interrupt_type_reg_t;

/* PD_STATUS Register Bit Definitions */
typedef struct {
    uint8_t default_config : 6;
    uint8_t current_port_role : 1;
    uint8_t reserved_1 : 1;
    uint8_t current_power_role : 1;
    uint8_t reserved_2 : 1;
    uint8_t contract_state : 1;
    uint8_t reserved_3 : 5;
} pd_status_reg_t;

/* TYPE_C_STATUS Register Bit Definitions */
typedef struct {
    uint8_t partner_connected : 1;
    uint8_t cc_polarity : 1;
    uint8_t attached_device : 3;
    uint8_t reserved : 3;
} type_c_status_reg_t;

#define TYPE_C_DEVICE_ATTACHED    0x01
#define TYPE_C_CC_POLARITY_CC2    0x01

/* BUS_VOLTAGE Register Bit Definitions */
typedef struct __attribute__((packed)) {
    uint8_t voltage : 8;
} bus_voltage_reg_t;

/* SWAP_RESPONSE Register Bit Definitions */
typedef struct {
    uint8_t dr_swap_response : 2;
    uint8_t reserved_1 : 2;
    uint8_t vconn_swap_response : 2;
    uint8_t reserved_2 : 2;
} swap_response_reg_t;

/* EVENT_STATUS Register Bit Definitions */
typedef struct {
    uint8_t type_c_event : 1;
    uint8_t pd_event : 1;
    uint8_t reserved : 6;
} event_status_reg_t;

/* DEV_RESPONSE Register Bit Definitions*/
typedef struct __attribute__((packed)) {
    uint8_t response_code : 8;
    uint8_t response_type : 1;
    uint8_t length : 7;
} dev_response_reg_t;

/* PD_RESPONSE Register Bit Definitions */
typedef struct __attribute__((packed)) {
    uint8_t response_code : 8;
    uint8_t response_type : 1;
    uint8_t length1 : 7;
    uint16_t length2 : 16;
} pd_response_reg_t;

#define PD_RESPONSE_GOOD_CRC    0x01
#define PD_RESPONSE_ACCEPT      0x03
#define PD_RESPONSE_REJECT      0x04
#define PD_RESPONSE_WAIT        0x07
#define PD_RESPONSE_ERROR       0x05
#define PD_RESPONSE_SOFT_RESET  0x0D

extern i2c_master_dev_handle_t cypd3177_i2c_handle;
extern i2c_device_config_t cypd3177_i2c_config;
extern gpio_config_t cypd3177_intr_config;


/* Function Prototypes */
void power_check(void *pvParameter);
void get_interrupt_response_code(void *pvParameter);
void get_dev_response(void *pvParameter);
void get_pd_response(void *pvParameter);
esp_err_t enable_high_power_charging(void);
esp_err_t cypd3177_change_pdo(void);

extern void IRAM_ATTR cypd3177_isr_handler(void *arg);

#endif /* CYPD3177_I2C_H */
 