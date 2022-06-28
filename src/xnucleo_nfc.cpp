#include "xnucleo_nfc.h"
#include "string.h"

void check_uart_ret(const char *tag, int ret)
{
    if (ret < 0)
        ESP_LOGE(tag, "UART Error: %d", ret);
}

void XNucleoNFC::init()
{
    // Memory allocation for RX buffer
    rx_buffer = (uint8_t*) malloc(NFC_UART_BUFFER_SIZE);
    // Config UART PORT
    uart_config_t uart_config = {
        .baud_rate = NFC_BAUDRATE,
        .data_bits = NFC_DATA_BITS,
        .parity = NFC_PARITY,
        .stop_bits = NFC_STOP_BITS,
        .flow_ctrl = NFC_FLOWCTRL,
    };
    ESP_ERROR_CHECK(uart_param_config(NFC_UART_PORT, &uart_config));
    //  Config UART pins
    ESP_ERROR_CHECK(uart_set_pin(NFC_UART_PORT, NFC_IRQ_IN, NFC_IRQ_OUT, -1, -1));
    // UART driver installation
    ESP_ERROR_CHECK(uart_driver_install(NFC_UART_PORT, NFC_UART_BUFFER_SIZE*2,
                                        NFC_UART_BUFFER_SIZE*2, 10, &uart_queue, 0));
}


void XNucleoNFC::echo()
{
    uint8_t cmd[] = {NFC_CMD_ECHO};
    // Send Command
    ESP_LOGI(tag, "Sending echo to XNucleoNFC ...");
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, cmd, 1));
    // Get response
    size_t len = wait_get_uart_response(10, NFC_MAX_NUM_CMDS);
    if(len > 0 && rx_buffer[0] == NFC_CMD_ECHO)
        ESP_LOGI(tag, "Successfully echo to XNucleoNFC");
    else ESP_LOGE(tag, "Failed to echo to XNucleoNFC");
    clean_buffer();
}

void XNucleoNFC::idle_tag_detector(uint8_t wu_source)
{
    uint8_t cmd[16] = {
        NFC_CMD_IDLE, 0x0E, 
        wu_source, // WakeUp source
        0x21, 0x00, // Enter control
        0x79, 0x01, // WU control
        0x18, 0x00, // Leave Control
        0x20,       // WU period
        0x60,       // Osc Start
        0x60,       // DAC Start
        (uint8_t)(dac_data_ref - 8),       // DAC Data Low
        (uint8_t)(dac_data_ref + 8),       // DAC Data High
        0x3F,       // Swing Count
        0x08,       // Max Sleep
    };
    // Send the command
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, cmd, 16));
    ESP_LOGI(tag, "Entering Idle Tag Detetor mode");
}




void XNucleoNFC::tag_detection_calibration()
{
    const uint8_t max_dac_data = 0xFC;
    uint8_t wu_source;
    uint8_t step = 0x80;
    uint8_t cmd[16] = {
        NFC_CMD_IDLE, 0x0E, 
        NFC_WU_TAG | NFC_WU_TIMEOUT, // WakeUp source
        0xA1, 0x00, // Enter control = Tag Detector Calibration
        0xF8, 0x01, // WU control
        0x18, 0x00, // Leave Control
        0x20,       // WU period
        0x60,       // Osc Start
        0x60,       // DAC Start
        0x00,       // DacDataL
        max_dac_data, // DAC Data
        0x3F,       // Swing Count
        0x01,       // Max Sleep
    };
    uint8_t* pt_dac_data_h = cmd + 13;
    
    ESP_LOGI(tag, "Begin Tag Detection Calibration");
    // Step 0: force wake-up event to Tag Detect (set DacDataH = 0x00)
    // With these conditions Wake-Up event must be Tag Detect (0x02)
    ESP_LOGI(tag, "Step 0: force wake-up event to Tag Detect (set DacDataH = 0x00)");
    (*pt_dac_data_h) = 0x00;
    wu_source = wait_get_wakeup_response(cmd);
    if(wu_source == NFC_WU_TAG){ ESP_LOGI(tag, "Step 0: done"); clean_buffer();}
    else abort();

    // Step 1: force Wake-up event to Timeout (set DacDataH = 0xFC)
    // With these conditions, Wake-Up event must be Timeout
    ESP_LOGI(tag, "Step 1: force Wake-up event to Timeout (set DacDataH = 0xFC)");
    (*pt_dac_data_h) = max_dac_data;
    wu_source = wait_get_wakeup_response(cmd);
    if(wu_source == NFC_WU_TIMEOUT){ ESP_LOGI(tag, "Step 1: done"); clean_buffer();}
    else abort();

    // Step 2: new DacDataH value = 0x7C
    // If previous Wake-up event was Timeout (0x01) we must decrease DacDataH (-0x80)
    ESP_LOGI(tag, "Step 2: Search for DacDataRef");
    for (size_t i = 0; i < 6; i++)
    {
        if(wu_source == NFC_WU_TIMEOUT) (*pt_dac_data_h) = (*pt_dac_data_h) - step;
        else if(wu_source == NFC_WU_TAG) (*pt_dac_data_h) = (*pt_dac_data_h) + step;
        else ESP_LOGE(tag, "Invalid WakeUp source: %02x", wu_source);
        ESP_LOGI(tag, "DacDataH: 0x%02x", (*pt_dac_data_h));
        wu_source = wait_get_wakeup_response(cmd);
        step /= 2;
    }
    if(wu_source == NFC_WU_TIMEOUT) dac_data_ref = (*pt_dac_data_h) - 0x04;
    else if(wu_source == NFC_WU_TAG) dac_data_ref = (*pt_dac_data_h);
    ESP_LOGI(tag, "Tag Detection Calibration done. DacDataRef=%02x", dac_data_ref);
}

void XNucleoNFC::set_iso_14443A()
{
    // Send command
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, PS_ISO_14443A_CMD, 4));
    // Get response
    size_t len = wait_get_uart_response(10, NFC_MAX_NUM_CMDS);
    if(len == 2 && rx_buffer[0] == 0x00)
        ESP_LOGD(tag, "Successfully set the protocol 14443A");
    else ESP_LOGE(tag, "Failed to set the protocol 14443A");
    clean_buffer();
}

bool XNucleoNFC::is_tag_available()
{
    uid_size = 0;
    set_iso_14443A();
    uint8_t cmd[4] = {0x04, 0x02, 0x26, 0x07};
    bool ret = 0;
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, cmd, 4));
    size_t len = wait_get_uart_response(10, 10);
    // ESP_LOGI("is_tag_available", "len %d", len);
    // ESP_LOG_BUFFER_HEX("is_tag_available", rx_buffer, len);
    if(len > 2 && rx_buffer[0] == 0x80 && rx_buffer[1] == 0x05){
        ret = 1;
        update_uid_size(rx_buffer[2]);
        clean_buffer();
    } 
    else ret = 0;
    return ret;
}

void XNucleoNFC::print_message(size_t len)
{
    ESP_LOG_BUFFER_HEX(tag, rx_buffer, len);
    clean_buffer();
}

// Must be called after is_tag_available
size_t XNucleoNFC::get_tag_uid()
{
    uint8_t sak = 0x04;
    uint8_t data[5];
    // Anticollision loop
    for (auto i = 0; i < 3; i++) {
        if ((sak & 0x04) >> 2) {
            if (!anticol(i)) {
                ESP_LOGE(tag, "Fail to send ANTICOLLISION command, level %d", i+1);
                return 0;
            }
            sak = select(i);
            if (sak == SAK_FAIL) {
                ESP_LOGE(tag, "Fail to send SELECT command, level %d", i+1);
                return 0;
            }
        }
    }
    return uid_size;
}



size_t XNucleoNFC::listen(TickType_t xTicksToWait)
{
    size_t ret = 0;
    if(xQueueReceive(uart_queue, (void*)&(event), xTicksToWait)){
        if(event.type == UART_DATA){
            ESP_ERROR_CHECK(uart_get_buffered_data_len(NFC_UART_PORT, &ret));
            if(ret){
                clean_buffer();
                uart_read_bytes(NFC_UART_PORT, rx_buffer, event.size, 100/portTICK_PERIOD_MS);
            }
        }
    }
    if(ret){
        print_message(ret);
    }
    return ret;
}


size_t XNucleoNFC::wait_get_uart_response(uint32_t timeout_ms, uint32_t num_trials)
{
    clean_buffer();
    size_t length = 0;
    for (size_t i = 0; i < num_trials; i++)
    {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(NFC_UART_PORT, &length));
        if (length > 0)
        {
            length = uart_read_bytes(NFC_UART_PORT, rx_buffer, length, 100);
            check_uart_ret(tag, length);
            break;
        }
        vTaskDelay(timeout_ms / portTICK_PERIOD_MS);
    }
    return length;
}

uint8_t XNucleoNFC::wait_get_wakeup_response(const uint8_t* idle_cmd)
{
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, idle_cmd, 16));
    size_t len = wait_get_uart_response(10, NFC_MAX_NUM_CMDS);
    if(len != 3){
        ESP_LOGE(tag, "Idle response should have 3 bytes. Get len=%d", len);
        return -1;
    }
    return rx_buffer[2];
}

void XNucleoNFC::update_uid_size(uint8_t atqa_first_byte)
{
    uint8_t uid_size_code = atqa_first_byte >> 6;
    switch (uid_size_code)
    {
    case 0:
        uid_size = MIFARE_UID_SINGLE_SIZE;
        break;
    case 1:
        uid_size = MIFARE_UID_DOUBLE_SIZE;
        break;
    case 2:
        uid_size = MIFARE_UID_TRIPLE_SIZE;
        break;
    case 3:
        ESP_LOGW(tag, "Invalid UID size coding: %d", uid_size_code);
        break;
    default:
        break;
    }
}

uint8_t XNucleoNFC::select(uint8_t level)
{
    uint8_t cmd[10];
    cmd[0] = NFC_CMD_SENDRECV;
    cmd[1] = 8;
    cmd[2] = level_code[level];
    cmd[3] = 0x70;
    memcpy(cmd+4, temp_data, 5);
    cmd[9] = 0x28;
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, cmd, 10));
    size_t len = wait_get_uart_response(10, NFC_MAX_NUM_CMDS);
    // if(len) ESP_LOG_BUFFER_HEX("debug select", rx_buffer, len);
    if(len && (rx_buffer[0] == NFC_FRAME_RECV_OK)){
        return rx_buffer[2]; // SAK byte
    }
    return SAK_FAIL;
}


bool XNucleoNFC::anticol(uint8_t level)
{
    uint8_t cmd[5] = {NFC_CMD_SENDRECV, 3, level_code[level], 0x20, 0x08};
    check_uart_ret(tag, uart_write_bytes(NFC_UART_PORT, cmd, 5));
    size_t len = wait_get_uart_response(10, NFC_MAX_NUM_CMDS);
    // if(len) ESP_LOG_BUFFER_HEX("debug anticol", rx_buffer, len);
    if(len && (rx_buffer[0] == NFC_FRAME_RECV_OK)){
        memcpy(temp_data, rx_buffer+2, 5);
        // TODO check collision?
        if(temp_data[0] != MIFARE_CT) memcpy(uid + 3*level, temp_data, 4);
        else memcpy(uid + 3*level, temp_data + 1, 3);
        return true;
    }
    else return false;
}

void XNucleoNFC::print_uid(){
    ESP_LOGI(tag, "UID:");
    ESP_LOG_BUFFER_HEX(tag, uid, uid_size);
}