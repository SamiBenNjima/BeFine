#pragma once
#include "esp_err.h"
#include "dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t storage_init(void);
void      storage_task(void *pvParameters);
void      storage_enqueue_result(const session_result_t *r);

/* SPIFFS — session metadata */
esp_err_t spiffs_save_session(const session_result_t *r);
esp_err_t spiffs_list_sessions(void);

/* SD card — raw flow CSV */
esp_err_t sd_open_session (uint64_t session_id);
esp_err_t sd_write_sample (uint32_t ts_ms, float flow_lpm);
esp_err_t sd_close_session(void);

#ifdef __cplusplus
}
#endif
