/**
 * @file  flow_calculator.c
 * @brief Converts filtered differential pressure (Pa) to flow rate (L/min),
 *        integrates volume, and computes FEV1 / PEF.
 *
 * Physics:
 *   Q [L/min] = C × sign(ΔP) × √|ΔP|
 *   C = span calibration constant stored in NVS (default 8.5 L/min/√Pa)
 *
 * FEV1 = volume accumulated during the first 1 second of forced exhalation
 *        = samples[0..49] integrated with the trapezoid rule (50 Hz × 1s)
 *
 * Calibration constant is read from NVS at init and cached in RAM.
 */

#include "flow_calculator.h"
#include "befine_config.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <math.h>
#include <string.h>

static const char *TAG = "FLOW_CALC";

/* Cached calibration constants (loaded from NVS at init) */
static float s_zero_offset_pa = BEFINE_CALIB_ZERO_DEFAULT;
static float s_span_constant  = BEFINE_CALIB_SPAN_DEFAULT;

/* ── Load calibration from NVS ──────────────────────────────────────────── */
esp_err_t flow_calculator_init(void)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(BEFINE_NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS namespace not found — using defaults");
        return ESP_OK;   /* Non-fatal: use compile-time defaults */
    }

    uint32_t zero_raw = 0, span_raw = 0;
    /* Floats stored as uint32 bit-cast to survive NVS type restrictions */
    if (nvs_get_u32(nvs, BEFINE_NVS_KEY_ZERO_OFF, &zero_raw) == ESP_OK) {
        memcpy(&s_zero_offset_pa, &zero_raw, sizeof(float));
    }
    if (nvs_get_u32(nvs, BEFINE_NVS_KEY_SPAN, &span_raw) == ESP_OK) {
        memcpy(&s_span_constant, &span_raw, sizeof(float));
    }
    nvs_close(nvs);

    ESP_LOGI(TAG, "Calibration: zero=%.3f Pa, span=%.3f L/min/√Pa",
             s_zero_offset_pa, s_span_constant);
    return ESP_OK;
}

/* ── Real-time conversion (called per sample at 50 Hz) ──────────────────── */
float flow_from_pressure(float dp_pa)
{
    float corrected = dp_pa - s_zero_offset_pa;
    float sign      = (corrected >= 0.0f) ? 1.0f : -1.0f;
    return sign * s_span_constant * sqrtf(fabsf(corrected));
}

/* ── Post-session: FEV1 + PEF from a complete flow array ────────────────── */
void flow_compute_results(
    const float *flow_lpm,   /* Array of flow samples at BEFINE_SAMPLE_RATE_HZ */
    size_t       n_samples,
    float       *fev1_out,
    float       *pef_out)
{
    float volume = 0.0f;
    float peak   = 0.0f;
    /* dt in minutes: 1 sample at 50 Hz = 1/50/60 min */
    const float dt_min = 1.0f / ((float)BEFINE_SAMPLE_RATE_HZ * 60.0f);

    *fev1_out = 0.0f;
    *pef_out  = 0.0f;

    for (size_t i = 1; i < n_samples; i++) {
        /* Trapezoid rule: only count positive (inhale) flow for FVC/FEV1 */
        float avg = (flow_lpm[i - 1] + flow_lpm[i]) * 0.5f;
        if (avg > 0.0f) {
            volume += avg * dt_min;
        }

        if (flow_lpm[i] > peak) {
            peak = flow_lpm[i];
        }

        /* FEV1 = volume at exactly t = 1 s from onset */
        if (i == (size_t)BEFINE_FEV1_SAMPLE_INDEX) {
            *fev1_out = volume;
        }
    }
    *pef_out = peak;

    /* If session shorter than 1s, fev1 = total volume */
    if (n_samples <= (size_t)BEFINE_FEV1_SAMPLE_INDEX) {
        *fev1_out = volume;
    }
}

/* ── Technique scoring ───────────────────────────────────────────────────── */
uint8_t flow_compute_score(float fev1_l, float pef_lpm, uint32_t hold_ms)
{
    /* Simple heuristic — replace with spirometry reference values in v2 */
    bool pef_ok  = (pef_lpm  >= BEFINE_FLOW_MIN_LPM);
    bool fev1_ok = (fev1_l   >= 0.5f);
    bool hold_ok = (hold_ms  >= (uint32_t)(BEFINE_HOLD_TARGET_SEC * 1000));

    uint8_t score = (uint8_t)(pef_ok ? 1 : 0) +
                    (uint8_t)(fev1_ok ? 1 : 0) +
                    (uint8_t)(hold_ok ? 2 : 0);
    return score;   /* 0=F 1=C 2=B 3=B 4=A */
}

/* ── Factory calibration helpers ─────────────────────────────────────────── */
esp_err_t flow_calculator_save_calibration(float zero_pa, float span)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(BEFINE_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) return ret;

    uint32_t zero_raw, span_raw;
    memcpy(&zero_raw, &zero_pa, sizeof(float));
    memcpy(&span_raw, &span,    sizeof(float));
    nvs_set_u32(nvs, BEFINE_NVS_KEY_ZERO_OFF, zero_raw);
    nvs_set_u32(nvs, BEFINE_NVS_KEY_SPAN,     span_raw);
    ret = nvs_commit(nvs);
    nvs_close(nvs);

    if (ret == ESP_OK) {
        s_zero_offset_pa = zero_pa;
        s_span_constant  = span;
        ESP_LOGI(TAG, "Calibration saved: zero=%.3f Pa span=%.3f", zero_pa, span);
    }
    return ret;
}
