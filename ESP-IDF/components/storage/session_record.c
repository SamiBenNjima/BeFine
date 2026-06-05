/**
 * @file  session_record.c
 * @brief Orchestrates SPIFFS + SD card writes for a complete session.
 */
#include "storage.h"
#include "esp_log.h"

static const char *TAG = "SESSION_REC";

esp_err_t session_record_save(const session_result_t *r)
{
    esp_err_t ret;

    ret = spiffs_save_session(r);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SPIFFS save failed: %s", esp_err_to_name(ret));
    }

    ret = sd_close_session();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD close failed: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "Session %llu saved",
             (unsigned long long)r->session_id);
    return ESP_OK;
}
