#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RELAY_OUTPUT_IO          (23)

#define LED_G_TIMER              LEDC_TIMER_0
#define LED_G_MODE               LEDC_LOW_SPEED_MODE
#define LED_G_OUTPUT_IO          (5) // Define the output GPIO
#define LED_G_CHANNEL            LEDC_CHANNEL_0
#define LED_G_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LED_G_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define LED_R_TIMER              LEDC_TIMER_0
#define LED_R_MODE               LEDC_LOW_SPEED_MODE
#define LED_R_OUTPUT_IO          (18) // Define the output GPIO
#define LED_R_CHANNEL            LEDC_CHANNEL_1
#define LED_R_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LED_R_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

static void led_init(void){
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t led_g_timer = {
        .speed_mode       = LED_G_MODE,
        .timer_num        = LED_G_TIMER,
        .duty_resolution  = LED_G_DUTY_RES,
        .freq_hz          = LED_G_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&led_g_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t led_g_chanel = {
        .speed_mode     = LED_G_MODE,
        .channel        = LED_G_CHANNEL,
        .timer_sel      = LED_G_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_G_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&led_g_chanel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t led_r_timer = {
        .speed_mode       = LED_R_MODE,
        .timer_num        = LED_R_TIMER,
        .duty_resolution  = LED_R_DUTY_RES,
        .freq_hz          = LED_R_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&led_r_timer));
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t led_r_chanel = {
        .speed_mode     = LED_R_MODE,
        .channel        = LED_R_CHANNEL,
        .timer_sel      = LED_R_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_R_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&led_r_chanel));
}

/*
0x0 = off
0x1 = rouge
0x2 = verte
0x3 = orange
*/
void set_led_color(uint8_t color){
    switch (color)
    {
        case 0:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 0));
            break;
        case 1:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 1));
            break;
        case 2:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 1));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 0));
            break;
        case 3:
            ESP_ERROR_CHECK(ledc_set_duty(LED_G_MODE, LED_G_CHANNEL, 64)); // Set duty to 25%. ((2^8) - 1) * 25% = 64
            ESP_ERROR_CHECK(ledc_update_duty(LED_G_MODE, LED_G_CHANNEL)); // Update duty to apply the new value

            ESP_ERROR_CHECK(ledc_set_duty(LED_R_MODE, LED_R_CHANNEL, 190)); // Set duty to 75%. ((2^8) - 1) * 75% = 190
            ESP_ERROR_CHECK(ledc_update_duty(LED_R_MODE, LED_R_CHANNEL)); // Update duty to apply the new value
            break;
        default:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 0));
            break;
    }
}

void relay_init(void){
    gpio_set_direction(RELAY_OUTPUT_IO, GPIO_MODE_OUTPUT);
}

void set_relay(bool state){
    if(state == 1){
        gpio_set_level(RELAY_OUTPUT_IO, 1);
    }else{
        gpio_set_level(RELAY_OUTPUT_IO, 0);
    }
}

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
