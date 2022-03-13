#include "app_peripherals.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "app_peripherals";

void led_init(void){
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
    if(state == TRUE){
        gpio_set_level(RELAY_OUTPUT_IO, HIGH);
    }else{
        gpio_set_level(RELAY_OUTPUT_IO, LOW);
    }
}