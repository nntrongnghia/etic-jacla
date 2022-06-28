#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_relay.h"

void app_main(void)
{
    // Set peripherals configuration
    led_init();
    relay_init();

    while(1)
    {
        for (size_t i = 0; i < 5; i++)
        {   
            set_relay(1);
            vTaskDelay(1000/portTICK_PERIOD_MS);   
            set_led_color(i);
            set_relay(0);
            vTaskDelay(1000/portTICK_PERIOD_MS);   
        }
    }
}