#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "xnucleo_nfc.h"

static const char *TAG = "NFC_TEST";


static void nfc_task(void* arg)
{
    XNucleoNFC nfc_reader;
    size_t len;

    nfc_reader.init();
    nfc_reader.echo();
    nfc_reader.tag_detection_calibration();
    nfc_reader.idle_tag_detector(NFC_WU_TAG | NFC_WU_SPI_SS);
    while(1){
        // nfc_reader.listen(100/portTICK_PERIOD_MS);
        if(nfc_reader.is_tag_available()){
            ESP_LOGI(TAG, "Tag detected");
            if(nfc_reader.get_tag_uid()){
                nfc_reader.print_uid();
            } 
        } 
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


extern "C" void app_main(void)
{

    xTaskCreatePinnedToCore(nfc_task, TAG, 4 * 1024, NULL, 6, NULL, 0);

    // while(1){
    //     vTaskDelay(10/portTICK_PERIOD_MS);
    // }
    
}