/**
 * @file  dsp.c
 * @brief DSP pipeline task — runs on Core 1 at 50 Hz.
 *
 * Consumes sensor_data_t from g_sensor_queue.
 * Applies: IIR LP filter → Bernoulli flow conversion → peak/FEV1 tracking.
 * Pushes flow_sample_t to g_dsp_queue for ble_notify_task.
 */

#include "dsp.h"
#include "iir_filter.h"
#include "flow_calculator.h"
#include "peak_detector.h"
#include "sensors.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern QueueHandle_t g_sensor_queue;
extern QueueHandle_t g_dsp_queue;

static const char *TAG = "DSP";
static iir_biquad_t s_filter;

esp_err_t dsp_init(void)
{
    iir_biquad_init(&s_filter);
    return flow_calculator_init();
}

void dsp_reset_session(void)
{
    iir_biquad_reset(&s_filter);
    peak_detector_reset();
    ESP_LOGI(TAG, "Session DSP state reset");
}

void dsp_get_result(float *fev1_out, float *pef_out, uint8_t *score_out)
{
    float *buf   = peak_detector_get_buf();
    uint32_t n   = peak_detector_get_count();
    flow_compute_results(buf, (size_t)n, fev1_out, pef_out);
    *pef_out  = peak_detector_get_peak();  /* Use tracked peak for accuracy */
    *score_out = flow_compute_score(*fev1_out, *pef_out, 10000);
    ESP_LOGI(TAG, "Session result: FEV1=%.2fL PEF=%.1f L/min score=%d",
             *fev1_out, *pef_out, *score_out);
}

void dsp_task(void *pvParameters)
{
    (void)pvParameters;
    sensor_data_t frame;

    ESP_LOGI(TAG, "DSP task started");

    for (;;) {
        /* Block until sensor_task pushes a frame */
        if (xQueueReceive(g_sensor_queue, &frame, pdMS_TO_TICKS(100)) != pdTRUE) {
            continue;
        }

        /* 1. IIR low-pass filter on differential pressure */
        float filtered_pa = iir_biquad_process(&s_filter, frame.differential_pa);

        /* 2. Bernoulli conversion → flow in L/min */
        float flow_lpm = flow_from_pressure(filtered_pa);

        /* 3. Update peak + session buffer */
        peak_detector_push(flow_lpm);

        /* 4. Build output sample */
        flow_sample_t sample = {
            .flow_lpm     = flow_lpm,
            .is_inhaling  = (flow_lpm >  BEFINE_FLOW_ONSET_LPM),
            .is_exhaling  = (flow_lpm < -BEFINE_FLOW_ONSET_LPM),
            .timestamp_ms = frame.timestamp_ms,
        };

        /* 5. Push to BLE notify task (non-blocking — drop if full) */
        if (xQueueSend(g_dsp_queue, &sample, 0) != pdTRUE) {
            ESP_LOGD(TAG, "dsp_queue full — sample dropped");
        }
    }
}
