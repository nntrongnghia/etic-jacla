#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "color14.h"
#include "esp_intr_alloc.h"

#include "misc.h"


#define NFC_IRQ_IN 39 
#define NFC_IRQ_OUT 38

// Define UART settings
#define NFC_BAUDRATE 57600
#define NFC_UART_PORT UART_NUM_1
#define NFC_DATA_BITS UART_DATA_8_BITS
#define NFC_PARITY UART_PARITY_DISABLE
#define NFC_STOP_BITS UART_STOP_BITS_2
#define NFC_FLOWCTRL UART_HW_FLOWCTRL_DISABLE

#define NFC_UART_BUFFER_SIZE 1024

#define NFC_MAX_NUM_CMDS 528

// Define command codes
#define NFC_CMD_ECHO 0x55
#define NFC_CMD_IDN 0x01
#define NFC_CMD_PS 0x02 // PS = Protocol Select
#define NFC_CMD_SENDRECV 0x04
#define NFC_CMD_IDLE 0x07
#define NFC_CMD_RDREG 0x08
#define NFC_CMD_WRREG 0x09
#define NFC_CMD_BAUDRATE 0x0A

#define DAC_GUARD 0x08

// Wakeup source
#define NFC_WU_TIMEOUT 0x01
#define NFC_WU_TAG 0x02
#define NFC_WU_IRQ_IN 0x08
#define NFC_WU_SPI_SS 0x10


#define NFC_FIELD_OFF 0x00
#define NFC_ISO_15693 0x01
#define NFC_ISO_14443A 0x02
#define NFC_ISO_14443B 0x03
#define NFC_ISO_18092 0x04

#define NFC_FRAME_RECV_OK 0x80

#define MIFARE_UID_SINGLE_SIZE 4
#define MIFARE_UID_DOUBLE_SIZE 7
#define MIFARE_UID_TRIPLE_SIZE 10

#define SAK_FAIL 0xFF

#define MIFARE_CL_1 0x93
#define MIFARE_CL_2 0x95
#define MIFARE_CL_3 0x97

#define MIFARE_CT 0x88


const uint8_t level_code[3] = {
    MIFARE_CL_1,
    MIFARE_CL_2,
    MIFARE_CL_3
};

// Command to select protocol ISO 14443A
const uint8_t PS_ISO_14443A_CMD[4] = {
    NFC_CMD_PS, 0x02,
    NFC_ISO_14443A,
    0x00 // Transmission rate = 106 Kbps, reception data rate = 106 Kbps
};

// Only support ISO 14443A
class XNucleoNFC {
    private:
        const char *tag = "XNucleoNFC";
        uint8_t dac_data_ref;
        uint8_t temp_data[5]; // contain temporary data while anticol
        uint8_t uid_size = 0;
        uint8_t uid[10];

        size_t wait_get_uart_response(uint32_t timeout_ms, uint32_t num_trials);
        uint8_t wait_get_wakeup_response(const uint8_t* idle_cmd);
        void update_uid_size(uint8_t atqa_first_byte);
        
        /**
         * @brief Performs ISO/IEC 14443 ANTICOLLISION command
         * 
         * @param Mifare level 
         * @return success/failure
         */
        bool anticol(uint8_t level);


        /**
         * @brief Performs ISO/IEC 14443 SELECT command
         *
         * @param level CL1, CL2 or CL3
         * @return SAK (select acknowledge) or 0xFF in case of failure
         */
        uint8_t select(uint8_t level);

    public:
        QueueHandle_t uart_queue;
        uart_event_t event;
        uint8_t *rx_buffer;

        void clean_buffer(){bzero(rx_buffer, NFC_UART_BUFFER_SIZE);}
        void init();
        void echo();
        void idle_tag_detector(uint8_t wu_source);
        void tag_detection_calibration();
        void set_iso_14443A();
        bool is_tag_available();
        void print_message(size_t len);

        /**
         * @brief Get the tag uid object
         * 
         * @return size_t len of uid, 0 if failure
         */
        size_t get_tag_uid();
        size_t listen(TickType_t xTicksToWait);
        void print_uid();
};