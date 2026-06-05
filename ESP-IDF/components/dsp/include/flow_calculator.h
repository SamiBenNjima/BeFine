#pragma once
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t flow_calculator_init(void);
float     flow_from_pressure(float dp_pa);
void      flow_compute_results(const float *flow_lpm, size_t n,
                                float *fev1_out, float *pef_out);
uint8_t   flow_compute_score(float fev1_l, float pef_lpm, uint32_t hold_ms);
esp_err_t flow_calculator_save_calibration(float zero_pa, float span);

#ifdef __cplusplus
}
#endif
