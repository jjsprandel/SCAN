/*
 * Header file for I2C communication with the CYPD3177 EZ-PD BCR
 *
 * This file defines register addresses, commands, macros, and structs
 * for interacting with the CYPD3177 over I2C.
 */

#ifndef CYPD3177_I2C_H
#define CYPD3177_I2C_H
 
#include <stdint.h>
#include "../../../i2c/include/i2c_config.h"
 
#define CYPD3177_I2C_ADDR      0x08  // 7-bit I2C address of the BCR

/* Status Registers */
#define DEVICE_MODE_REG        0x0000  // Always returns 0x92
#define SILICON_ID_REG         0x0002  // Read-only Silicon ID (0x11B0)
#define INTERRUPT_REG          0x0006  // Interrupt status register
#define PD_STATUS_REG          0x1008  // Power Delivery status
#define TYPE_C_STATUS_REG      0x100C  // Type-C port status
#define BUS_VOLTAGE_REG        0x100D  // VBUS voltage in 100mV units
#define CURRENT_PDO_REG        0x1010  // Active Power Data Object (PDO)
#define CURRENT_RDO_REG        0x1014  // Active Request Data Object (RDO)
#define SWAP_RESPONSE_REG      0x1028  // Swap response control
#define EVENT_STATUS_REG       0x1044  // Type-C/PD event status
#define PD_RESPONSE_REG        0x1400  // Power Delivery response

/* Command Registers */
#define RESET_REG              0x0008  // Reset device or I2C module
#define EVENT_MASK_REG         0x1024  // Event mask configuration
#define DM_CONTROL_REG         0x1000  // Send Power Delivery Data Message
#define SELECT_SINK_PDO_REG    0x1005  // Select Sink PDO settings
#define PD_CONTROL_REG         0x1006  // Send PD control message
#define REQUEST_REG            0x1050  // Send custom PD Request Data Object
#define SET_GPIO_MODE_REG      0x0080  // Configure GPIO mode
#define SET_GPIO_LEVEL_REG     0x0081  // Set GPIO level

/* Special Reset Commands */
#define RESET_SIGNATURE        0x52  // 'R' character required for reset

/* Expected Response Codes */
#define RESPONSE_SUCCESS       0x02  // Command executed successfully
#define RESPONSE_INVALID_CMD   0x09  // Invalid command or argument
#define RESPONSE_TRANS_FAILED  0x0C  // Transaction failed
#define RESPONSE_PD_FAILED     0x0D  // PD command failed
#define RESPONSE_PORT_BUSY     0x12  // PD port busy

/* INTERRUPT Register Bit Definitions */
typedef struct {
    uint8_t device_interrupt : 1;
    uint8_t pd_port_interrupt : 1;
    uint8_t reserved : 6;
} cypd3177_interrupt_reg_t;

#define INTERRUPT_DEVICE_PENDING  0x01
#define INTERRUPT_PD_PENDING      0x01

/* PD_STATUS Register Bit Definitions */
typedef struct {
    uint8_t default_config : 6;
    uint8_t current_port_role : 1;
    uint8_t reserved_1 : 1;
    uint8_t current_power_role : 1;
    uint8_t reserved_2 : 1;
    uint8_t contract_state : 1;
    uint8_t reserved_3 : 5;
} cypd3177_pd_status_reg_t;

#define PD_STATUS_CONTRACT_ACTIVE 0x01

/* TYPE_C_STATUS Register Bit Definitions */
typedef struct {
    uint8_t partner_connected : 1;
    uint8_t cc_polarity : 1;
    uint8_t attached_device : 3;
    uint8_t reserved : 3;
} cypd3177_type_c_status_reg_t;

#define TYPE_C_DEVICE_ATTACHED    0x01
#define TYPE_C_CC_POLARITY_CC2    0x01

/* BUS_VOLTAGE Register Bit Definitions */
typedef struct {
    uint16_t voltage;
} cypd3177_bus_voltage_reg_t;

/* SWAP_RESPONSE Register Bit Definitions */
typedef struct {
    uint8_t dr_swap_response : 2;
    uint8_t reserved_1 : 2;
    uint8_t vconn_swap_response : 2;
    uint8_t reserved_2 : 2;
} cypd3177_swap_response_reg_t;

#define SWAP_ACCEPT   0x00
#define SWAP_REJECT   0x01
#define SWAP_WAIT     0x02
#define SWAP_NOT_SUP  0x03

/* EVENT_STATUS Register Bit Definitions */
typedef struct {
    uint8_t type_c_event : 1;
    uint8_t pd_event : 1;
    uint8_t reserved : 6;
} cypd3177_event_status_reg_t;

#define EVENT_TYPE_C  0x01
#define EVENT_PD      0x01

/* PD_RESPONSE Register Bit Definitions */
typedef struct {
    uint8_t message_type : 5;
    uint8_t port_role : 1;
    uint8_t spec_revision : 2;
    uint8_t message_id : 3;
    uint8_t data_objects : 3;
    uint8_t extended : 1;
    uint8_t reserved : 1;
} cypd3177_pd_response_reg_t;

#define PD_RESPONSE_GOOD_CRC    0x01
#define PD_RESPONSE_ACCEPT      0x03
#define PD_RESPONSE_REJECT      0x04
#define PD_RESPONSE_WAIT        0x07
#define PD_RESPONSE_ERROR       0x05
#define PD_RESPONSE_SOFT_RESET  0x0D

extern i2c_master_dev_handle_t cypd3177_i2c_handle;
extern i2c_device_config_t cypd3177_i2c_config;


/* Function Prototypes */
int cypd3177_write_register(uint16_t reg, uint8_t *data, uint8_t length);
int cypd3177_read_register(uint16_t reg, uint8_t *buffer, uint8_t length);

#endif /* CYPD3177_I2C_H */
 