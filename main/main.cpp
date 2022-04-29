#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "camera.h"
#include "database.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "esp_code_scanner.h"
#include "color14.h"
#include "xnucleo_nfc.h"

#define BRIGHTNESS_THRESHOLD 2
#define EXAMPLE_UART_WAKEUP_THRESHOLD   3

typedef enum
{
    INIT,
    IDLE,
    READ_QR,
    READ_RFID,
    CHECK_UID,
    LORA
} state_t;

esp_err_t init();
state_t idle();
void read_qr();
void read_rfid();
void lora();
void check_uid();

static const char *TAG = "MAIN";

XNucleoNFC nfc_reader;

extern "C" void app_main()
{
    state_t state = INIT;
    while (true)
    {
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
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

esp_err_t init()
{
    // Color14 init
    ESP_ERROR_CHECK(color14_init());
    ESP_ERROR_CHECK(color14_activate_light_sensor());
    ESP_ERROR_CHECK(color14_enable_als_var_int());
    // color14_set_ls_persist(2);
    ESP_ERROR_CHECK(color14_set_ls_thres_var(BRIGHTNESS_THRESHOLD));

    // ESP32 interrupt init
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << CONFIG_COLOR14_INT,
        .mode = GPIO_MODE_INPUT,
        // .pull_up_en = 1,
        // .pull_down_en = 1,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    /* Enable wake up from GPIO */
    ESP_RETURN_ON_ERROR(gpio_wakeup_enable((gpio_num_t)CONFIG_COLOR14_INT, GPIO_INTR_LOW_LEVEL),
                        TAG, "Enable gpio wakeup failed");
    ESP_RETURN_ON_ERROR(esp_sleep_enable_gpio_wakeup(), TAG, "Configure gpio as wakeup source failed");

    // Camera init

    // RFID init
    nfc_reader.init();
    nfc_reader.echo();
    nfc_reader.tag_detection_calibration();
    nfc_reader.idle_tag_detector(NFC_WU_TAG | NFC_WU_SPI_SS);
    /* UART will wakeup the chip up from light sleep if the edges that RX pin received has reached the threshold
     * Besides, the Rx pin need extra configuration to enable it can work during light sleep */
    ESP_RETURN_ON_ERROR(gpio_sleep_set_direction((gpio_num_t)NFC_IRQ_OUT, GPIO_MODE_INPUT), TAG, "Set uart sleep gpio failed");
    ESP_RETURN_ON_ERROR(gpio_sleep_set_pull_mode((gpio_num_t)NFC_IRQ_OUT, GPIO_PULLUP_ONLY), TAG, "Set uart sleep gpio failed");
    ESP_RETURN_ON_ERROR(uart_set_wakeup_threshold(NFC_UART_PORT, EXAMPLE_UART_WAKEUP_THRESHOLD),
                        TAG, "Set uart wakeup threshold failed");
    /* Only uart0 and uart1 (if has) support to be configured as wakeup source */
    ESP_RETURN_ON_ERROR(esp_sleep_enable_uart_wakeup(NFC_UART_PORT),
                        TAG, "Configure uart as wakeup source failed");
    return ESP_OK;
}

state_t idle()
{
    state_t ret = IDLE;
    //calculate timer interrupt
    //enable timer wake up
    ESP_LOGW(TAG, "Entering light sleep");
    /* To make sure the complete line is printed before entering sleep mode,
        * need to wait until UART TX FIFO is empty:
        */
    uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Get timestamp before entering sleep */
    int64_t t_before_us = esp_timer_get_time();

    /* Enter sleep mode */
    esp_light_sleep_start();

    /* Get timestamp after waking up from sleep */
    int64_t t_after_us = esp_timer_get_time();

    /* Determine wake up reason */
    const char *wakeup_reason;
    switch (esp_sleep_get_wakeup_cause())
    {
    case ESP_SLEEP_WAKEUP_GPIO:
        wakeup_reason = "pin";
        color14_get_ls_int_status();
        ret = READ_QR;
        break;
    case ESP_SLEEP_WAKEUP_UART:
        wakeup_reason = "uart";
        vTaskDelay(1);
        ret = READ_RFID;
        break;
    default:
        wakeup_reason = "other";
        ret = IDLE;
        break;
    }
    ESP_LOGI(TAG, "Returned from light sleep, reason: %s, t=%lld ms, slept for %lld ms",
                wakeup_reason, t_after_us / 1000, (t_after_us - t_before_us) / 1000);
    // if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
    //     /* Waiting for the gpio inactive, or the chip will continously trigger wakeup*/
    //     example_wait_gpio_inactive();
    // }
    return ret;
}
void read_qr()
{
    //led orange
    //wake up camera
    //read QRcode
    //decrypt message
    //update RTC
    //retrun ID
}
void read_rfid()
{
    //led orange
    //read ID
    //retrun ID

    while(1) // TODO for 
    {
        if(nfc_reader.is_tag_available())
        {
            ESP_LOGI(TAG, "Tag detected");
            if(nfc_reader.get_tag_uid())
            {
                nfc_reader.print_uid();
            }
            break;
        }
        vTaskDelay(10);
    }
    nfc_reader.idle_tag_detector(NFC_WU_TAG | NFC_WU_SPI_SS);
}

void lora()
{
    //read history
    //send history

    //receipt something ?
}

void check_uid()
{
    //verify ID
    //if ok : relay ON, green led
    //else : relay OFF, red led
}