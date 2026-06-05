/**
 * @file  storage.c
 * @brief Storage init and async storage task.
 *
 * Receives session_result_t from g_storage_queue and writes
 * to SPIFFS (JSON metadata) + SD card (closes open CSV file).
 */

#include "storage.h"
#include "befine_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern QueueHandle_t g_storage_queue;
static const char *TAG = "STORAGE";

/* Forward declarations from sub-modules */
extern esp_err_t spiffs_init(void);
extern esp_err_t sd_init(void);
extern esp_err_t session_record_save(const session_result_t *r);

esp_err_t storage_init(void)
{
    esp_err_t ret = spiffs_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SPIFFS unavailable — session metadata will not be saved");
    }
    ret = sd_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD card unavailable — raw CSV logging disabled");
    }
    return ESP_OK;   /* Non-fatal — device works without storage */
}

void storage_enqueue_result(const session_result_t *r)
{
    if (xQueueSend(g_storage_queue, r, 0) != pdTRUE) {
        ESP_LOGW(TAG, "storage_queue full — result dropped");
    }
}

void storage_task(void *pvParameters)
{
    (void)pvParameters;
    session_result_t result;

    ESP_LOGI(TAG, "Storage task started");

    for (;;) {
        if (xQueueReceive(g_storage_queue, &result,
                          portMAX_DELAY) == pdTRUE) {
            session_record_save(&result);
        }
    }
}
