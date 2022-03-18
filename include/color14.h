#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "driver/i2c.h"


/**
 * @brief Generic I2C define
 * 
 */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

/**
 * @brief Color 14 pin configuration
 * 
 */
#define COLOR14_I2C_MASTER_NUM 1
#define COLOR14_I2C_FREQ_HZ 100000


/**
 * @brief Color 14 description register.
 * @details Specified register for description of Color 14 Click driver.
 */
#define COLOR14_REG_MAIN_CTRL       0x00
#define COLOR14_REG_PS_VCSEL        0x01
#define COLOR14_REG_PS_PULSES       0x02
#define COLOR14_REG_PS_MEASRATE     0x03
#define COLOR14_REG_LS_MEAS_RATE    0x04
#define COLOR14_REG_LS_GAIN         0x05
#define COLOR14_REG_PART_ID         0x06
#define COLOR14_REG_MAIN_STATUS     0x07
#define COLOR14_REG_PS_DATA_0       0x08
#define COLOR14_REG_PS_DATA_1       0x09
#define COLOR14_REG_LS_DATA_IR_0    0x0A
#define COLOR14_REG_LS_DATA_IR_1    0x0B
#define COLOR14_REG_LS_DATA_IR_2    0x0C
#define COLOR14_REG_LS_DATA_GREEN_0 0x0D
#define COLOR14_REG_LS_DATA_GREEN_1 0x0E
#define COLOR14_REG_LS_DATA_GREEN_2 0x0F
#define COLOR14_REG_LS_DATA_BLUE_0  0x10
#define COLOR14_REG_LS_DATA_BLUE_1  0x11
#define COLOR14_REG_LS_DATA_BLUE_2  0x12
#define COLOR14_REG_LS_DATA_RED_0   0x13
#define COLOR14_REG_LS_DATA_RED_1   0x14
#define COLOR14_REG_LS_DATA_RED_2   0x15
#define COLOR14_REG_INT_CFG         0x19        
#define COLOR14_REG_INT_PST         0x1A
#define COLOR14_REG_PS_THRES_UP_0   0x1B
#define COLOR14_REG_PS_THRES_UP_1   0x1C
#define COLOR14_REG_PS_THRES_LOW_0  0x1D
#define COLOR14_REG_PS_THRES_LOW_1  0x1E
#define COLOR14_REG_PS_CAN_0        0x1F
#define COLOR14_REG_PS_CAN_1_ANA    0x20
#define COLOR14_REG_LS_THRES_UP_0   0x21
#define COLOR14_REG_LS_THRES_UP_1   0x22
#define COLOR14_REG_LS_THRES_UP_2   0x23
#define COLOR14_REG_LS_THRES_LOW_0  0x24
#define COLOR14_REG_LS_THRES_LOW_1  0x25
#define COLOR14_REG_LS_THRES_LOW_2  0x26
#define COLOR14_REG_LS_THRES_VAR    0x27


/**
 * @brief Color 14 device ID data.
 * @details Specified data for device ID of Color 14 Click.
 */
#define COLOR14_ID 0xC2

/**
 * @brief Color 14 device address setting.
 * @details Specified setting for device slave address selection of
 * Color 14 Click driver.
 */
#define COLOR14_ADDR  0x52

/**
 * @brief Color 14 color data object.
 * @details Color data object of Color 14 Click driver.
 */
typedef struct
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t ir;

} color14_color_t;


/**
 * @brief Color 14 Click return value data.
 * @details Predefined enum values for driver return values.
 */
typedef enum
{
   COLOR14_OK = 0,
   COLOR14_ERROR = -1,
   COLOR14_ERROR_OVF = -2,
   COLOR14_ERROR_PARAM = -3,
   COLOR14_ERROR_CFG = -4

} color14_return_value_t;



/**
 * @brief i2c master initialization for Color 14
 */
esp_err_t color14_init();

typedef struct
{
    i2c_port_t i2c_num;
} color14_cfg_t;

/**
 * @brief Read register from an adress
 *
 * @param reg_addr register address
 * @param size number of bytes to read
 * @param out_buffer buffer to store read values
 * @return esp_err_t
 */
esp_err_t color14_register_read(uint8_t reg_addr, size_t size, uint8_t *out_buffer);

/**
 * @brief Read an 8-bit register
 * 
 * @return uint8_t 
 */
uint8_t color14_read_byte(uint8_t reg_addr);


/**
 * @brief Activate Light sensor
 * 
 * @return esp_err_t 
 */
esp_err_t color14_activate_light_sensor();

/**
 * @brief Deactivate Light sensor
 * 
 * @return esp_err_t 
 */
esp_err_t color14_deactivate_light_sensor();


/**
 * @brief Read Ambient Light Sensor
 * 
 * @return uint32_t 
 */
uint32_t color14_read_als();


/**
 * @brief Enable Ambient Light interrupt in variation mode
 * 
 */
esp_err_t color14_enable_als_var_int();


/**
 * @brief Set the LS_PERSIST which controls the number of consecutive 
 * LS measures to assert an interrupt
 * 
 * @param nb_meas Must be in range 1..16
 * @return esp_err_t 
 */
esp_err_t color14_set_ls_persist(uint8_t nb_meas);


/**
 * @brief Set the variation threshold to assert an interrupt
 * raw LS threshold value = 8*2^exp
 * exp in range 0 - 7
 * @param exp
 * @return esp_err_t 
 */
esp_err_t color14_set_ls_thres_var(uint8_t exp);


/**
 * @brief Get LS INTERRUPT STATUS bit of MAIN_STATUS reg
 * 
 * @return either 0 or 1
 */
uint8_t color14_get_ls_int_status();