#define SANITY_CHECK(ret, val) if(ret != val){vTaskDelete(NULL); return;}
#define SANITY_CHECK_M(ret, val, tag, message) if(ret != val){ESP_LOGE(tag, message); vTaskDelete(NULL); return;}