#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "color14.h"

static const char *TAG = "LIGHT_DETECTION";

static void light_detection_task()
{
    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(light_detection_task, TAG, 4 * 1024, NULL, 6, NULL, 0);
}