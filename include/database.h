#pragma once
#include <stdio.h>
#include <assert.h>
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"


#define P_USER "user_db"
#define P_HISTORY "history"
#define P_HISTORY_CTRL "history_ctrl"


class UserDB {
    private:
        nvs_handle_t nvs_handle;
        const char *_tag = "UserDB";
    public:
        void open();
        void close();
        uint8_t get(const char* uid);
        void set(const char* uid, uint8_t value);
};


class ScanHistoryDB {
    private:
        const char *_tag = "ScanHistoryDB";
        const esp_partition_t *partition;
        nvs_handle_t nvs_hist_ctrl;
        size_t cursor; // sizeof(size_t) = 32 bits
        size_t block_size; // in bytes
        size_t uid_size; // = block_size - sizeof(time_t)
    public:
        ScanHistoryDB(size_t uid_size=4);
        void close();
        void clear_history();
        size_t get_block_size();
        size_t get_uid_size() const;
        size_t get_cursor();
        void update_cursor(size_t offset);
        uint32_t get_nb_entries() const;
        void add_history(const char* uid, const time_t timestamp);
        void get_history(const uint32_t entry, char *uid, time_t *time_stamp);
        void print_all_history();
};






