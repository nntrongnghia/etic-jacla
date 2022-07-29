#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"
#include "aes.h"
#include "esp_check.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "camera.h"
#include "database.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_code_scanner.h"
#include "color14.h"
#include "xnucleo_nfc.h"
#include "led_relay.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"

#define BRIGHTNESS_THRESHOLD 2
#define EXAMPLE_UART_WAKEUP_THRESHOLD 3
#define READ_QR_TIMEOUT 10000 // ms
#define CONFIG_CAMERA_CORE0

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
bool uid_read = false;


//decode qr code AES
const unsigned char dec_key[] = "#LogKerKey2022!!";//CONFIG_AES_KEY;
const unsigned int keybits = 128;

static void state_machine(void *args)
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


extern "C" void app_main()
{
    xTaskCreatePinnedToCore(state_machine, TAG, 100 * 1024, NULL, 6, NULL, 0);
}

esp_err_t init()
{
    /**** Led & Relay init ****/
    led_init();
    set_led_color(2);
    relay_init();
    set_relay(0);
    /**** Color14 init ****/
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
        // .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&io_conf);
    //Enable wake up from GPIO
    ESP_RETURN_ON_ERROR(gpio_wakeup_enable((gpio_num_t)CONFIG_COLOR14_INT, GPIO_INTR_LOW_LEVEL), TAG, "Enable gpio wakeup failed");
    ESP_RETURN_ON_ERROR(esp_sleep_enable_gpio_wakeup(), TAG, "Configure gpio as wakeup source failed");

    /**** Camera init ****/
    ESP_RETURN_ON_ERROR(app_camera_init(), TAG, "Fail to init camera");
    
    /**** NFC init ****/
    nfc_reader.init();
    nfc_reader.echo();
    nfc_reader.tag_detection_calibration();
    nfc_reader.idle_tag_detector(NFC_WU_TAG | NFC_WU_SPI_SS);
    /* UART will wakeup the chip up from light sleep if the edges that RX pin received has reached the threshold
     * Besides, the Rx pin need extra configuration to enable it can work during light sleep */
    ESP_RETURN_ON_ERROR(gpio_sleep_set_direction((gpio_num_t)NFC_IRQ_OUT, GPIO_MODE_INPUT), TAG, "Set uart sleep gpio failed");
    ESP_RETURN_ON_ERROR(gpio_sleep_set_pull_mode((gpio_num_t)NFC_IRQ_OUT, GPIO_PULLUP_ONLY), TAG, "Set uart sleep gpio failed");
    ESP_RETURN_ON_ERROR(uart_set_wakeup_threshold(NFC_UART_PORT, EXAMPLE_UART_WAKEUP_THRESHOLD), TAG, "Set uart wakeup threshold failed");
    /* Only uart0 and uart1 (if has) support to be configured as wakeup source */
    ESP_RETURN_ON_ERROR(esp_sleep_enable_uart_wakeup(NFC_UART_PORT), TAG, "Configure uart as wakeup source failed");
    return ESP_OK;
}

state_t idle()
{
    set_led_color(0);
    state_t ret = IDLE;
    //calculate timer interrupt
    //enable timer wake up
    ESP_LOGW(TAG, "Entering light sleep");
    /* To make sure the complete line is printed before entering sleep mode,
        * need to wait until UART TX FIFO is empty:
        */
    uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);


    /* Enter sleep mode */
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    color14_get_ls_int_status();
    esp_light_sleep_start();
    // TODO: color14 keeps waking up esp32

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
    // TODO: other case  : timer interrupt - LoRa
    ESP_LOGI(TAG, "Returned from light sleep, reason: %s", wakeup_reason);
    // if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
    //     /* Waiting for the gpio inactive, or the chip will continously trigger wakeup*/
    //     example_wait_gpio_inactive();
    // }
    return ret;
}
void read_qr()
{
    ESP_LOGI(TAG, "Read QR");
    set_led_color(3);
    camera_fb_t *fb = NULL;
    int64_t time1, time2, time3, end, start;
    unsigned char dec_output[128];
    unsigned char dec_input[128];
    size_t dec_len;
    int num_codes;
    start = esp_timer_get_time();
    while (1)
    {
        fb = esp_camera_fb_get();
        if (fb == NULL)
        {
            ESP_LOGI(TAG, "camera get failed\n");
            continue;
        }

        time1 = esp_timer_get_time();
        // Decode Progress
        esp_image_scanner_t *esp_scn = esp_code_scanner_create();
        if(!esp_scn) ESP_LOGE(TAG, "Fail to create ESP code scanner");
        esp_code_scanner_config_t config = {ESP_CODE_SCANNER_MODE_FAST, ESP_CODE_SCANNER_IMAGE_GRAY, fb->width, fb->height};
        esp_code_scanner_set_config(esp_scn, config);
        int decoded_num = esp_code_scanner_scan_image(esp_scn, fb->buf);
        ESP_LOGI(TAG, "decoded_num %d", decoded_num);
        // ESP_LOGI(TAG, "Image size: %zu bytes", fb->len);
        if (decoded_num)
        {
            // Read QR code message
            esp_code_scanner_symbol_t result = esp_code_scanner_result(esp_scn);
            time2 = esp_timer_get_time();
            ESP_LOGI(TAG, "Read QR code in %lld ms.", (time2 - time1) / 1000);
            ESP_LOGI(TAG, "%s: \"%s\"", result.type_name, result.data);

            // Convert QR code from Base64 to hex
            mbedtls_base64_decode(dec_input, 128, &dec_len, (const unsigned char *)result.data, strlen(result.data));

            // Decode message using AES-ECB
            aes_decrypt(dec_input, dec_len, dec_output, dec_key);
            dec_output[dec_len] = 0;
            time3 = esp_timer_get_time();
            ESP_LOGI(TAG, "Decode AES in %lld ms.", (time3 - time2) / 1000);
            ESP_LOGI(TAG, "QR message: %s", dec_output);
            break;
        }
        esp_code_scanner_destroy(esp_scn);

        esp_camera_fb_return(fb);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        end = esp_timer_get_time();
        if((end - start) / 1000 > READ_QR_TIMEOUT) break;
    }
    // TODO
    //update RTC
    //retrun ID
    // baisser le flag
    color14_get_ls_int_status();
}
void read_rfid()
{
    ESP_LOGI(TAG, "Read RFID");
    set_led_color(3);
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
    ESP_LOGI(TAG, "LORA");
    //read history
    //send history

    //receipt something ?
}

void check_uid()
{
    ESP_LOGI(TAG, "Check UID");
    if(uid_read)
    {
        set_led_color(1);
        set_relay(1);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        set_relay(0);
    }
    else
    {
        set_led_color(3);
    }
}