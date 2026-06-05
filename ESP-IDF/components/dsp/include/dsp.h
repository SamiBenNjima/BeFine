#pragma once
/**
 * @file  dsp.h
 * @brief DSP pipeline public interface.
 *
 * Pipeline (50 Hz):
 *   sensor_data_t  →  IIR low-pass  →  Bernoulli conversion
 *                  →  Volume integration (FEV1)  →  Peak detection (PEF)
 *                  →  flow_sample_t  →  g_dsp_queue
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Processed sample (output of DSP pipeline) ──────────────────────────── */
typedef struct {
    float    flow_lpm;        /**< Calibrated flow rate (L/min)               */
    float    volume_l;        /**< Cumulative inspired volume (L)              */
    bool     is_inhaling;     /**< flow_lpm > BEFINE_FLOW_ONSET_LPM            */
    bool     is_exhaling;     /**< flow_lpm < -BEFINE_FLOW_ONSET_LPM           */
    uint32_t timestamp_ms;    /**< Timestamp at sensor read                    */
} flow_sample_t;

/* ── Session aggregate result (computed at COMPLETE) ───────────────────── */
typedef struct {
    uint64_t session_id;
    uint64_t timestamp_unix;
    float    fev1_l;          /**< Forced Expiratory Volume at 1 second (L)   */
    float    pef_lpm;         /**< Peak Expiratory Flow (L/min)               */
    uint8_t  technique_score; /**< 4=A 3=B 2=C 1=D 0=F                       */
    uint32_t duration_ms;
} session_result_t;

/* ── Public API ─────────────────────────────────────────────────────────── */
esp_err_t dsp_init(void);
void      dsp_task(void *pvParameters);

/* Session-level helpers — called by session FSM */
void  dsp_reset_session(void);
void  dsp_get_result(float *fev1_out, float *pef_out, uint8_t *score_out);

#ifdef __cplusplus
}
#endif
