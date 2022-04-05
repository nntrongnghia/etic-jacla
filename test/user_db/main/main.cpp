/* Non-Volatile Storage (NVS) Read and Write a Value - Example
   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "database.h"

static const char *TAG = "NVS_APP";

extern "C" void app_main(void)
{
    UserDB user_db;

    user_db.open();

    // user_db.set("123456", 1);
    // user_db.set("543210", 0);

    ESP_LOGI(TAG, "123456: %d", user_db.get("123456"));
    ESP_LOGI(TAG, "543210: %d", user_db.get("543210"));
    // ESP_LOGI(TAG, "404: %d", user_db.get("404"));

    user_db.close();

}