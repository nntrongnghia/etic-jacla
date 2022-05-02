#pragma once

#include "esp_log.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RELAY_OUTPUT_IO          (gpio_num_t)35

#define LED_G_TIMER              LEDC_TIMER_0
#define LED_G_MODE               LEDC_LOW_SPEED_MODE
#define LED_G_OUTPUT_IO          (13) // Define the output GPIO
#define LED_G_CHANNEL            LEDC_CHANNEL_0
#define LED_G_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LED_G_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define LED_R_TIMER              LEDC_TIMER_0
#define LED_R_MODE               LEDC_LOW_SPEED_MODE
#define LED_R_OUTPUT_IO          (14) // Define the output GPIO
#define LED_R_CHANNEL            LEDC_CHANNEL_1
#define LED_R_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LED_R_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#ifdef __cplusplus
extern "C"
{
#endif

static void led_init(void){
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t led_g_timer;
    led_g_timer.speed_mode       = LED_G_MODE;
    led_g_timer.timer_num        = LED_G_TIMER;
    led_g_timer.duty_resolution  = LED_G_DUTY_RES;
    led_g_timer.freq_hz          = LED_G_FREQUENCY;  // Set output frequency at 5 kHz
    led_g_timer.clk_cfg          = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&led_g_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t led_g_chanel;
    led_g_chanel.speed_mode     = LED_G_MODE;
    led_g_chanel.channel        = LED_G_CHANNEL;
    led_g_chanel.timer_sel      = LED_G_TIMER;
    led_g_chanel.intr_type      = LEDC_INTR_DISABLE;
    led_g_chanel.gpio_num       = LED_G_OUTPUT_IO;
    led_g_chanel.duty           = 0; // Set duty to 0%
    led_g_chanel.hpoint         = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&led_g_chanel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t led_r_timer;
    led_r_timer.speed_mode       = LED_R_MODE;
    led_r_timer.timer_num        = LED_R_TIMER;
    led_r_timer.duty_resolution  = LED_R_DUTY_RES;
    led_r_timer.freq_hz          = LED_R_FREQUENCY;  // Set output frequency at 5 kHz
    led_r_timer.clk_cfg          = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&led_r_timer));
    
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t led_r_chanel;
    led_r_chanel.speed_mode     = LED_R_MODE;
    led_r_chanel.channel        = LED_R_CHANNEL;
    led_r_chanel.timer_sel      = LED_R_TIMER;
    led_r_chanel.intr_type      = LEDC_INTR_DISABLE;
    led_r_chanel.gpio_num       = LED_R_OUTPUT_IO;
    led_r_chanel.duty           = 0; // Set duty to 0%
    led_r_chanel.hpoint         = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&led_r_chanel));
}

/*
0x0 = off
0x1 = green
0x2 = orange
0x3 = red
others = off
*/
void set_led_color(uint8_t color){
    switch (color)
    {
        case 0:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 0));
            break;
        case 1:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 1));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 0));
            break;
        case 2:
            ESP_ERROR_CHECK(ledc_set_duty(LED_G_MODE, LED_G_CHANNEL, 1)); // Set duty to 25%. ((2^8) - 1) * 25% = 64
            ESP_ERROR_CHECK(ledc_update_duty(LED_G_MODE, LED_G_CHANNEL)); // Update duty to apply the new value

            ESP_ERROR_CHECK(ledc_set_duty(LED_R_MODE, LED_R_CHANNEL, 255)); // Set duty to 75%. ((2^8) - 1) * 75% = 190
            ESP_ERROR_CHECK(ledc_update_duty(LED_R_MODE, LED_R_CHANNEL)); // Update duty to apply the new value
            break;
        case 3:
            ESP_ERROR_CHECK(ledc_stop(LED_G_MODE, LED_G_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_stop(LED_R_MODE, LED_R_CHANNEL, 1));
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
/*
0 = off
1 = on
*/
void set_relay(bool state){
    if(state == 1){
        gpio_set_level(RELAY_OUTPUT_IO, 1);
    }else{
        gpio_set_level(RELAY_OUTPUT_IO, 0);
    }
}

#ifdef __cplusplus
}
#endif