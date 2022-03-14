#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HELLO_WORLD_APP";

static void simple_task()
{
    while (1)
    {
        ESP_LOGI(TAG, "Hello ETIC");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(simple_task, TAG, 4 * 1024, NULL, 6, NULL, 0);
}