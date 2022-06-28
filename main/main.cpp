#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "camera.h"
#include "database.h"
#include "esp_code_scanner.h"
#include "color14.h"

typedef enum {
    INIT, IDLE, 
    READ_QR, READ_RFID, 
    CHECK_UID, LORA
} state_t;

void init(){
    color14_init();
}
state_t idle();
void read_qr();
void read_rfid();
void lora();
void check_uid();

extern "C" void app_main()
{
    state_t state = INIT;  
    switch (state)
    {
    case INIT:
        init();
        state = IDLE;
        break;
    case IDLE:
        state = idle();
        break;
    case READ_QR:
        read_qr();
        state = CHECK_UID;
        break;
    case READ_RFID:
        read_rfid();
        state = CHECK_UID;
        break;
    case LORA:
        lora();
        state = IDLE;
    case CHECK_UID:
        check_uid();
        state = IDLE;
        break;
    default:
        state = IDLE;
        break;
    }
}