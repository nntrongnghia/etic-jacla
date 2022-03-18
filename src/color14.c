#include "color14.h"
#include "misc.h"

/**
 * @brief i2c master initialization for Color 14
 */
esp_err_t color14_init()
{
    int i2c_num = COLOR14_I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_COLOR14_SDA,
        .scl_io_num = CONFIG_COLOR14_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = COLOR14_I2C_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(i2c_num, &conf);
    if (err != ESP_OK) {
        ESP_LOGE("color14_init", "i2c_param_config error code: %d", err);
        return err;
    }
    return i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
}


esp_err_t color14_register_read(uint8_t reg_addr, size_t size, uint8_t *out_buffer)
{
    i2c_port_t i2c_num = COLOR14_I2C_MASTER_NUM;
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (COLOR14_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (COLOR14_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, out_buffer, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, out_buffer + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


esp_err_t color14_write_byte(uint8_t reg_addr, uint8_t val){
    i2c_port_t i2c_num = COLOR14_I2C_MASTER_NUM;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (COLOR14_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, val, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


uint8_t color14_read_byte(uint8_t reg_addr){
    uint8_t val[1];
    color14_register_read(reg_addr, 1, val);
    return val[0];
}

esp_err_t color14_activate_light_sensor(){
    // Get current value of MAIN_CTRL reg
    uint8_t reg = color14_read_byte(COLOR14_REG_MAIN_CTRL);
    // Write LS_EN to MAIN_CTRL reg
    return color14_write_byte(COLOR14_REG_MAIN_CTRL, reg | 0x02);
}


esp_err_t color14_deactivate_light_sensor(){
    // Get current value of MAIN_CTRL reg
    uint8_t reg = color14_read_byte(COLOR14_REG_MAIN_CTRL);
    // Write LS_EN to MAIN_CTRL reg
    return color14_write_byte(COLOR14_REG_MAIN_CTRL, reg & 0xFD);
}


uint32_t color14_read_als(){
    uint8_t bytes[3];
    uint32_t ret;
    color14_register_read(COLOR14_REG_LS_DATA_GREEN_0, 3, bytes);
    ret = bytes[2] << 16 | bytes[1] << 8 | bytes[0];
    return ret;
}


esp_err_t color14_enable_als_var_int(){
    uint8_t reg;
    // Get current value of INT_CFG reg
    reg = color14_read_byte(COLOR14_REG_INT_CFG);

    // Enable ALS interrupt in variation mode
    // LS_VAR_MODE = 1
    // LS_INT_EN = 1
    // LS_INT_SEL = 0b01
    return color14_write_byte(COLOR14_REG_INT_CFG, reg | 0x1C);
}


esp_err_t color14_set_ls_persist(uint8_t nb_meas){
    uint8_t reg;
    uint8_t ls_persist = (nb_meas - 1) << 4;
    // Get current value of INT_CFG reg
    reg = color14_read_byte(COLOR14_REG_INT_PST);
    return color14_write_byte(COLOR14_REG_INT_CFG, reg | ls_persist);
}


esp_err_t color14_set_ls_thres_var(uint8_t exp){
    if(exp > 7) exp = 7;
    return color14_write_byte(COLOR14_REG_LS_THRES_VAR, exp);
}


uint8_t color14_get_ls_int_status(){
    uint8_t reg = color14_read_byte(COLOR14_REG_MAIN_STATUS);
    return (reg >> 4) & 0x01;
}