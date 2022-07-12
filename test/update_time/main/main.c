/**
 * The QR message is a string with a date, for example "2022-06-01-11-11"
 * This code shows how to convert a string to Unix time,
 * then we can set the system time with the Unix time.
 * 
 * In the code, we can get the current time by time() function.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include <sys/time.h>

static const char *TAG = "UPDATE_TIME";


time_t char2unixtime(char* str){
    struct tm tm;
    time_t t;
    // initialize tm
    memset(&tm, 0, sizeof(struct tm));
    // convert a string of time to structure tm
    strptime(str, "%Y-%m-%d-%H-%M", &tm);
    // get unix time from tm
    t = mktime(&tm);
    return t;
}


static void simple_task()
{
    
    time_t now;
    struct timeval tv;
    tv.tv_sec = char2unixtime("2022-06-07-13-11");
    settimeofday(&tv, NULL);


    while (1)
    {
        time(&now);
        ESP_LOGI(TAG, "unix time: %ld", tv.tv_sec);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(simple_task, TAG, 4 * 1024, NULL, 6, NULL, 0);
}