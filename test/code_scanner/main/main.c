#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "camera.h"
#include "aes.h"
#include "esp_code_scanner.h"
#include "misc.h"

static const char *TAG = "APP_CODE_SCANNER";
const unsigned char dec_key[] = CONFIG_AES_KEY;
const unsigned int keybits = 128;

static void decode_task()
{
    int64_t time1, time2, time3;
    // Init AES
    size_t dec_len;
    unsigned char dec_output[128];
    unsigned char dec_input[128];
    // Check AES key
    // ESP_LOGI(TAG, "Dec key len: %d", strlen((const char *)dec_key));
    SANITY_CHECK_M(strlen((const char *)dec_key)*8, keybits, TAG, "Invalid AES key. Run idf.py menuconfig to set AES key");

    // Init camera
    SANITY_CHECK_M(app_camera_init(), ESP_OK, TAG, "Fail to init camera");
    camera_fb_t *fb = NULL;

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
        esp_code_scanner_config_t config = {ESP_CODE_SCANNER_MODE_FAST, ESP_CODE_SCANNER_IMAGE_RGB565, fb->width, fb->height};
        esp_code_scanner_set_config(esp_scn, config);
        int decoded_num = esp_code_scanner_scan_image(esp_scn, fb->buf);

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
        }
        esp_code_scanner_destroy(esp_scn);

        esp_camera_fb_return(fb);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(decode_task, TAG, 4 * 1024, NULL, 6, NULL, 0);
}