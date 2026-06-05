/**
 * @file  peak_detector.c
 * @brief Tracks peak flow and session buffer for post-session FEV1 calculation.
 *
 * Maintains a ring buffer of the last PEAK_BUF_SIZE flow samples.
 * After INHALING→HOLDING transition, the FSM calls dsp_get_result()
 * which reads peak and computes FEV1 from the buffered samples.
 */

#include "peak_detector.h"
#include "befine_config.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "PEAK_DET";

#define SESSION_BUF_SIZE  500u   /* 10 seconds at 50 Hz — enough for any session */

static float    s_buf[SESSION_BUF_SIZE];
static uint32_t s_buf_head = 0;
static float    s_peak_lpm = 0.0f;
static bool     s_onset    = false;

void peak_detector_reset(void)
{
    memset(s_buf, 0, sizeof(s_buf));
    s_buf_head = 0;
    s_peak_lpm = 0.0f;
    s_onset    = false;
    ESP_LOGD(TAG, "Session buffer reset");
}

void peak_detector_push(float flow_lpm)
{
    /* Detect onset of first significant flow */
    if (!s_onset && flow_lpm > BEFINE_FLOW_ONSET_LPM) {
        s_onset = true;
        ESP_LOGD(TAG, "Flow onset detected at %.1f L/min", flow_lpm);
    }

    /* Only buffer samples from onset onwards */
    if (s_onset && s_buf_head < SESSION_BUF_SIZE) {
        s_buf[s_buf_head++] = flow_lpm;
    }

    if (flow_lpm > s_peak_lpm) {
        s_peak_lpm = flow_lpm;
    }
}

float peak_detector_get_peak(void)  { return s_peak_lpm; }
float *peak_detector_get_buf(void)  { return s_buf; }
uint32_t peak_detector_get_count(void) { return s_buf_head; }
