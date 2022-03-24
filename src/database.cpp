#include "database.h"

#define JACLA_PARTITION_USER "user_db"
#define _TAG "jacla_db"

#define LOG_ERR(err)                                       \
    if (err != ESP_OK)                                     \
    {                                                      \
        ESP_LOGI(_TAG, "Error: %s", esp_err_to_name(err)); \
        ESP_ERROR_CHECK(err);                              \
    }

void UserDB::open(){
    // Initialize NVS
    esp_err_t err = nvs_flash_init_partition(JACLA_PARTITION_USER);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition(JACLA_PARTITION_USER));
        err = nvs_flash_init_partition(JACLA_PARTITION_USER);
    }
    LOG_ERR(err);
    ESP_LOGI(_TAG, "NVS init from %s partition: DONE", JACLA_PARTITION_USER);

    // Open
    err = nvs_open_from_partition(JACLA_PARTITION_USER, "storage", NVS_READWRITE, &nvs_handle);
    LOG_ERR(err);
    err = nvs_set_u8(nvs_handle, "dummy", 123);
    LOG_ERR(err);
}

void UserDB::close(){
    nvs_close(nvs_handle);
}

uint8_t UserDB::get(const char* uid){
    uint8_t value;
    esp_err_t err = nvs_get_u8(nvs_handle, uid, &value);
    LOG_ERR(err);
    return value;
}

void UserDB::set(const char* uid, uint8_t value){
    esp_err_t err = nvs_set_u8(nvs_handle, uid, value);
    LOG_ERR(err);
    err = nvs_commit(nvs_handle);
    LOG_ERR(err);
}





