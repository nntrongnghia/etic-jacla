#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "string.h"
static const char *TAG = "LORA";

static const int RX_BUF_SIZE = 1024;

#define U1TXD (17)
#define U1RXD (18)

#define UART UART_NUM_2

void init(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, U1TXD, U1RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void app_main(void)
{
    init();
	char* Txdata = (char*) malloc(100);
    __int8_t num = 0;
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    sprintf(Txdata, "AT+DEVEUI=AC1F09FFFE05464A?\r\n");
    uart_write_bytes(UART, Txdata, strlen(Txdata));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    sprintf(Txdata, "AT+APPEUI=0000000000000001?\r\n");
    uart_write_bytes(UART, Txdata, strlen(Txdata));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    sprintf(Txdata, "AT+APPKEY=796B3580CEE62BA795DF7E452BBE708E?\r\n");
    uart_write_bytes(UART, Txdata, strlen(Txdata));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    sprintf(Txdata, "AT+JOIN=1:0:10:8?\r\n");
    uart_write_bytes(UART, Txdata, strlen(Txdata));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {
    	sprintf(Txdata, "AT+SEND=2:12345678?\r\n");
        uart_write_bytes(UART, Txdata, strlen(Txdata));
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}