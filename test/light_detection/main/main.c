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
#include "color14.h"
#include "misc.h"
#include "esp_intr_alloc.h"

static const char *TAG = "LIGHT_DETECTION";
static QueueHandle_t gpio_evt_queue = NULL;

void IRAM_ATTR color14_isr_handler()
{
    xQueueSendFromISR(gpio_evt_queue, NULL, NULL);
}


static void light_detection_task()
{
    int isr_count = 0;
    while (1)
    {   
        if(xQueueReceive(gpio_evt_queue, NULL, 1)){
            isr_count += 1;
            color14_get_ls_int_status();
        }
        ESP_LOGI("Color14", "ALS: %d\tisr_count: %d", color14_read_als(), isr_count);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    // Color14 init
    SANITY_CHECK(color14_init(), ESP_OK);
    color14_activate_light_sensor();
    color14_enable_als_var_int();
    // color14_set_ls_persist(2);
    color14_set_ls_thres_var(4);

    // ESP32 interrupt init
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << CONFIG_COLOR14_INT,
        .mode = GPIO_MODE_INPUT,
        // .pull_up_en = 1,
        // .pull_down_en = 1,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(CONFIG_COLOR14_INT, color14_isr_handler, NULL);
    
    // esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, color14_isr_handler, NULL, NULL);
    // gpio_isr_register(color14_isr_handler, NULL, 0, NULL);

    // Create event queue to handle interrupt event
    gpio_evt_queue = xQueueCreate(10, 0);

    xTaskCreatePinnedToCore(light_detection_task, TAG, 4 * 1024, NULL, 6, NULL, 0);
}