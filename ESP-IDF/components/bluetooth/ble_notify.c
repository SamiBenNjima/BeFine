/**
 * @file  ble_notify.c
 * @brief BLE notify task — dequeues DSP samples and sends GATT notifications.
 *
 * Runs on Core 0 at BEFINE_TASK_PRIO_BLE_NTF.
 * Consumes flow_sample_t from g_dsp_queue.
 * Environment notifications are sent at 1 Hz (every 50 flow samples).
 */

#include "bluetooth.h"
#include "dsp.h"
#include "sensors.h"
#include "befine_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

extern QueueHandle_t     g_dsp_queue;
extern EventGroupHandle_t g_ble_events;
#define BLE_BIT_CONNECTED  (1 << 0)

/* Last known environment reading (updated by sensor_task side-channel) */
extern float g_last_temp_c;
extern float g_last_humidity_pct;

static const char *TAG = "BLE_NOTIFY";

void ble_notify_task(void *pvParameters)
{
    (void)pvParameters;
    flow_sample_t sample;
    uint32_t env_cycle = 0;

    ESP_LOGI(TAG, "BLE notify task started");

    for (;;) {
        /* Wait for a processed sample */
        if (xQueueReceive(g_dsp_queue, &sample, pdMS_TO_TICKS(100)) != pdTRUE) {
            continue;
        }

        /* Only send if a client is connected */
        EventBits_t bits = xEventGroupGetBits(g_ble_events);
        if (!(bits & BLE_BIT_CONNECTED)) {
            continue;
        }

        /* Flow Rate notification — 50 Hz */
        gatt_notify_flow(sample.flow_lpm,
                         sample.is_inhaling,
                         sample.is_exhaling);

        /* Environment notification — 1 Hz (every BEFINE_SAMPLE_RATE_HZ cycles) */
        if (++env_cycle >= (uint32_t)BEFINE_SAMPLE_RATE_HZ) {
            env_cycle = 0;
            gatt_notify_env(g_last_temp_c, g_last_humidity_pct);
        }
    }
}
