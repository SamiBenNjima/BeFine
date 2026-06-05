/**
 * @file  spiffs_storage.c
 * @brief SPIFFS — session metadata storage (JSON per session).
 */

#include "storage.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "SPIFFS";
static bool s_mounted = false;

esp_err_t spiffs_init(void)
{
    esp_vfs_spiffs_conf_t cfg = {
        .base_path              = BEFINE_SPIFFS_BASE_PATH,
        .partition_label        = NULL,
        .max_files              = BEFINE_SPIFFS_MAX_FILES,
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_spiffs_register(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Mount failed: %s", esp_err_to_name(ret));
        return ret;
    }
    s_mounted = true;
    size_t total = 0, used = 0;
    esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI(TAG, "Mounted — %d/%d bytes used", (int)used, (int)total);
    return ESP_OK;
}

esp_err_t spiffs_save_session(const session_result_t *r)
{
    if (!s_mounted) return ESP_ERR_INVALID_STATE;

    char path[64];
    snprintf(path, sizeof(path),
             BEFINE_SPIFFS_BASE_PATH "/sess_%llu.json",
             (unsigned long long)r->session_id);

    FILE *f = fopen(path, "w");
    if (!f) {
        ESP_LOGE(TAG, "Cannot open %s for writing", path);
        return ESP_ERR_NOT_FOUND;
    }

    fprintf(f,
        "{\"id\":%llu,\"ts\":%llu,"
        "\"fev1\":%.3f,\"pef\":%.1f,"
        "\"score\":%u,\"dur_ms\":%lu,"
        "\"fw\":\"%d.%d\"}\n",
        (unsigned long long)r->session_id,
        (unsigned long long)r->timestamp_unix,
        r->fev1_l, r->pef_lpm,
        r->technique_score,
        (unsigned long)r->duration_ms,
        BEFINE_FW_MAJOR, BEFINE_FW_MINOR);

    fclose(f);
    ESP_LOGI(TAG, "Saved %s (fev1=%.2f pef=%.0f score=%d)",
             path, r->fev1_l, r->pef_lpm, r->technique_score);
    return ESP_OK;
}
