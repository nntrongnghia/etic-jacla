#include "database.h"
#include <cstring>

#define LOG_ERR(tag, err)                                       \
    if (err != ESP_OK)                                     \
    {                                                      \
        ESP_LOGE(tag, "Error: %s", esp_err_to_name(err)); \
        ESP_ERROR_CHECK(err);                              \
    }

void UserDB::open(){
    // Initialize NVS
    esp_err_t err = nvs_flash_init_partition(P_USER);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(_tag, "NVS partition %s was truncated and needs to be erased", P_HISTORY_CTRL);
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition(P_USER));
        ESP_LOGI(_tag, "NVS partition %s erased", P_HISTORY_CTRL);
        err = nvs_flash_init_partition(P_USER);
    }
    LOG_ERR(_tag, err);
    ESP_LOGI(_tag, "NVS init from %s partition: DONE", P_USER);

    // Open
    err = nvs_open_from_partition(P_USER, "storage", NVS_READWRITE, &nvs_handle);
    LOG_ERR(_tag, err);
}

void UserDB::close(){
    nvs_close(nvs_handle);
}

uint8_t UserDB::get(const char* uid){
    uint8_t value;
    esp_err_t err = nvs_get_u8(nvs_handle, uid, &value);
    LOG_ERR(_tag, err);
    return value;
}

void UserDB::set(const char* uid, uint8_t value){
    esp_err_t err = nvs_set_u8(nvs_handle, uid, value);
    LOG_ERR(_tag, err);
    err = nvs_commit(nvs_handle);
    LOG_ERR(_tag, err);
}



ScanHistoryDB::ScanHistoryDB(size_t uid_size){
    /* -------------------------- get partition handler ------------------------- */
    partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, P_HISTORY);
    assert(partition != NULL);
    ESP_LOGI(_tag, "Get Jacla Scan history partition successfully!");
    ESP_LOGI(_tag, "Partition \"%s\" size: %d bytes", P_HISTORY, partition->size);

    /* ---------- Open nvs handler to read/write cursor and block size ---------- */
    // Initialize NVS
    esp_err_t err = nvs_flash_init_partition(P_HISTORY_CTRL);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition(P_HISTORY_CTRL));
        err = nvs_flash_init_partition(P_HISTORY_CTRL);
    }
    LOG_ERR(_tag, err);
    ESP_LOGI(_tag, "NVS init from %s partition: DONE", P_HISTORY_CTRL);

    // Open
    err = nvs_open_from_partition(P_HISTORY_CTRL, "storage", NVS_READWRITE, &nvs_hist_ctrl);
    LOG_ERR(_tag, err);

    /* ----------------- get block size from nvs if available ----------------- */
    err = nvs_get_u32(nvs_hist_ctrl, "block_size", &block_size);
    if (err == ESP_OK)
    {
        if(block_size != uid_size + 4){
            ESP_LOGW(_tag, "Block size stored in NVS %s is not the equal (uid_size + 4)", P_HISTORY_CTRL);
        }
        ESP_LOGI(_tag, "Load block_size = %d stored in NVS %s", block_size, P_HISTORY_CTRL);
        this->uid_size = block_size - sizeof(time_t);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND){
        err = nvs_set_u32(nvs_hist_ctrl, "block_size", uid_size + 4); // sizeof(size_t) + sizeof(time_t)
        LOG_ERR(_tag, err);
        err = nvs_commit(nvs_hist_ctrl);
        LOG_ERR(_tag, err);
        this->uid_size = uid_size;
    }
    /* -------------------- get cursor from nvs if available -------------------- */
    err = nvs_get_u32(nvs_hist_ctrl, "cursor", &cursor);
    if (err == ESP_OK)
    {
        ESP_LOGI(_tag, "Load cursor = %d stored in NVS %s", cursor, P_HISTORY_CTRL);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND){
        err = nvs_set_u32(nvs_hist_ctrl, "cursor", 0); 
        LOG_ERR(_tag, err);
        err = nvs_commit(nvs_hist_ctrl);
        LOG_ERR(_tag, err);
    }

    ESP_LOGI(_tag, "ScanHistoryDB init done!");
}

void ScanHistoryDB::close()
{
    nvs_close(nvs_hist_ctrl);
}

void ScanHistoryDB::clear_history(){
    /* ------------------------- clear history partition ------------------------ */
    LOG_ERR(_tag, esp_partition_erase_range(partition, 0, partition->size));
    ESP_LOGI(_tag, "Partition \"%s\" cleared", P_HISTORY);
    /* ---------------------- delete history_ctrl namespace --------------------- */
    esp_err_t err = nvs_erase_all(nvs_hist_ctrl);
    LOG_ERR(_tag, err);
    err = nvs_commit(nvs_hist_ctrl);
    LOG_ERR(_tag, err);
    ESP_LOGI(_tag, "NVS Partition \"%s\" cleared", P_HISTORY_CTRL);
    update_cursor(0);
}

size_t ScanHistoryDB::get_block_size(){
    esp_err_t err = nvs_get_u32(nvs_hist_ctrl, "block_size", &block_size);
    LOG_ERR(_tag, err);
    return block_size;
}

size_t ScanHistoryDB::get_uid_size() const
{
    return uid_size;
}

uint32_t ScanHistoryDB::get_nb_entries() const {
    return (uint32_t)(cursor/block_size);
}


void ScanHistoryDB::add_history(const char* uid, const time_t timestamp){
    char data[block_size];
    strncpy(data, uid, block_size - sizeof(time_t)); // uid length = block size - sizeof(time_t) bytes
    for (size_t i = 0; i < sizeof(time_t); i++)
    {
        data[uid_size + i] = (uint8_t)(timestamp >> (8*i));
    }
    LOG_ERR(_tag, esp_partition_write(partition, cursor, data, block_size));
    update_cursor(cursor + block_size);
}

void ScanHistoryDB::get_history(const uint32_t entry, char *uid, time_t *time_stamp)
{
    char data[block_size];
    *time_stamp = 0;
    LOG_ERR(_tag, esp_partition_read(partition, entry*block_size, data, block_size));
    strncpy(uid, data, uid_size);
    uid[uid_size] = 0; // array termination
    for (size_t i = 0; i < sizeof(time_t); i++)
    {
        (*time_stamp) += data[uid_size + i] << (8*i); 
    }
}

void ScanHistoryDB::print_all_history()
{
    uint32_t nb_entries = get_nb_entries();
    char uid[uid_size];
    time_t time_stamp;
    for (size_t i = 0; i < nb_entries; i++)
    {
        get_history(i, uid, &time_stamp);
        ESP_LOGI(_tag, "UID-UNIX time: %s - %ld", uid, time_stamp);
    }   
}

size_t ScanHistoryDB::get_cursor(){
    esp_err_t err = nvs_get_u32(nvs_hist_ctrl, "cursor", &cursor);
    LOG_ERR(_tag, err);
    return cursor;
}

void ScanHistoryDB::update_cursor(size_t offset)
{
    LOG_ERR(_tag, nvs_set_u32(nvs_hist_ctrl, "cursor", offset)); 
    LOG_ERR(_tag, nvs_commit(nvs_hist_ctrl));
    cursor = offset;
}
