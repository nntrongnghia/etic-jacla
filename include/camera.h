#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_camera.h"

/**
 * @brief Camera pin configuration
 * 
 */
/*
#define CAMERA_MODULE_NAME "ESP-EYE"
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 4
#define CAMERA_PIN_SIOD 18
#define CAMERA_PIN_SIOC 23

#define CAMERA_PIN_D7 36
#define CAMERA_PIN_D6 37
#define CAMERA_PIN_D5 38
#define CAMERA_PIN_D4 39
#define CAMERA_PIN_D3 35
#define CAMERA_PIN_D2 14
#define CAMERA_PIN_D1 13
#define CAMERA_PIN_D0 34
#define CAMERA_PIN_VSYNC 5
#define CAMERA_PIN_HREF 27
#define CAMERA_PIN_PCLK 25
*/

#define CAMERA_MODULE_NAME "ESP-EYE"
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1

#define CAMERA_PIN_XCLK 5
#define CAMERA_PIN_SIOD 42
#define CAMERA_PIN_SIOC 3

#define CAMERA_PIN_D7 10
#define CAMERA_PIN_D6 6
#define CAMERA_PIN_D5 11
#define CAMERA_PIN_D4 7
#define CAMERA_PIN_D3 12
#define CAMERA_PIN_D2 15
#define CAMERA_PIN_D1 13
#define CAMERA_PIN_D0 16
#define CAMERA_PIN_VSYNC 46
#define CAMERA_PIN_HREF 4
#define CAMERA_PIN_PCLK 9

#define XCLK_FREQ_HZ 20000000
#define CAMERA_PIXFORMAT PIXFORMAT_GRAYSCALE
#define CAMERA_FRAME_SIZE FRAMESIZE_240X240//240*240
#define CAMERA_FB_COUNT 1

/**
 * @brief Init camera
 * 
 */
#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t app_camera_init();
#ifdef __cplusplus
}
#endif