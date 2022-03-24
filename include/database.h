#pragma once
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#define JACLA_PARTITION_USER "user_db"
#define _TAG "jacla_db"

#define LOG_ERR(err)         \
    if(err != ESP_OK){ESP_LOGI(_TAG, "Error: %s", esp_err_to_name(err));} \
    else{ESP_LOGI(_TAG, "Done");}


class UserDB {
    private:
        nvs_handle_t nvs_handle;
    public:
        void open();
        void close();
        uint8_t get(const char* uid);
        void set(const char* uid, uint8_t value);
};







