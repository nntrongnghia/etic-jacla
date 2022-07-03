#pragma once

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
/*
void led_init(void);
void set_led_color(uint8_t color);
void relay_init(void);
void set_relay(bool state);*/