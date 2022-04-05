/* Non-Volatile Storage (NVS) Read and Write a Value - Example
   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "database.h"

static const char *TAG = "SCAN_HISTORY_TEST";

#define BUTTON GPIO_NUM_19 // to simulate the user scan action
#define CLEAR_BUTTON GPIO_NUM_10 // to clear history

#define DEBOUNCE_TIME 2 // minimum time [s] between 2 scans

static QueueHandle_t scan_evt_queue = NULL;

// button isr send an event to queue
static void IRAM_ATTR button_isr_handler(void* arg)
{
    xQueueSendFromISR(scan_evt_queue, NULL, NULL);
}


// scan task waits for events from scan actions (ISR)
static void scan_history_task(void* arg)
{
    char uid[] = "ABCD";
    time_t now, debounce_now;
    debounce_now = 0;

    ScanHistoryDB history_db(4);

    ESP_LOGI(TAG, "block size %zd bytes", history_db.get_block_size());
    ESP_LOGI(TAG, "uid size %zd bytes", history_db.get_uid_size());
    ESP_LOGI(TAG, "Number of entries: %d", history_db.get_nb_entries());

    // history_db.clear_history();
    // history_db.close();

    for(;;) {
        if(xQueueReceive(scan_evt_queue, NULL, 100)) {
            now = time(NULL);
            if(now > debounce_now + DEBOUNCE_TIME){
                // Add scan to history
                history_db.add_history(uid, now);
                ESP_LOGI(TAG, "Number of entries: %d - cursor %d", history_db.get_nb_entries(), history_db.get_cursor());
                // Print all history
                history_db.print_all_history();

                // ESP_LOGI(TAG, "User: %s - time: %ld", uid, now);
                debounce_now = now;
            }
        }
    }
}


extern "C" void app_main(void)
{
    // Config button to simulate user scan action
    gpio_config_t button_conf = {
        .pin_bit_mask = (BIT(BUTTON)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&button_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON, button_isr_handler, NULL);
    
    scan_evt_queue = xQueueCreate(10, 0);
    
    // ScanHistoryDB history_db(4);
    // history_db.clear_history();
    // history_db.close();


    xTaskCreatePinnedToCore(scan_history_task, TAG, 4 * 1024, NULL, 6, NULL, 0);

    while(1){
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    
}